/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        swconfig.h
 *
 *
 * Description:     Top Level Software Defintions for C Files
 *
 *
 * Author:          Bobby Bradford
 *
 ****************************************************************************/
/* $Header: swconfig.h, 27, 5/26/04 8:13:21 PM, Steven Shen$
 ****************************************************************************/

#ifndef __STBCFG_H__
   /* Should generate a "warning" here, but don't recall how */
   #include "stbcfg.h"
#endif   /*#ifndef __STBCFG_H__*/

#ifndef  __SWCONFIG_H__
#define  __SWCONFIG_H__

/****************************************************************************
 * Basic Structure of this file is as follows ...
 *
 * SECTION 1 - Definition of SOFTWARE CONFIG parameter values.  This section
 *    will define all of the possible values that a software configuration
 *    parameter can take.
 *
 * SECTION 2 - Optional inclusion of translated SWCONFIG file.  To allow for
 *    the transition to using software configuration files, if no software
 *    config file is specified on the command line, a default value of NONE
 *    will be assumed for directory tree purposes, no translation will take
 *    place, no translated file will be included, and the default values
 *    that are defined in SECTION 3 will be used.
 *
 * SECTION 3 - Defintion of DEFAULT value for all SOFTWARE configuration
 *    features.  These values will be used if a SWCONFIG file does not
 *    specify a feature, or if the SWCONFIG option is not specified on the
 *    make command line.
 *
 ****************************************************************************/

/****************************************************************************
 * SECTION 1 - Defintion of Software Config Parameter Values
 *
 * These values are now generated dynamically during the build from the 
 * file CONFIGS/SWCONFIG.CFG using the CFGTOH tool.
 ****************************************************************************/
#include "swopts.h"

/********************************************/
/* Generic Driver Task Priorities and Names */
/********************************************/
#if (RTOS==UCOS)
#include "ucosconf.h"
#else 
#if (RTOS==UCOS2)
#include "..\ucos2bsp\ucos2conf.h"
#else
#include "taskprio.h"
#endif
#endif

/****************************************************************************
 * SECTION 2 - Optional inclusion of translated SWCONFIG file
 ****************************************************************************/
   #if (SWCONFIG_INCL == 1)
      #include "swboxcfg.h"
   #endif   /* #if (SWCONFIG_INCL == 1) */

/****************************************************************************
 * SECTION 3 - Definition of Default Values
 *
 * These values are now generated dynamically during the build from the 
 * file CONFIGS/SWCONFIG.CFG using the CFGTOH tool.
 ****************************************************************************/
#include "swdefault.h"

/*
 * Define IMAGE_TYPE variations which must be defined for 'C' files
 */
#define GENERIC       1111
#define BOOT          2222
#define BOOTEXT       4444

/*
 * Define CLX_ENTRY_POINT variations which must be defined for 'C' files
 */
#define MIN_INIT      1
#define PARTIAL_INIT  2
#define FULL_INIT     3

