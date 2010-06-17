/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pcdisk.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pcdisk.h, 4, 4/3/04 12:13:19 AM, Nagaraja Kolur$
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
*       pcdisk.h                            Nucleus FILE\ARM7TDMI\ADS 2.5.6 
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This file contains Nucleus FILE constants common to both the   
*       application and the actual Nucleus FILE components.  This file  
*       also contains data structure definitions that hide internal     
*       information from the application.                               
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       FATSWAP                             Fat blocks structure.       
*       DDRIVE                              Block 0 image.              
*       DOSINODE                            Dos Directory Entry.        
*       BLKBUFF                             Block buffer.               
*       LNAMINFO                            Long file name information. 
*       LNAMENT                             Long file name Directory    
*                                            Entry.                     
*       FINODE                              DOS entry.                  
*       DIRBLK                              Directory information.      
*       DROBJ                               Object used to find a dirent
*                                            on a disk and its parent's.
*       DATESTR                             Date stamping buffer.       
*       PC_FILE                             Internal file representation.
*       DSTAT                               File search structure.      
*       FMTPARMS                            Format parameter blcok.     
*       FILE_SYSTEM_USER                    File system user structure. 
*       _PC_BDEVSW                          Device driver dispatch table.
*       _NUF_FSTBL                          File system dispatch table. 
*       PTABLE_ENTRY                        Partition table descriptions.
*       PTABLE                              Boot Record structure.      
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       None.                                                           
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       nucleus.h                           System definitions          
*                                                                       
*************************************************************************/

#ifndef __PCDISK__
#define __PCDISK__ 1

#include        <stddef.h>

#if 0 //ndef PLUS_VERSION_COMP

typedef char           INT8;    /*  1                   1   */
typedef unsigned char  UINT8;   /*  1                   1   */
typedef signed short   INT16;   /*  2                   2   */
typedef unsigned short UINT16;  /*  2                   2   */
typedef signed long    INT32;   /*  4                   4   */
typedef unsigned long  UINT32;  /*  4                   4   */

#endif /* PLUS_VERSION_COMP */

#ifndef NUCLEUS
#include        "nucleus.h"                 /* System definitions      */
#endif


/* #define DEBUG */

/* ====================== ARM little endian port ====================== */
#include        <stdio.h>


/* ARM PID board */
#define ARMPID                  1

#ifdef DEBUG
/*#define PRINT_STRING    UART_Put_String*/
#define PRINT_STRING   printf 
/*void va_error_print(char* format, ...); */
/*#define DEBUG_PRINT     va_error_print  */
#define DEBUG_PRINT    printf
#else
#define PRINT_STRING
#endif  /* ifdef DEBUG */

#define FAR
#define NEAR

/* Use the x86 processor. */
#define X86                     0           /* 1 : Use  0 : Not use */

/* ============= MGC DRIVERS =============== */
/* Set the following line to 1 if you purchased the IDE driver */
#define EBS_IDE                 1

/* If using ATA hard disk, define BOTH EBS_IDE and IDE_ATA  */
#define   IDE_ATA               1

/* If using PCMCIA card, define BOTH EBS_IDE and IDE_PCM  */
#define   IDE_PCM               0

/* Set the following line to 1 if you purchased the FLOPPY driver */
#define EBS_FLOPPY              0

/* Set the following line to 1 if you purchased the RAMDISK driver */
#define RAMDISK                 0


/* ============ OTHER CONSTANTS ============ */
/*  Set this to total number of drives to support.  This value (6) supports
    (A,B,C,D,E,F). If you are going to use the RAM Disk set NDRIVES to 1. */

#define NDRIVES                 3


/* NUF_Release_String contains a string describing this release of Nucleus
   FILE.  */

#define NUF_Release_String "Copyright MGC 2003 - Nucleus FILE - ARM7TDMI ADS v. 2.5"


#define YES                     1
#define NO                      0
#define BLOCKEQ0                0L

/* These handle byte alignment and endianess */
#define SWAP16(X,Y) fswap16(X,Y)
#define SWAP32(X,Y) fswap32(X,Y)

/* =========== User tunable section ================ */
/* ============ Device drivers =============== */

/* ============= RAMDISK =============== */

/* For the demonstration port we create a 50K ram disk.  If you purchased
   MGC's RAM Disk Driver, you can change the following definitions to
   any size you desire (with the exceptions as listed below).

   Note on Intel the RAM Disk is always in the far heap.  So you may create
   a large ram disk even in small model. Note the ram disk driver will
   allocate this much core when it is first mounted.

   If you don't need a ramdisk eliminate it from devtable.c
*/


/* Set the following to one if you wish to allocate ram disk memory from
   a PLUS memory pool. This affects code in nufp.c and ramdisk.c. This should
   be set to 1 for 32 bit systems where it is possible to use allocate > 64K
   to a memory pool.  If you wish to allocate a RAM Disk larger than 48K on
   an Intel real-mode based Nucleus PLUS port, then you should set this
   manifest to 0.  In that case, the pool will be created by using a DOS
   malloc call.
*/
#define RAMDISK_FROMPOOL        1      /* Plus only */

#if (RAMDISK_FROMPOOL)

/* Nucleus PLUS: If allocating the ram disk from a partition pool we
   allocated 12 pages (48K). This is because 8086 real mode ports under plus
   may only allocate 64K at a time for a pool. For 32 bit ports this
   restriction does not exist.  */
