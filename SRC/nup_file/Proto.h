/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       proto.h
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: Proto.h, 2, 4/2/04 9:31:32 PM, Nagaraja Kolur$
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
*       PROTO.H                                   2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*      Nucleus FILE                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*      Function prototypes for Nucleus FILE.                             
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*      None.                                                            
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*      None.                                                            
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*      None.                                                            
*                                                                       
*************************************************************************/

#include "pcdisk.h"
/* =========== File NU_FILE.C ================ */
STATUS          NU_Open_Disk(CHAR *path);
STATUS          NU_Close_Disk(CHAR *path);
VOID            NU_Disk_Abort(CHAR *path);
STATUS          NU_Make_Dir(CHAR *name);
VOID            _synch_file_ptrs(PC_FILE *pfile);
STATUS          NU_Open(CHAR *name, UINT16 flag, UINT16 mode);
INT32           NU_Read(INT fd, CHAR *buf, INT32 count);
INT32           NU_Write(INT fd, CHAR *buf, INT32 count);
INT32           NU_Seek(INT fd, INT32 offset, INT16 origin);
STATUS          NU_Truncate(INT fd, INT32 offset);
STATUS          _po_lseek(PC_FILE *pfile, INT32 offset, INT16 origin);
STATUS          _po_flush(PC_FILE *pfile);
STATUS          NU_Flush(INT fd);
STATUS          NU_Close(INT fd);
STATUS          NU_Set_Attributes(CHAR *name, UINT8 newattr);
STATUS          NU_Get_Attributes(UINT8 *attr, CHAR *name);
STATUS          NU_Rename(CHAR *name, CHAR *newname);
STATUS          NU_Delete(CHAR *name);
STATUS          NU_Remove_Dir(CHAR *name);
UINT32          pc_fat_size(UINT16 bytepsec, UINT16 nreserved, UINT16 cluster_size, 
                        UINT16 n_fat_copies, UINT16 root_entries, UINT32 volume_size, UINT16 fasize);
STATUS          NU_Format(INT16 driveno, FMTPARMS *pfmt);
STATUS          NU_FreeSpace(CHAR *path, UINT8 *secpcluster, UINT16 *bytepsec, 
                             UINT32 *freecluster, UINT32 *totalcluster);
STATUS          NU_Get_First(DSTAT *statobj, CHAR *pattern);
STATUS          NU_Get_Next(DSTAT *statobj);
VOID            NU_Done(DSTAT *statobj);
STATUS          NU_Set_Default_Drive(UINT16 driveno);
UINT16          NU_Get_Default_Drive(VOID);
STATUS          NU_Set_Current_Dir(CHAR *path);
STATUS          NU_Current_Dir(UINT8 *drive, CHAR *path);


/* =========== File APIEXT.C ================ */
INT16           pc_cluster_size(UINT8 *drive);
UINT32          po_extend_file(INT fd, UINT32 n_clusters, INT16 method);
UINT32          pc_find_contig_clusters(DDRIVE *pdr, UINT32 startpt,
                                UINT32  *pchain, UINT32 min_clusters, INT16 method);
/* =========== File APIUTIL.C ================ */
STATUS          pc_dskinit(INT16 driveno);
STATUS          pc_idskclose(INT16 driveno);
PC_FILE        *pc_fd2file(INT fd);
INT             pc_allocfile(VOID);
VOID            pc_freefile(INT fd);
VOID            pc_free_all_fil(DDRIVE *pdrive);
INT16           pc_log_base_2(UINT16 n);
DROBJ          *pc_get_cwd(DDRIVE *pdrive);
VOID            pc_upstat(DSTAT *statobj);

/* =========== File BLOCK.C ================ */
INT             pc_alloc_blk(BLKBUFF **ppblk, DDRIVE *pdrive, UINT32 blockno);
BLKBUFF        *pc_blkpool(DDRIVE *pdrive);
VOID            pc_free_all_blk(DDRIVE *pdrive);
VOID            pc_free_buf(BLKBUFF *pblk, INT waserr);
STATUS          pc_read_blk(BLKBUFF **pblk, DDRIVE *pdrive, UINT32 blockno);
STATUS          pc_write_blk(BLKBUFF *pblk);
STATUS          pc_init_blk(DDRIVE *pdrive, UINT32 blockno);

