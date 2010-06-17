/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           FLASHCFG.H                                           */
/*                                                                          */
/* Description:        Public header file for the flash memory operations   */
/*                                                                          */
/* Author:             Anzhi Chen                                           */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: flashcfg.h, 2, 6/4/01 6:53:18 PM, Dave Moore$
$Log: 
 2    mpeg      1.1         6/4/01 6:53:18 PM      Dave Moore      SCR(s) 2017 
       :
       
       
 1    mpeg      1.0         1/31/01 2:27:24 PM     Tim White       
$
 * 
 *    Rev 1.1   04 Jun 2001 17:53:18   mooreda
 * SCR(s) 2017 :
 * 
 * 
 *    Rev 1.0   31 Jan 2001 14:27:24   whiteth
 * Initial revision.
 * 
 *    Rev 1.0   31 Jan 2001 14:25:52   whiteth
 * Initial revision.
 * 
 *    Rev 1.31   19 Oct 1999 16:00:50   achen
 * Added prototype for flash_erase().
 * 
 *    Rev 1.29   19 Oct 1999 14:43:26   achen
 * Recovered the file contents that was overwritten by last put. 
 * 
 *    Rev 1.27   13 Oct 1999 15:51:00   whiteth
 * Use new FlashInfo structure instead of hardcoded flash ROM addresses.
 * 
 *    Rev 1.26   11 Aug 1999 16:25:26   dawilson
 * Added definitions of SUCCESS and FAILURE to allow non-OpenTV, non-KAL apps 
 * to use the header and flashrw.c files.
 * 
 *    Rev 1.25   30 Jul 1999 14:43:36   velusws
 * o:
 * changed flash_read(), flash_write(), flash_alloc(), flash_query_size()
 * - to work with nvstore driver
 * removed (commented out) flash id defines. Ids now in nvstore.h
 * Replaced declaration for TRACE_FLS with TRACE_NV in Kal.h
 * - and added define to derive TRACE_FLS from TRACE_NV
 * the changes are highlighted by use of "//" comments demarkating
 * - the old code.
 * 
 *    Rev 1.24   18 Jun 1999 11:40:16   achen
 * Added more members to flash_info.
 * 
 *    Rev 1.23   07 Jun 1999 16:36:58   achen
 * Moved the #endif for #ifdef _FLASH_H to the end of the file.
 * 
 *    Rev 1.22   29 Apr 1999 10:39:54   achen
 * Added FLASH_CODEBLOCK_INFO structure.
 * 
 *    Rev 1.21   25 Jan 1999 16:36:34   achen
 * Added definitions of the trace messages.
 * 
 *    Rev 1.20   07 Oct 1998 15:56:44   achen
 * Added #ifdef stuff so flash download utility will be able to use it.
 * 
 *    Rev 1.19   28 Sep 1998 15:52:08   achen
 * Added device ID for AMD and INTEL flash.
 * 
 *    Rev 1.18   18 Sep 1998 13:37:10   achen
 * Added manufacturer IDs for INTEL and AMD flash.  Added MAGIC_NUMBER.
 * 
 *    Rev 1.17   16 Sep 1998 16:44:56   achen
 * Moved SIZE_SEC_HEADER and DIRHEADER structure to here.
 * 
 *    Rev 1.16   09 Sep 1998 14:15:28   achen
 * Added ifdef _FLASH_H stuff to prevent multiple include of this file.
 * 
 *    Rev 1.15   02 Sep 1998 16:18:16   achen
 * Changed the type of return value of flash_alloc() and return_address of the
 * flash_param structure.
 * 
 *    Rev 1.14   01 Sep 1998 10:56:04   achen
 * Changed source_id to type u_int8.
 * 
 *    Rev 1.13   27 Aug 1998 11:26:24   achen
 * Changed member of flash_params addr to type u_int8 *.
 * 
 *    Rev 1.12   27 Aug 1998 11:12:12   achen
 * Added a new member into flash_params structure.  Move the flash action types
 * from flash.c to here.  Added three more action types for flash file system.
 * 
 *    Rev 1.11   10 Aug 1998 09:20:20   achen
 * Added one more member in flash_params structure.
 * 
 *    Rev 1.10   07 Aug 1998 14:08:08   achen
 * Changed the return type for flash_cleanup().
 * 
 *    Rev 1.9   07 Aug 1998 13:46:48   achen
 * Changed one of the flash_param's member name.
 * 
 *    Rev 1.8   07 Aug 1998 13:32:10   achen
 * Added opentv.h, changed the type for flash_alloc().
 * 
 *    Rev 1.7   29 Jul 1998 11:04:24   achen
 * Added new member offset to the fp structure.
 * 
 *    Rev 1.6   27 Jul 1998 09:27:52   achen
 * Added flash_cleanup() API.
 * 
 *    Rev 1.5   20 Jul 1998 10:44:52   achen
 * Added one more parameter to flash_write() and flash_read().  Added one more API 
 * flash_alloc to allocate flash memory for the config and crypto datas.
 * 
 *    Rev 1.4   15 Jul 1998 15:11:16   achen
 * Added flash_terminate() function.
 * 
 *    Rev 1.3   14 Jul 1998 16:22:36   achen
 * Added return_size member to the flash_params structure.
 * 
 *    Rev 1.2   14 Jul 1998 15:19:46   achen
 * Added flash_params structure.
 * 
 *    Rev 1.1   14 Jul 1998 10:46:28   achen
 * Changed type of flash_init() to bool.
 * 
 *    Rev 1.0   14 Jul 1998 09:49:46   achen
 * Initial revision.