#define NUM_RAMDISK_PAGES       16    /*  (Must be at least 1) */
#else
#define NUM_RAMDISK_PAGES       16     /*  (Must be at least 1) */
#endif


#define RAMDISK_PAGE_SIZE       16      /*  8 blocks ='s 4 k (dont exceed 32) */
#define NRAMDISKBLOCKS          (NUM_RAMDISK_PAGES * RAMDISK_PAGE_SIZE)

/* ======== MULTI TASKING SUPPORT   =============== */

/*  Maximum # of tasks that may use the file system.   */
#define NUM_USERS               10

/* Blocks in the buffer pool. Uses 532 bytes per block.
   impacts performance during directory traversals 20 to 30 is optimal */
#define NBLKBUFFS               30

/* Maximum Number of open Files */
#define NUSERFILES              50

/* Directory Object Needs. Conservative guess is One CWD per user per drive +
   One per file + one per User for directory traversal */
#define NFINODES                (NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)
#define NDROBJS                 (NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)

#define EMAXPATH                255     /* Maximum path length Change if you like */

#define MAXSECTORS              256     /*  Maximum sectors */

/* Number of maximum file sectors(2GB) */
#define MAXFILE_SIZE            4194304 /* (2GB / 512) */

/* NOTE: See the end of this file for more tunable constants which pertain
   to the Nucleus environment, specifically allocating memory. */

/*=============== END TUNABLE CONSTANTS ==========================*/

/*=============== MULTI TASKING DATA STRUCTURES and MACROS =======*/

#if (NUM_USERS == 1)
#define LOCK_METHOD             0
/* These macros are API call prolog and epilogs. In multitasking mode
they lock/unlock the RTFS semaphore and check if the user is registered
via pc_rtfs_become_user() respectively. In single tasking mode they do nothing */

#define PC_FS_ENTER()
#define PC_FS_EXIT()
#define CHECK_USER(X, Y)
#define VOID_CHECK_USER()
#else 
/* Num users > 1 */
/* These macros are API call prolog and epilogs. In multitasking mode
they lock/unlock the RTFS semaphore and check if the user is registered
via pc_rtfs_become_user() respectively. In single tasking mode they do nothing */

#define PC_FS_ENTER()           UINT16 process_flags = pc_fs_enter();
#define PC_FS_EXIT()            pc_fs_exit(process_flags);
#define CHECK_USER(X, Y)        if (!NU_Check_File_User()) \
                                        { pc_fs_exit(process_flags); \
                                          return((X) Y);}
#define VOID_CHECK_USER()       if (!NU_Check_File_User()) \
                                        { pc_fs_exit(process_flags); \
                                          return;}

/* This is the type of a unique number that is associated with each task. It
   could be a process ID as in unix or a task control block address or
   index in a real time exec.  It is used to validate tasks as registered
   Nucleus FILE users. (see pc_users.c)  */
typedef int CONTEXT_HANDLE_TYPE;

/* See the porting manual and pc_locks.c for a description of this constant */
/* Fine grained multitask support */
#define LOCK_METHOD             2
#endif  /* Num users > 1 */

#if (LOCK_METHOD == 2)
/* Natural type for an event handle (see pc_locks.c)  */
typedef int WAIT_HANDLE_TYPE;

/* This is how many event handles we will allocate at startup. */
#define NHANDLES                ((NDRIVES*3) + NFINODES))

/* Lock object: We keep track of opencount and exclusive internally.
   wait_handle is a handle to an event channel that is provided by
   the executive or os environment. via pc_alloc_lock(). When we need
   to trigger an event on the channel we call pc_wake_lock(wait_handle)
   when we need to block we call pc_wait_lock(wait_handle)
*/
typedef struct lockobj
{
    WAIT_HANDLE_TYPE    wait_handle;
    INT16               opencount;
    INT                 exclusive;
} LOCKOBJ;

/* Special macros providing fine grained re-entrancy */
#define PC_DRIVE_ENTER(X, Y)    pc_drive_enter((X), (Y));
#define PC_DRIVE_EXIT(X)        pc_drive_exit((X));
#define PC_INODE_ENTER(X,Y)     pc_inode_enter((X), (Y));
#define PC_INODE_EXIT(X)        pc_inode_exit((X));
#define PC_FAT_ENTER(X)         pc_fat_enter((X));
#define PC_FAT_EXIT(X)          pc_fat_exit((X));
#define PC_DRIVE_IO_ENTER(X)    pc_drive_io_enter((X));
#define PC_DRIVE_IO_EXIT(X)     pc_drive_io_exit((X));
#else
/* LOCK_METHOD 1 or 0 */
#define PC_DRIVE_ENTER(X, Y)
#define PC_DRIVE_EXIT(X)
#define PC_INODE_ENTER(X, Y)
#define PC_INODE_EXIT(X)
#define PC_FAT_ENTER(X)
#define PC_FAT_EXIT(X)
#define PC_DRIVE_IO_ENTER(X)
#define PC_DRIVE_IO_EXIT(X)
#endif /*  (LOCK_METHOD == 2) */

/* Changing to '/' and "//" should give unix style path separators. This
   is not tested but it should work */
#define BACKSLASH '\\'
#define STRBACKSLASH "\\"

#define PCDELETE (UINT8) 0xE5       /* MS-DOS file deleted char */