#endif   /* #ifndef  __SWCONFIG_H__ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  27   mpeg      1.26        5/26/04 8:13:21 PM     Steven Shen     CR(s) 
 *        9273 9274 : Removed the path for the include file ucosconf.h.
 *  26   mpeg      1.25        5/26/04 3:31:50 AM     Steven Shen     CR(s) 
 *        9273 9274 : Fix the path of the header file for NDS uCOS building.
 *  25   mpeg      1.24        3/4/04 7:06:16 AM      Ian Mitchell    CR(s) 
 *        8460 8461 : Removed the path for the include file ucosconf.h.
 *        
 *  24   mpeg      1.23        11/14/03 7:48:13 AM    Ganesh Banghar  CR(s): 
 *        7926 added deinitions for UCOS.
 *  23   mpeg      1.22        7/9/03 3:28:26 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  22   mpeg      1.21        6/24/03 6:35:32 PM     Tim White       SCR(s) 
 *        6831 :
 *        Add flash, hsdp, demux, OSD, and demod support to codeldrext
 *        
 *        
 *  21   mpeg      1.20        11/20/02 9:36:36 AM    Dave Wilson     SCR(s) 
 *        4904 :
 *        Replaced previous header with a wrapper which pulls in the 
 *        automatically
 *        generated config files headers.
 *        
 *  20   mpeg      1.19        11/15/02 11:42:18 AM   Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Made SOFTWARE_AC_SPDIF_FIX the default fix for AC3 pass through
 *        
 *  19   mpeg      1.18        11/6/02 5:41:32 PM     Miles Bintz     SCR(s) 
 *        4913 :
 *        Added demod_lock_timeout config option for conservative or 
 *        development timeouts
 *        
 *        
 *  18   mpeg      1.17        11/1/02 3:45:30 PM     Dave Wilson     SCR(s) 
 *        4878 :
 *        Removed previous VMC_MASK_PTS_BEFORE_SYNC by redefining it such that 
 *        it
 *        includes the mainline macroblock-based error recovery microcode 
 *        (which now
 *        includes the PTS masking by default).
 *        
 *  17   mpeg      1.16        10/21/02 4:42:16 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Added define for the Software AC3 fix.
 *        
 *  16   mpeg      1.15        9/13/02 11:01:58 AM    Tim White       SCR(s) 
 *        4600 :
 *        Add comments to the pawser microcode swconfig options.
 *        
 *        
 *  15   mpeg      1.14        8/9/02 6:30:18 PM      Bob Van Gulick  SCR(s) 
 *        4362 :
 *        Define DRIP_TEST to be NO in cases where it is not defined in the 
 *        swconfig file
 *        
 *        
 *  14   mpeg      1.13        7/18/02 3:05:56 PM     Dave Wilson     SCR(s) 
 *        4231 :
 *        Changed the default video microcode type to the macroblock-based 
 *        error
 *        recovery version.
 *        
 *  13   mpeg      1.12        7/10/02 4:07:22 PM     Dave Wilson     SCR(s) 
 *        3954 :
 *        Added 2 new video microcode variants - VMC_MASK_PTS_BEFORE_SYNC and
 *        VMC_MACROBLOCK_ERROR_RECOVERY.
 *        
 *  12   mpeg      1.11        6/21/02 11:35:32 AM    Miles Bintz     SCR(s) 
 *        4001 :
 *        Added define for Cable demod annex mode
 *        
 *  11   mpeg      1.10        5/30/02 12:30:26 PM    Tim White       SCR(s) 
 *        3899 :
 *        Add new -DDMA_MUX microcode build option support.
 *        
 *        
 *  10   mpeg      1.9         5/21/02 3:48:06 PM     Tim White       SCR(s) 
 *        3642 :
 *        Added PARSER_PTS_OFFSET=NO, added PARSER_PES_PID_INT=YES, changed 
 *        PARSER_BAD_PES_INT=NO to YES, changed DISCARD_BAD_PES=NO to YES, 
 *        changed PARSER_PCR_PID_INT=NO to YES.
 *        
 *        
 *  9    mpeg      1.8         5/15/02 6:12:30 PM     Matt Korte      SCR(s) 
 *        2438 :
 *        Changed PCM to PCM_OPTION to avoid name collision. Changed
 *        GENERIC to GENERIC_PCM for the same reason.
 *        
 *        
 *  8    mpeg      1.7         5/15/02 3:36:42 PM     Matt Korte      SCR(s) 
 *        2438 :
 *        Put for Steve Glennon, changes to support the PCM flag in swconfig's
 *        
 *  7    mpeg      1.6         4/30/02 5:56:30 PM     Dave Wilson     SCR(s) 
 *        3243 :
 *        Added default value for CANAL_AUDIO_HANDLING
 *        
 *  6    mpeg      1.5         4/30/02 5:17:30 PM     Tim White       SCR(s) 
 *        3664 :
 *        Added -dBAD_PES_INT parser microcode build option support.
 *        
 *        
 *  5    mpeg      1.4         4/26/02 3:21:32 PM     Tim White       SCR(s) 
 *        3562 :
 *        Use YES/NO instead of 1/0 for swconfig build options.
 *        
 *        
 *  4    mpeg      1.3         4/5/02 10:08:04 AM     Dave Wilson     SCR(s) 
 *        3496 :
 *        Added VIDEO_UCODE_TYPE definitions and default value
 *        
 *  3    mpeg      1.2         3/25/02 2:30:48 PM     Tim White       SCR(s) 
 *        3433 :
 *        Changed YES's to 1's and NO's to 0's.  Also made the default 
 *        microcode selection
 *        build NDS_ICAM support since it's required for otv12ctl.
 *        
 *        
 *  2    mpeg      1.1         3/25/02 10:29:06 AM    Tim White       SCR(s) 
 *        3433 :
 *        Added defaults for Parser Microcode software configuration options.
 *        
 *        
 *  1    mpeg      1.0         2/25/02 8:47:44 AM     Bobby Bradford  
 * $
 * 
 *    Rev 1.22   09 Jul 2003 14:28:26   whiteth
 * SCR(s) 6901 :
 * Phase 3 codeldrext drop.
 * 
 * 
 *    Rev 1.21   24 Jun 2003 17:35:32   whiteth
 * SCR(s) 6831 :
 * Add flash, hsdp, demux, OSD, and demod support to codeldrext
 * 
 * 
 *    Rev 1.20   20 Nov 2002 09:36:36   dawilson
 * SCR(s) 4904 :
 * Replaced previous header with a wrapper which pulls in the automatically
 * generated config files headers.
 * 
 *    Rev 1.18   06 Nov 2002 17:41:32   bintzmf
 * SCR(s) 4913 :
 * Added demod_lock_timeout config option for conservative or development timeouts
 * 
 * 
 *    Rev 1.17   01 Nov 2002 15:45:30   dawilson
 * SCR(s) 4878 :
 * Removed previous VMC_MASK_PTS_BEFORE_SYNC by redefining it such that it
 * includes the mainline macroblock-based error recovery microcode (which now
 * includes the PTS masking by default).
 * 
 *    Rev 1.16   21 Oct 2002 15:42:16   velusws
 * SCR(s) 4790 :
 * Added define for the Software AC3 fix.
 * 
 *    Rev 1.15   13 Sep 2002 10:01:58   whiteth
 * SCR(s) 4600 :
 * Add comments to the pawser microcode swconfig options.
 * 
 * 
 *    Rev 1.14   09 Aug 2002 17:30:18   vangulr
 * SCR(s) 4362 :
 * Define DRIP_TEST to be NO in cases where it is not defined in the swconfig file
 * 
 * 
 *    Rev 1.13   18 Jul 2002 14:05:56   dawilson
 * SCR(s) 4231 :
 * Changed the default video microcode type to the macroblock-based error
 * recovery version.
 * 
 *    Rev 1.12   10 Jul 2002 15:07:22   dawilson
 * SCR(s) 3954 :
 * Added 2 new video microcode variants - VMC_MASK_PTS_BEFORE_SYNC and
 * VMC_MACROBLOCK_ERROR_RECOVERY.
 * 
 *    Rev 1.11   21 Jun 2002 10:35:32   bintzmf
 * SCR(s) 4001 :
 * Added define for Cable demod annex mode
 * 
 *    Rev 1.10   30 May 2002 11:30:26   whiteth
 * SCR(s) 3899 :
 * Add new -DDMA_MUX microcode build option support.
 * 
 * 
 *    Rev 1.9   21 May 2002 14:48:06   whiteth
 * SCR(s) 3642 :
 * Added PARSER_PTS_OFFSET=NO, added PARSER_PES_PID_INT=YES, changed PARSER_BAD_PES_INT=NO to YES, changed DISCARD_BAD_PES=NO to YES, changed PARSER_PCR_PID_INT=NO to YES.
 * 
 * 
 *    Rev 1.8   15 May 2002 17:12:30   kortemw
 * SCR(s) 2438 :
 * Changed PCM to PCM_OPTION to avoid name collision. Changed
 * GENERIC to GENERIC_PCM for the same reason.
 * 
 * 
 *    Rev 1.7   15 May 2002 14:36:42   kortemw
 * SCR(s) 2438 :
 * Put for Steve Glennon, changes to support the PCM flag in swconfig's
 * 
 *    Rev 1.6   30 Apr 2002 16:56:30   dawilson
 * SCR(s) 3243 :
 * Added default value for CANAL_AUDIO_HANDLING
 * 
 *    Rev 1.5   30 Apr 2002 16:17:30   whiteth
 * SCR(s) 3664 :
 * Added -dBAD_PES_INT parser microcode build option support.
 * 
 * 
 *    Rev 1.4   26 Apr 2002 14:21:32   whiteth
 * SCR(s) 3562 :
 * Use YES/NO instead of 1/0 for swconfig build options.
 * 
 * 
 *    Rev 1.3   05 Apr 2002 10:08:04   dawilson
 * SCR(s) 3496 :
 * Added VIDEO_UCODE_TYPE definitions and default value
 * 
 *    Rev 1.2   25 Mar 2002 14:30:48   whiteth
 * SCR(s) 3433 :
 * Changed YES's to 1's and NO's to 0's.  Also made the default microcode selection
 * build NDS_ICAM support since it's required for otv12ctl.
 * 
 * 
 *    Rev 1.1   25 Mar 2002 10:29:06   whiteth
 * SCR(s) 3433 :
 * Added defaults for Parser Microcode software configuration options.
 * 
 * 
 *    Rev 1.0   Feb 25 2002 08:47:44   bradforw
 * SCR(s) 3201 :
 * 
 *
 ****************************************************************************/