*/
#ifndef _FLASHCFG_H
#define _FLASHCFG_H

#ifndef __OPTVX_H__
#include <basetype.h>
#endif

//#ifndef DLOAD
//#include "custom.h"                             
//#include "opentvx.h"
//#endif

//#define FLASH_ID_PSOSCONFIG 1
//#define FLASH_ID_CRYPTO 2
//#define FLASH_ID_RSSCONFIG 3

#ifndef DLOAD
/* following are the action types that used in flash_task */
#define FLASH_ALLOC 1
#define FLASH_CLEANUP 2
#define FLASH_READ 3
#define FLASH_WRITE 4
#define FLASH_QUERY 5
#define FLASH_SHUTDOWN 6
#define FLASH_FS_READ 7
#define FLASH_FS_WRITE 8
#define FLASH_FS_ERASE 9
#define FLASH_ERASE 10
#endif

// Used for ROM Config Area
#define ROM_CONFIG_SECTOR_SIZE		0x200

// Used for ROM Fs Area
#define ROM_FS_SECTOR_SIZE	128 /* N.B. if too small, OpenTV don't like it... */

#define SIZE_SEC_HEADER 4           /* size of sector header */

#define MANUFACTURER_ID_INTEL 0x89
#define MANUFACTURER_ID_AMD 0x01
#define INTEL_28F800B_ID 0x9D
#define AMD_29F800BB_ID 0x58
#define MAGIC_NUMBER 0xDEADBEEF

#ifndef DLOAD
//u_int32 flash_read(u_int8 source_id, void* buffer, u_int32 buffer_size, u_int32 offset);
u_int32 flash_read(u_int8 source_id, void* buffer, u_int32 buffer_size, u_int32 offset, u_int32* num_bytes_actually_read);

/* This function do the write first and then do read to verify data, and then update the directory structure */
/* If there is an write error, keep trying until finish trying the whole flash */
u_int32 flash_write(u_int8 source_id, void* source_buffer, u_int32 buffer_size, u_int32 offset, u_int32* num_bytes_actually_written);

//u_int8 * flash_alloc(u_int8 source_id, u_int32 size);
u_int32 flash_alloc(u_int8 source_id, u_int32 size);

/* This function is called while the power off button is pushed.  It erases the inactive block and */
/* and do block switch if not enough space left in the active block. */
bool flash_cleanup(void);

/* This function returns the refered information size which is stored at the begining of each sector */
u_int32 flash_query_size(u_int8 source_id, u_int32* source_data_size);

u_int32 flash_erase(void);

/* This is the flash init function which tells if KAL is running to decide to use tasks etc. or not */
bool flash_init(void);       

/* This function terminates tasks\queues if KAL is running. */
bool flash_terminate(void);

typedef struct _flash_params
{
	u_int8  action_type;
	u_int8 source_id;
	u_int8 * addr;
	u_int32 * buffer;
	u_int32 data_size;
	u_int32 offset;
	u_int32 return_value;
	u_int8 * return_address;
	task_id_t proc_id;
} flash_params;
#endif

typedef struct{
	u_int32 MagicNumber;
	u_int8 Num_Client;
	u_int8 HeaderLen;
	u_int16 Num_Sector;
	u_int8 Num_Sector_Byte; /*number of bytes needed to store sector start and end for client */
	u_int16 Size_Sector;
	u_int32 CheckSum;
} DIRHEADER;

// Debug messaging functions
#define TRACE_FLS TRACE_NV /* Alias. NV device used will be Flash. */
#define FLS_ERROR_MSG                           (TRACE_FLS|TRACE_LEVEL_ALWAYS)
#define FLS_FUNC_TRACE                          (TRACE_FLS|TRACE_LEVEL_2)
#define FLS_MSG                                 (TRACE_FLS|TRACE_LEVEL_2)
#define FLS_ISR_MSG                             (TRACE_FLS|TRACE_LEVEL_1)

// Return codes
#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

#endif