#define ARDONLY     0x01        /* Read only fil attributes */
#define AHIDDEN     0x02        /* Hidden fil attributes */
#define ASYSTEM     0x04        /* System fil attributes */
#define AVOLUME     0x08        /* Volume Lable fil attributes */
#define ADIRENT     0x10        /* Dirrectory fil attributes */
#define ARCHIVE     0x20        /* Archives fil attributes */
#define ANORMAL     0x00        /* Normal fil attributes */


/* Structure used to track cached fat blocks */
typedef struct fatswap
{
    UINT32      n_to_swap;          /* Next to swap. For implementing round robin */
    UINT32      base_block;         /* base_block of FAT sector index in data_map */
    /*  These two counters track cache usage as we fill it. Eventually the
        FAT fills and we go into swapping mode at steady state */
    UINT16      n_blocks_used;      /* How many blocks in the cache have we used */
    INT         n_blocks_total;     /* How many blocks are available in the cache */

    UINT8       *pdirty;            /* BIT-map of blocks needing flushing */
    INT         block_0_is_valid;   /* If YES then data_map[0] contains the offset
                                       of the first block of the FAT */
    INT         data_map_size;      /* data_map table size */
    UINT16      *data_map;          /* Table that maps block numbers in the fat to
                                       block offsets in our data array. zero means
                                       the block is not mapped. Except.. data_map[0]
                                       contains block zero of the FAT which
                                       is at location 0 in the data array */
    UINT8 FAR   *data_array;        /* Block buffer area on Intel systems always FAR */

} FATSWAP;


/* Structure to contain block 0 image from the disk */
typedef struct ddrive
{
    UINT32      fs_type;            /* FAT_FILE_SYSTEM */
    INT16       opencount;          /* Drive open count */
    UINT16      bytespcluster;      /* Bytes per cluster */
    UINT32      byte_into_cl_mask;  /* And this with file pointer to get the
                                       byte offset into the cluster */
    UINT16      fasize;             /* Nibbles per fat entry. (2 or 4) */
    UINT32      rootblock;          /* First block of root dir */
    UINT32      firstclblock;       /* First block of cluster area */
    UINT16      driveno;            /* Driveno. Set when open succeeds */
    UINT32      maxfindex;          /* Last element in the fat */
    UINT16      secproot;           /* blocks in root dir */
    INT         fat_is_dirty;       /* FAT update flag */
    INT         use_fatbuf;         /* Use FAT buffer flag */
    FATSWAP     fat_swap_structure; /* Fat swap structure if swapping */
    UINT16      log2_secpalloc;     /* Log of sectors per cluster */
    UINT32      maxfsize_cluster;   /* Maximum file cluster size */
    UINT16      valid_fsinfo;       /* fsinfo valid  flag */
          /******** BPB  ***********/
    INT8        oemname[8];         /* 0x03 OEM name */
    UINT16      bytspsector;        /* 0x0b Must be 512 for this implementation */
    UINT8       secpalloc;          /* 0x0d Sectors per cluster */
    UINT16      fatblock;           /* 0x0e Reserved sectors before the FAT */
    UINT8       numfats;            /* 0x10 Number of FATS on the disk */
    UINT16      numroot;            /* 0x11 Maximum # of root dir entries */
    UINT32      numsecs;            /* 0x13 Total # sectors on the disk */
    UINT8       mediadesc;          /* 0x15 Media descriptor byte */
    UINT32      secpfat;            /* 0x16 Size of each fat */
    UINT16      secptrk;            /* 0x18 sectors per track */
    UINT16      numhead;            /* 0x1a number of heads */
    UINT32      numhide;            /* 0x1c # hidden sectors */
    UINT32      bignumsecs;         /* 0x20 Sectors per FAT32 */
 /* FAT16/12 */
    UINT8       phys_num;           /* 0x24 physical drive number */
    UINT8       xtbootsig;          /* 0x26 extend boot recored signature */
    UINT32      volid;              /* 0x27 or 0x43 (FAT32) volume serial number */
    UINT8       vollabel[11];       /* 0x2b or 0x47 (FAT32) volueme_label */
 /* FAT32 */
    UINT32      bigsecpfat;         /* 0x24 Size of each FAT on the FAT32 */
    UINT16      fat_flag;           /* 0x28 FAT flag */
    UINT16      file_version;       /* 0x2a file system version */
    UINT32      rootdirstartcl;     /* 0x2c root cluster */
    UINT16      fsinfo;             /* 0x30 fsinfo block */
    UINT16      bpb_backup;         /* 0x32 BPB backup */
 /* FAT32 FSINFO */
    UINT32      free_clusters_count; /* If non-zero pc_free may use this value */
    UINT32      free_contig_pointer; /* Efficiently stored */

} DDRIVE;


/* Dos Directory Entry Memory Image of Disk Entry */
#define INOPBLOCK       16          /* 16 of these fit in a block */
typedef struct dosinode
{
    UINT8       fname[8];           /* 00-07h   File name */
    UINT8       fext[3];            /* 08-0Ah   File extension */
    UINT8       fattribute;         /*    0Bh   File attributes */
    UINT8       reserve;            /*    0Ch   Reserved */
    UINT8       fcrcmsec;           /*    0Dh   File create centesimal mili second */
    UINT16      fcrtime;            /* 0E-0Fh   File create time */
    UINT16      fcrdate;            /* 10-11h   File create date */
    UINT16      faccdate;           /* 12-13h   Access date */
    UINT16      fclusterhigh;       /* 14-15h   High cluster for data file */
    UINT16      fuptime;            /* 16-17h   File update time */
    UINT16      fupdate;            /* 18-19h   File update */
    UINT16      fclusterlow;        /* 1A-1Bh   Low cluster for data file */
    UINT32      fsize;              /* 1C-1Fh   File size */

} DOSINODE;