/* =========== File DROBJ.C ================ */
STATUS          pc_fndnode(INT16 driveno, DROBJ **pobj, UINT8 *path);
STATUS          pc_get_inode(DROBJ **pobj, DROBJ *pmom, UINT8 *filename);
STATUS          pc_next_inode(DROBJ *pobj, DROBJ *pmom, UINT8 *filename, INT attrib);
UINT8           chk_sum(UINT8 *sname);
INT             pc_cre_longname(UINT8 *filename, LNAMINFO *linfo);
INT             pc_cre_shortname(UINT8 *filename, UINT8  *fname, UINT8 *fext);
VOID            lnam_clean(LNAMINFO *linfo, BLKBUFF *rbuf);
STATUS          pc_findin(DROBJ *pobj, UINT8 *filename);
STATUS          pc_get_mom(DROBJ **pmom, DROBJ *pdotdot);
DROBJ          *pc_mkchild(DROBJ *pmom);
STATUS          pc_mknode(DROBJ **pobj, DROBJ *pmom, UINT8 *filename, UINT8 *fileext, UINT8 attributes);
STATUS          pc_insert_inode(DROBJ *pobj, DROBJ *pmom, UINT8 *filename, INT longfile);
VOID            pc_del_lname_block(DROBJ *pobj);
STATUS          pc_renameinode(DROBJ *pobj, DROBJ *pmom, UINT8 *fnambuf, 
                                    UINT8 *fextbuf, UINT8 *newfilename, INT longdest);
STATUS          pc_rmnode(DROBJ *pobj);
STATUS          pc_update_inode(DROBJ *pobj, INT setdate);
DROBJ          *pc_get_root(DDRIVE *pdrive);
UINT32          pc_firstblock(DROBJ *pobj);
STATUS          pc_next_block(DROBJ *pobj);
STATUS          pc_l_next_block(UINT32 *nxt, DDRIVE *pdrive, UINT32 curblock);
VOID            pc_marki(FINODE *pfi, DDRIVE *pdrive, UINT32 sectorno, INT16  index);
FINODE         *pc_scani(DDRIVE *pdrive, UINT32 sectorno, INT16 index);
DROBJ          *pc_allocobj(VOID);
FINODE         *pc_alloci(VOID);
VOID            pc_free_all_i(DDRIVE *pdrive);
VOID            pc_freei(FINODE *pfi);
VOID            pc_freeobj(DROBJ *pobj);
VOID            pc_dos2inode (FINODE *pdir, DOSINODE *pbuff);
VOID            pc_init_inode(FINODE *pdir, UINT8 *filename, UINT8 *fileext, 
                                UINT8 attr, UINT32 cluster, UINT32 size, DATESTR *crdate);
VOID            pc_ino2dos (DOSINODE *pbuff, FINODE *pdir);
INT             pc_isavol(DROBJ *pobj);
INT             pc_isadir(DROBJ *pobj);
INT             pc_isroot(DROBJ *pobj);

#if (EBS_IDE)
/* =========== File PC_ATA.C ================ */
INT             pc_get_ataparams(INT16 driveno, FMTPARMS *pfmt);
#endif

/* =========== File LOWL.C ================ */
STATUS          pc_alloc_chain(UINT32 *, DDRIVE *pdr, UINT32 *pstart_cluster, UINT32 n_clusters);
STATUS          pc_find_free_cluster(UINT32 *, DDRIVE *pdr, UINT32 startpt, UINT32 endpt);
STATUS          pc_clalloc(UINT32 *clno, DDRIVE *pdr);
STATUS          pc_clgrow(UINT32 *nxt,DDRIVE *pdr, UINT32  clno);
STATUS          pc_clnext(UINT32 *nxt, DDRIVE *pdr, UINT32  clno);
STATUS          pc_clrelease(DDRIVE   *pdr, UINT32  clno);
STATUS          pc_faxx(DDRIVE *pdr, UINT32 clno, UINT32 *pvalue);
STATUS          pc_flushfat(DDRIVE *pdr);
STATUS          pc_freechain(DDRIVE *pdr, UINT32 cluster);
INT32           pc_get_chain(DDRIVE *pdr, 
                            UINT32 start_cluster, 
                            UINT32 *pnext_cluster,
                            UINT32 n_clusters);
STATUS          pc_pfaxx(DDRIVE *pdr, UINT32  clno, UINT32  value);
STATUS          pc_pfswap(UINT8 FAR **, DDRIVE *pdr, UINT32 index, INT for_write);
STATUS          pc_pfpword(DDRIVE *pdr, UINT16 index, UINT16 *pvalue);
STATUS          pc_pfgword(DDRIVE *pdr, UINT16 index, UINT16 *value);
STATUS          pc_pfflush(DDRIVE *pdr);
STATUS          pc_clzero(DDRIVE *pdrive, UINT32 cluster);
DDRIVE         *pc_drno2dr(INT16 driveno);
STATUS          pc_dskfree(INT16 driveno, INT  unconditional);
UINT32          pc_ifree(INT16 driveno);
UINT32          pc_sec2cluster(DDRIVE *pdrive, UINT32 blockno);
UINT16          pc_sec2index(DDRIVE *pdrive, UINT32 blockno);
UINT32          pc_cl2sector(DDRIVE *pdrive, UINT32 cluster);
STATUS          pc_update_fsinfo(DDRIVE *pdr);

