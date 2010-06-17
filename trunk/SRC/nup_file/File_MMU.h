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
* FILE NAME                                         VERSION     
*                                                                       
*       file_mmu.h                                    2.5 
*                                                                       
* COMPONENT                                                             
*                                                                       
*      Nucleus FILE                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*      MMU support for Nucleus FILE.                             
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*      pcdisk.h
*                                                                       
*************************************************************************/



#ifndef __FILE_MMU__
#define __FILE_MMU__ 1

#include "pcdisk.h"

/* Define Supervisor and User mode functions */
#if (!defined(NU_SUPERV_USER_MODE)) || (NU_SUPERV_USER_MODE < 1)
#define NU_IS_SUPERVISOR_MODE() (NU_TRUE)
#define NU_SUPERVISOR_MODE() ((void) 0)
#define NU_USER_MODE() ((void) 0)
#define NU_SUPERV_USER_VARIABLES    
#endif /* NU_SUPERV_USER_MODE */

#undef PC_FS_ENTER
#undef PC_FS_EXIT

#if (NUM_USERS == 1)

/* These macros are API call prolog and epilogs. In multitasking mode
they lock/unlock the RTFS semaphore and check if the user is registered
via pc_rtfs_become_user() respectively. In single tasking mode they do nothing */

#define PC_FS_ENTER()       NU_SUPERV_USER_VARIABLES  \
                            NU_SUPERVISOR_MODE();


#define PC_FS_EXIT()        NU_USER_MODE();


#else /* Num users > 1 */

/* These macros are API call prolog and epilogs. In multitasking mode
they lock/unlock the RTFS semaphore and check if the user is registered
via pc_rtfs_become_user() respectively. In single tasking mode they do nothing */

#define PC_FS_ENTER()           UINT16 process_flags; \
                                NU_SUPERV_USER_VARIABLES  \
                                NU_SUPERVISOR_MODE(); \
                                process_flags = pc_fs_enter();

#define PC_FS_EXIT()            pc_fs_exit(process_flags); \
                                NU_USER_MODE();


#endif  /* Num users > 1 */

#endif   /* __FILE_MMU__ */