/* Block buffer */
typedef struct blkbuff
{
    struct      blkbuff *pnext;     /* Next block buffer pointer */
    UINT32      lru;                /* Last recently used stuff */
    INT16       locked;             /* Reserve */
    DDRIVE      *pdrive;            /* Drive block 0 image */
    UINT32      blockno;            /* I/O block number */
    INT16       use_count;          /* use count */
    INT         io_pending;         /* I/O pending flag */
    UINT8       data[512];          /* I/O data buffer */

} BLKBUFF;


/* Long file name information */
typedef struct lnaminfo
{
    BLKBUFF     *lnamblk1;          /* Long file name block buffer 1    */
    BLKBUFF     *lnamblk2;          /* Long file name block buffer 2    */
    BLKBUFF     *lnamblk3;          /* Long file name block buffer 3    */
    INT16       lnam_index;         /* Long file name start index       */
    INT16       lnament;            /* Number of long file name entries */
    UINT8       lnamchk;            /* Check value of short file name   */

} LNAMINFO;


/* Long file name Directory Entry Memory Image of Disk Entry */
typedef struct lnament
{
    UINT8       ent_num;            /*    00h   Long filename entry number  */
    UINT8       str1[10];           /* 01-0Ah   File name                   */
    UINT8       attrib;             /*    0Bh   File attributes, always 0Fh */
    UINT8       reserve;            /*    0Ch   Reserved                    */
    UINT8       snamchk;            /*    0Dh   Short file name check       */
    UINT8       str2[12];           /* 0E-19h   File name                   */
    UINT8       fatent[2];          /* 1A-1Bh   File entry number, always 00*/
    UINT8       str3[4];            /* 1C-1Fh   File name                   */

} LNAMENT;


/* Internal representation of DOS entry */
typedef struct finode
{
    UINT8       fname[8];           /* File name */
    UINT8       fext[3];            /* File extension */
    UINT8       fattribute;         /* File attributes */
    UINT8       reserve;            /* Reserved */
    UINT8       fcrcmsec;           /* File create centesimal mili second */
    UINT16      fcrtime;            /* File create time */
    UINT16      fcrdate;            /* File create date */
    UINT16      faccdate;           /* Access date */
    UINT16      fuptime;            /* File update time */
    UINT16      fupdate;            /* File update */
    UINT32      fcluster;           /* Cluster for data file */
    UINT32      fsize;              /* File size */
    UINT32      alloced_size;       /* Size rounded up to the hearest cluster
                       (only maintained for files */
    INT16   opencount;

/* If the finode is an open file the following flags control the sharing.
   they are maintained by NU_Open                                      */
#ifdef OF_WRITE
#undef OF_WRITE
#endif
#define OF_WRITE            0x01    /* File is open for write by someone */
#define OF_WRITEEXCLUSIVE   0x02    /* File is open for write by someone
                                       they wish exclusive write access */
#define OF_EXCLUSIVE        0x04    /* File is open with exclusive access not
                                       sharing write or read */
    INT16       openflags;          /* For Files. Track how files have it open */
    DDRIVE      *my_drive;          /* Block 0 image from this disk */
    UINT32      my_block;           /* Number of this file directory entry block */
    INT16       my_index;           /* Directory entry index */
#if (LOCK_METHOD == 2)
    LOCKOBJ     lock_object;        /* for locking during critical sections */
#endif
    struct finode *pnext;           /* Next DOS entry pointer */
    struct finode *pprev;           /* Prevenience DOS entry pointer */
    UINT16      abs_length;         /* Absolute path length */

} FINODE;


/* contain location information for a directory */
typedef struct dirblk
{
    UINT32      my_frstblock;       /* First block in this directory */
    UINT32      my_block;           /* Current block number */
    UINT16      my_index;           /* dirent number in my cluster   */
    /* fname[0] == "\0" entry blocks and index */
    UINT32      end_block;          /* End block number in this directory entry */
    UINT16      end_index;          /* End directory entry number in my cluster */

} DIRBLK;


/* Object used to find a dirent on a disk and its parent's */
#define FUSE_UPBAR      2            /* Short file name used upperbar
                                         First six characters of the file name plus ~n */
typedef struct drobj
{
    DDRIVE      *pdrive;            /* Block 0 image from the disk */
    FINODE      *finode;            /* Dos entry */
    DIRBLK      blkinfo;            /* Directory information */
    INT         isroot;             /* True if this is the root */
    BLKBUFF     *pblkbuff;          /* Block buffer pointer */
    LNAMINFO    linfo;              /* for long file name */

} DROBJ;


/* Date stamping buffer */
#define DSET_ACCESS     0           /* Set the only access-time,date */
#define DSET_UPDATE     1           /* Set the access-time,date and update-time,date */
#define DSET_CREATE     2           /* Set the all time and date.(access-time,date, update-time,date 
                                        create-time,date) */
typedef struct datestr
{
    UINT8       cmsec;              /* Centesimal mili second */
    UINT16      date;               /* Date */
    UINT16      time;               /* Time */

} DATESTR;