/* =========== File NUFP.C ================ */
INT             NUFP_Current_Task_ID(VOID);
VOID            NUFP_Remove_User(INT task_id);
INT             NUFP_Initialize(VOID);

/* =========== File PC_ERROR.C ================ */
VOID            pc_report_error(INT error_number);

/* =========== File PC_LOCKS.C ================ */
#if (NUM_USERS > 1)
UINT16          pc_fs_enter(VOID);
VOID            pc_fs_exit(UINT16 restore_state);
#if (LOCK_METHOD == 2)
VOID            fs_suspend_task(VOID);
INT             fs_lock_task(VOID);
VOID            fs_unlock_task(INT state_var);
VOID            fs_release(VOID);
VOID            fs_reclaim(VOID);
WAIT_HANDLE_TYPE    pc_alloc_lock(VOID);
VOID            pc_wait_lock(WAIT_HANDLE_TYPE wait_handle);
VOID            pc_wake_lock(WAIT_HANDLE_TYPE wait_handle);
VOID            pc_drive_enter(INT16 driveno, INT exclusive);
VOID            pc_drive_exit(INT16 driveno);
VOID            pc_inode_enter(FINODE *pinode, INT exclusive);
VOID            pc_inode_exit(FINODE *pinode);
VOID            pc_fat_enter(INT16 driveno);
VOID            pc_fat_exit(INT16 driveno);
VOID            pc_drive_io_enter(INT16 driveno);
VOID            pc_drive_io_exit(INT16 driveno);
VOID            pc_generic_enter(LOCKOBJ *plock, INT exclusive);
VOID            pc_generic_exit(LOCKOBJ *plock);
#endif
#endif

/* =========== File PC_MEMRY.C ================ */
VOID            *NUF_Alloc(INT nbytes);
INT             pc_memory_init(VOID);
VOID            pc_memory_close(VOID);
DROBJ          *pc_memory_drobj(DROBJ *pobj);
FINODE         *pc_memory_finode(FINODE *pinode);

/* =========== File PC_PART.C ================ */
INT16           ext_partition_init(INT16 driveno, UINT32 *pstart, UINT32 *pend, INT16 max);

/* =========== File PC_UDATE.C ================ */
DATESTR        *pc_getsysdate(DATESTR *pd);

/* =========== File PC_USERS.C ================ */
VOID            pc_free_all_users(INT16 driveno);
PFILE_SYSTEM_USER fs_current_user_structure(VOID);
#if (NUM_USERS> 1)
INT             NU_Become_File_User(VOID);
VOID            NU_Release_File_User(VOID);
INT             NU_Check_File_User(VOID);
#endif

/* =========== File FILEUTIL.C ================ */
INT             pc_allspace(UINT8 *p, INT i);
VOID            copybuff(VOID *vto, VOID *vfrom, INT size);
VOID            pc_cppad(UINT8 *to, UINT8 *from, INT size);
INT             pc_isdot(UINT8 *fname, UINT8 *fext);
INT             pc_isdotdot(UINT8 *fname, UINT8 *fext);
VOID            pc_memfill(VOID *vto, INT size, UINT8 c);
UINT8          *pc_parsedrive(INT16 *driveno, UINT8  *path);
INT             pc_parsenewname(DROBJ *pobj, UINT8 *name, 
                            UINT8 *ext, VOID **new_name, 
                            VOID **new_ext, UINT8 *fname);
INT             pc_next_fparse(UINT8 *filename);
INT             pc_fileparse(UINT8 *filename, UINT8 *fileext, VOID *pfname, VOID *pfext);
UINT8          *pc_nibbleparse(UINT8 *topath);
STATUS          pc_parsepath(VOID **topath, VOID **pfname, VOID **pfext, UINT8 *path);
INT             pc_patcmp(UINT8 *disk_fnam, UINT8 *in_fnam);
VOID            pc_strcat(UINT8 *to, UINT8 *from);
INT             pc_use_wdcard(UINT8 *code);
INT             pc_use_upperbar(UINT8 *code);
INT             pc_checkpath(UINT8 *code, INT vol);
VOID            _swap16(UINT16 *to, UINT16 *from);
VOID            _swap32(UINT32 *to, UINT32 *from);
VOID            _through16(UINT16 *to, UINT16 *from);
VOID            _through32(UINT32 *to, UINT32 *from);
VOID            fswap16(UINT16 *x, UINT16 *y);
VOID            fswap32(UINT32 *x, UINT32 *y);
VOID            pc_ncpbuf(UINT8 *to, UINT8 *from, INT size);


/* =========== File PC_UNICD.C ================ */
UINT8           uni2asc(UINT8 *ptr);
UINT8           asc2uni(UINT8 *ptr, UINT8 ascii);

/* =========== File FileInit.C ================ */
STATUS          file_init(VOID);


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 9:31:32 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:48:58 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:48:58   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Function Prototypes)
 * 
 *
 ****************************************************************************/