/* Internal file representation */
typedef struct pc_file
{
    DROBJ       *pobj;              /* Info for getting at the inode */
    UINT16      flag;               /* Acces flags from NU_Open(). */
    UINT32      fptr;               /* Current file pointer */
    UINT32      fptr_cluster;       /* Current cluster boundary for fptr */
    UINT32      fptr_block;         /* Block address at boundary of fprt_cluster */
    INT         is_free;            /* If YES this FILE may be used (see pc_memry.c) */
    INT         fupdate;            /* File update flag */
    INT         at_eof;             /* True if fptr was > alloced size last time we set
                                       it. If so synch_file pointers will fix it if the
                                       file has expanded. */
} PC_FILE;


/* File search structure */
typedef struct dstat
{
    CHAR       sfname[9];          /* Null terminated file and extension */
    CHAR       fext[4];
    CHAR       lfname[EMAXPATH+1]; /* Null terminated long file name */
    UINT8       fattribute;         /* File attributes */
    UINT8       fcrcmsec;           /* File create centesimal mili second */
    UINT16      fcrtime;            /* File create time */
    UINT16      fcrdate;            /* File create date */
    UINT16      faccdate;           /* Access date */
    UINT16      fclusterhigh;       /* High cluster for data file */
    UINT16      fuptime;            /* File update time */
    UINT16      fupdate;            /* File update */
    UINT16      fclusterlow;        /* Low cluster for data file */
    UINT32      fsize;              /* File size */

    /* INTERNAL */
    UINT8       pname[EMAXPATH+1];  /* Pattern. */
    UINT8       pext[4];
    INT8        path[EMAXPATH+1];
    DROBJ       *pobj;              /* Info for getting at the inode */
    DROBJ       *pmom;              /* Info for getting at parent inode */

} DSTAT;


/* User supplied parameter block for formatting */
typedef struct fmtparms
{
    UINT8       partdisk;           /* 1 : Partitioned disk, 0 : Not paritioned disk */
    UINT16      bytepsec;           /* Bytes per sectors */
    UINT8       secpalloc;          /* Sectors per cluster */
    UINT16      secreserved;        /* Reserved sectors before the FAT */
    UINT8       numfats;            /* Number of FATS on the disk */
    UINT16      numroot;            /* Maximum # of root dir entries. FAT32 always 0 */
    UINT8       mediadesc;          /* Media descriptor byte */
    UINT16      secptrk;            /* Sectors per track */
    UINT16      numhead;            /* Number of heads */
    UINT16      numcyl;             /* Number of cylinders */
    UINT32      totalsec;           /* Total sectors */

    INT8        oemname[9];             /* Only first 8 bytes are used */
    UINT8       physical_drive_no;      /* Boot Drive Information */
                                        /* 80h BootDrive  00h NotBootDrive */
    UINT32      binary_volume_label;    /* Volume ID or Serial Number */
    INT8        text_volume_label[12];  /* Volume Label */

} FMTPARMS;


/* User structure: Each task that accesses the file system may have one of 
   these structures. The pointer fs_user points to the current user. In 
   a real time exec you might put this in the process control block or
   have an array of them an change the fs_user pointer at task switch time

   Note: Having one of these structures per task is NOT absolutely necessary.
     If you do not these properties are simply system globals. 

   Code in pc_memory.c and pc_user.c provide a plausible way of managing the
   user structure.
*/

typedef struct file_system_user
{
    INT         p_errno;            /* error number */
    INT16       dfltdrv;            /* Default drive to use if no drive specified */
    DROBJ       *lcwd[NDRIVES];     /* current working dirs */
#if (NUM_USERS > 1)
    /* If multitasking. We place the task's handle here when we register it
       as a USER. This allows us to validate users when API calls are made. */
    CONTEXT_HANDLE_TYPE context_handle;
#endif

} FILE_SYSTEM_USER;

typedef FILE_SYSTEM_USER *PFILE_SYSTEM_USER;


/* Internal Error codes */
#define PCERR_FAT_FLUSH         0       /* Cant flush FAT */
#define PCERR_FAT_NULLP         1       /* Flushfat called with null pointer */
#define PCERR_NOFAT             2       /* No FAT type in this partition., Can't Format */
#define PCERR_FMTCSIZE          3       /* Too many clusters for this partition., Can't Format */
#define PCERR_FMTFSIZE          4       /* File allocation Table Too Small, Can't Format */
#define PCERR_FMTRSIZE          5       /* Numroot must be an even multiple of INOPBLOCK */
#define PCERR_FMTWPBR           6       /* Failed writing partition boot record */
#define PCERR_FMTWFAT           7       /* Failed writing FAT block */
#define PCERR_FMTWROOT          8       /* Failed writing root block */
#define PCERR_FMT2BIG           9       /* Total sectors may not exceed 64k.  */
#define PCERR_FSTOPEN           10      /* pc_free_all freeing a file */
#define PCERR_INITMEDI          11      /* Not a DOS disk:pc_dskinit */
#define PCERR_INITDRNO          12      /* Invalid driveno to pc_dskinit */
#define PCERR_INITCORE          13      /* Out of core:pc_dskinit */
#define PCERR_INITDEV           14      /* Can't initialize device:pc_dskinit */
#define PCERR_INITREAD          15      /* Can't read block 0:pc_dskinit */
#define PCERR_BLOCKCLAIM        16      /* PANIC: Buffer Claim */
#define PCERR_INITALLOC         17      /* Fatal error: Not enough core at startup */
#define PCERR_BLOCKLOCK         18      /* Warning: freeing a locked buffer */
#define PCERR_FREEINODE         19      /* Bad free call to freei */
#define PCERR_FREEDROBJ         20      /* Bad free call to freeobj */
#define PCERR_FATCORE           21      /* "Not Enough Core To Load FAT" */
#define PCERR_FATREAD           22      /* "IO Error While Failed Reading FAT" */
#define PCERR_BLOCKALLOC        23      /* "Block Alloc Failure Memory Not Initialized" */
#define PCERR_DROBJALLOC        24      /* "Memory Failure: Out of DROBJ Structures" */
#define PCERR_FINODEALLOC       25      /* "Memory Failure: Out of FINODE Structures" */
/* The next two are only relaventin a multi tasking environment */
#define PCERR_USERS             26      /* Out of User structures increase NUM_USERS */
#define PCERR_BAD_USER          27      /* Trying to use the API without calling 
                                          pc_rtfs_become_user() first */
#define PCERR_NO_DISK           28      /* No disk      */
#define PCERR_DISK_CHANGED      29      /* Disk changed */
#define PCERR_DRVALLOC          30      /* Memory Failure: Out of DDRIVE Structures */

#define PCERR_PATHL             31      /* Path name too long */
#define PCERR_FMTWMBR           32       /* Failed writing MBR at block 0 */
#define PCERR_FMTWFSINFO        33       /* Failed writing FSINFO block */


/* File creation permissions for open */
/* Note: OCTAL */
#define PS_IWRITE       0000400         /* Write permitted      */
#define PS_IREAD        0000200         /* Read permitted. (Always true anyway)*/

/* File access flags */
#define PO_RDONLY       0x0000          /* Open for read only*/
#define PO_WRONLY       0x0001          /* Open for write only*/
#define PO_RDWR         0x0002          /* Read/write access allowed.*/
#define PO_APPEND       0x0008          /* Seek to eof on each write*/
#define PO_CREAT        0x0100          /* Create the file if it does not exist.*/
#define PO_TRUNC        0x0200          /* Truncate the file if it already exists*/
#define PO_EXCL         0x0400          /* Fail if creating and already exists*/
#define PO_TEXT         0x4000          /* Ignored*/
#define PO_BINARY       0x8000          /* Ignored. All file access is binary*/
#define PO_NOSHAREANY   0x0004          /* Wants this open to fail if already
                                           open.  Other opens will fail while
                                           this open is active */
#define PO_NOSHAREWRITE 0x0800          /* Wants this opens to fail if already
                                           open for write. Other open for
                                           write calls will fail while this
                                           open is active. */

/* Errno values */
#define PEBADF          9               /* Invalid file descriptor*/
#define PENOENT         2               /* File not found or path to file not found*/
#define PEMFILE         24              /* No file descriptors available (too many
                                           files open)*/
#define PEEXIST         17              /* Exclusive access requested but file
                                           already exists.*/
#define PEACCES         13              /* Attempt to open a read only file or a
                                           special (directory)*/
#define PEINVAL         22              /* Seek to negative file pointer attempted.*/
#define PENOSPC         28              /* Write failed. Presumably because of no space */
#define PESHARE         30              /* Open failed do to sharing */

/* Arguments to SEEK */
#define PSEEK_SET       0               /* offset from begining of file*/
#define PSEEK_CUR       1               /* offset from current file pointer*/
#define PSEEK_END       2               /* offset from end of file*/

/* Arguments to po_extend_file */
#define PC_FIRST_FIT    1
#define PC_BEST_FIT     2
#define PC_WORST_FIT    3


/* Define service completion status constants.  */
/* Nucleus FILE status value */
#define     NUF_BAD_USER        -3000       /* Not a file user.                         */
#define     NUF_BADDRIVE        -3001       /* Bad drive number.                        */
#define     NUF_BADPARM         -3002       /* Invalid parametor given */
    /* Disk */
#define     NUF_NOT_OPENED      -3003       /* The disk is not opened yet.               */
#define     NUF_NO_DISK         -3004       /* Disk is removed.                         */
#define     NUF_DISK_CHANGED    -3005       /* Disk is changed.                         */
#define     NUF_INVALID_CSIZE   -3006       /* The Disk has invalid clluster size.      */
#define     NUF_FATCORE         -3007       /* Fat cache table too small. */
#define     NUF_DEFECTIVEC      -3008       /* Defective cluster detected. */
#define     NUF_BADDISK         -3009       /* Bad Disk */
#define     NUF_NO_PARTITION    -3010       /* No partition in disk. */
#define     NUF_ROOT_FULL       -3011       /* Root directry full */
    /* Format */
#define     NUF_NOFAT           -3012       /* No FAT type in this partition. Can't Format */
#define     NUF_FMTCSIZE        -3013       /* Too many clusters for this partition. Can't Format */
#define     NUF_FMTFSIZE        -3014       /* File allocation table too small. Can't Format. */
#define     NUF_FMTRSIZE        -3015       /* Numroot must be an even multiple of 16  */
#define     NUF_FORMAT          -3016       /* Not formatted this disk. */
    /* Path */
#define     NUF_LONGPATH        -3017       /* Path or filename too long */
#define     NUF_INVNAME         -3018       /* Path or filename is invalid.  */
    /* File */
#define     NUF_PEMFILE         -3019       /* No file descriptors available (too many files open). */
#define     NUF_BADFILE         -3020       /* Invalid file descriptor */
#define     NUF_ACCES           -3021       /* Attempt to open a read only file or a special (directory). */
#define     NUF_NOSPC           -3022       /* Write failed. Presumably because of no space. */
#define     NUF_SHARE           -3023       /* The access conflict from multiple task to a specific file. */
#define     NUF_NOFILE          -3024       /* File not found or path to file not found. */
#define     NUF_EXIST           -3025       /* Exclusive access requested but file already exists. */
#define     NUF_NVALFP          -3026       /* Seek to negative file pointer attempted. */
#define     NUF_MAXFILE_SIZE    -3027       /* Over the maximum file size. */
#define     NUF_NOEMPTY         -3028       /* Directory is not empty.     */
#define     NUF_INVPARM         -3029       /* Invalid parameter is specified.  */
#define     NUF_INVPARCMB       -3030       /* Invalid parameter combination is specified.  */
    /* Memory  */
#define     NUF_NO_MEMORY       -3031       /* Can't allocate internal buffer. */
#define     NUF_NO_BLOCK        -3032       /* No block buffer available */
#define     NUF_NO_FINODE       -3033       /* No FINODE buffer available */
#define     NUF_NO_DROBJ        -3034       /* No DROBJ buffer available */
#define     NUF_IO_ERROR        -3035       /* Driver IO function routine returned error */

#define     NUF_INTERNAL        -3036       /* Nucleus FILE internal error */


/* IDE Driver Error code */
#define     NUF_IDE_ASSIGN              -3100      /* Logical drive assign error. */
#define     NUF_IDE_NUM_LOGICAL         -3101      /* NUM_LOGICAL_DRIVES set error. */
#define     NUF_IDE_NUM_PHYSICAL        -3102      /* NUM_PHYSICAL_DRIVES set error. */
#define     NUF_IDE_LOG_TABLE           -3103      /* LOG_DISK_TABLE setup error. */
#define     NUF_IDE_PHYS_TALBE          -3104      /* PHYS_DISK_TABLE setup error. */
#define     NUF_IDE_INITIALIZE          -3105      /* Initialize error. See c_s[].err_code */
#define     NUF_IDE_NOT_SETCOUNT        -3106      /* Read/Write sector count is zero. */
#define     NUF_IDE_NOT_LOG_OPENED      -3107      /* Logical drive is not opened. */
#define     NUF_IDE_NOT_PHYS_OPENED     -3108      /* Physical drive is not opened. */
#define     NUF_IDE_DISK_SIZE           -3109      /* illegal partition size. */
#define     NUF_IDE_FAT_TYPE            -3110      /* illegal FAT type. */
#define     NUF_IDE_NO_DOSPART          -3111      /* NO DOS partition in disk. */
#define     NUF_IDE_NO_EXTPART          -3112      /* NO Extension partition in disk. */
#define     NUF_IDE_NOT_CAPACITY        -3113      /* Partition capacity error. */
#define     NUF_IDE_OVER_PART           -3114      /* Over the partition end sectors. */
#define     NUF_IDE_MAX_BLOCKS          -3115      /* More than max blocks access. */
#define     NUF_IDE_RESET               -3116      /* Controller reset failed in initialize. */
#define     NUF_IDE_DIAG_FAILED         -3117      /* Drive diagnostic failed in initialize. */
#define     NUF_IDE_SETMULTIPLE         -3118      /* Set multiple mode command failed. */
#define     NUF_IDE_INITPARMS           -3119      /* initialize parmaters failed in initialize */
#define     NUF_IDE_NOT_READY           -3120      /* Drive not ready. */
#define     NUF_IDE_CMDERROR            -3121      /* IDE command error. See error register. */
#define     NUF_IDE_BUSERROR            -3122      /* DRQ should be asserted but it isn't. */
#define     NUF_IDE_EVENT_TIMEOUT       -3123      /* Event timeout. */


/* Device driver dispatch table */
typedef struct _pc_bdevsw
{
    /* Specify a lock number for this entry. If a driver controls multiple
       drives this should be set to the lowest driveno controlled by that
       driver for each drive controlled. see pc_drive_enter in lowl.c
       Note: In non multitasking or coarse grained multitasking ignore this
    */
    INT16   lock_no;

    /* Block IO open call. Opens a drive and sets up partitions */
    INT     (*open_proc)(UINT16 driveno);

    /* Special open call. Opens a drive doesn't set up partitions */
    INT     (*raw_open_proc)(UINT16 driveno);

    /* Close */
    INT     (*close_proc)(UINT16 driveno);

    /* Read & Write sector offset from partition base. If raw_open the
       logical == physical. */
    INT     (*io_proc)(UINT16 driveno, UINT32 sector, VOID *buffer, UINT16 count, INT reading);

    /* ioctl. User function call to device driver. RTFS does nothing
       with this but we have provided the hook for device maintenance 
       (such as format and special error returns */
    INT     (*ioctl_proc)(UINT16 driveno, UINT16 command, VOID *buffer);

    INT     (*dskchk_proc)(UINT16 driveno);

} _PC_BDEVSW;


/* File system dispatch table */
typedef struct _nf_fstbl
{
    STATUS (* fs_init)(INT16  driveno);

} _NUF_FSTBL;


/* Partition table descriptions. */
typedef struct ptable_entry
{
    UINT8       boot;                /* BootSignature */
    UINT8       s_head;              /* StartHead */
    UINT8       s_sec;               /* StartSector(Bit0-5) Bit6 and 7 are cylinder number */
    UINT8       s_cyl;               /* StartCylinder Upper two bit of starting clyinder number are in StartSector field. */
    UINT8       p_typ;               /* Partition Type */
    UINT8       e_head;              /* EndHead */
    UINT8       e_sec;               /* EndSector(Bit0-5) Bit6 and 7 are cylinder number*/
    UINT8       e_cyl;               /* EndCylinder Upper two bit of ending clyinder number are in StartSector field. */
    UINT32      r_sec;               /* Relativity sector */
    UINT32      p_size;              /* Size of partition */

} PTABLE_ENTRY;


/* (Master) Boot Record structure */
typedef struct ptable
{
    PTABLE_ENTRY    ents[4];        /* Entry table */
    UINT16          signature;      /* should be 0xAA55 */

} PTABLE;


/* NUCLEUS file constants that are needed to create IN_DATA.C and are
   needed by NUCLEUS File.

   The following  Values are user changable. See below for instructions:

    NUF_FILE_SYSTEM_MUTEX
    NUF_FLOPPY_EVENT .. or .. NUF_FIRST_EVENT_NUMBER
    NUF_FATSIZE_A
    NUF_FATSIZE_B
    NUF_FATSIZE_C
    NUF_FATSIZE_D
    NUF_FATSIZE_E
*/

/* NUCLEUS - fs_user is a function call under nucleus. See pc_users.c */
#define fs_user ((PFILE_SYSTEM_USER)(fs_current_user_structure()))


/* Nucleus FILE uses event groups to synchronize access to file system objects,
   via calls to NU_Retrieve_Events() and NU_Set_Events() in Nucleus PLUS.

   NUF_FIRST_EVENT_NUMBER is used by pc_memory_init to assign event handles to
   various file system objects. 
    
   If device drivers are being built, we grab a few event channels for interrupt
   processing in the drivers.

   NUF_NUM_EVENTS is the total number of events used by NUCLEUS File.
   IN_System_Event_Groups must be at least this large.


*/

#define NUF_FIRST_EVENT_NUMBER      0        /* May be changed */
#if (EBS_FLOPPY)
#define NUF_FLOPPY_EVENT            (NUF_FIRST_EVENT_NUMBER + (NDRIVES*3) + NFINODES)
#define NUF_FLOPPY_TIMER_EVENT      (NUF_FLOPPY_EVENT+1)
#define NUF_NUM_EVENTS              (2 + (NDRIVES*3) + NFINODES)
#else
#define NUF_NUM_EVENTS              ((NDRIVES*3) + NFINODES)
#endif

/* File allocation table buffer size in blocks. We reserve 9 blocks for the
   A & B drives. (enough for a 1.44M floppy). 32 blocks each for the C and
   D drives. And 9 blocks for the ram disk. Modify these values to suit your
   application. Make the C and D FAT Buffers larger or smaller depending on
   your hard disk configuration. Larger buffers give better performance, but
   making the buffers larger than the actual fat provides no benefit and wastes
   memory. 
*/

#define HEAPGRAN                1

#define NUF_FATSIZE_A           9
#define NUF_FATSIZE_B           9
#define NUF_FATSIZE_C           48 //32
#define NUF_FATSIZE_D           32
#define NUF_FATSIZE_E           32
#define NUF_FATSIZE_F           32

#if (RAMDISK)
extern  NU_PARTITION_POOL       NUF_RAMDISK_PARTITION;
#endif

#define ALLOC_SIZE              20
#define PARTITION_SIZE          20

#if (RAMDISK)
/* The ram disk memory is allocate from the fixed parttition at
   NUF_RAMDISK_PARTITION . There are NUM_RAMDISK_PAGES equal sized
   memory blocks of size NUF_RAMDISK_PARTITION_SIZE. This value need not
   be changed. To modify the ram disk size you should modify 
   RAMDISK_PAGE_SIZE */
#define NUF_RAMDISK_PARTITION_SIZE  (RAMDISK_PAGE_SIZE*512/HEAPGRAN)
#endif


/*  This declaration is only used in pc_user.c  */
signed int  NUFP_Current_Task_ID(void);

/*  This external declaration is for the conversion between 
    event group IDs and the pointers used by Nucleus PLUS.  */
extern  NU_EVENT_GROUP  NUFP_Events[NUF_NUM_EVENTS];


#define FAT_FILE_SYSTEM         1

/* Do not change the path for proto.h. */
#include "proto.h"



#endif   /* __PCDISK__ */



/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         4/3/04 12:13:19 AM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  3    mpeg      1.2         1/7/04 4:15:04 PM      Tim Ross        CR(s) 
 *        8181 : Define new error codes to properly reflect formatting errors.
 *  2    mpeg      1.1         8/22/03 6:48:30 PM     Dave Moore      SCR(s) 
 *        7350 :
 *        Fixed compiler warnings
 *        
 *        
 *  1    mpeg      1.0         8/22/03 5:26:28 PM     Dave Moore      
 * $
 * 
 *    Rev 1.1   22 Aug 2003 17:48:30   mooreda
 * SCR(s) 7350 :
 * Fixed compiler warnings
 * 
 * 
 *    Rev 1.0   22 Aug 2003 16:26:28   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (structs and config settings)
 * 
 *
 ****************************************************************************/

