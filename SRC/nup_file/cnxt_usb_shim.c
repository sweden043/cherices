/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                       Conexant Systems Inc. (c) 2003                     */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       cnxt_ata_shim.c
 *
 *
 * Description:    Maps Nucleus file driver API onto Conexant ATA driver API
 *                 See devtable.c for insertion of these functions into Nucleus
 *                 device switch table.
 *
 * Author:         Dave Moore
 *
 ****************************************************************************/
/* $Header: cnxt_ata_shim.c, 6, 6/9/04 6:40:13 PM, Adrian Kwong$
 ****************************************************************************/

#include "nucleus.h"
#include "pcdisk.h"
#include <stdio.h>
#include <string.h>
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "ata.h"
#include "..\..\sl811\TPBULK.H"


/*  Global data definitions.  */
//static bool   setf_called = FALSE, query_called = FALSE;
ata_query_t   q_buf;
//static UINT32 drive_size = 0;
//static UINT16 cnxt_opencount = 0;
static UINT32 cnxt_usb_drive_handle = 0;

/* Disk Partitioning */
/********************************************************************/
/*  open_drive                                                      */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      drive         - Drive to open                               */
/*      async_handler - Async callback handler                      */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Utility function to open a drive and get it ready for use.  */
/*                                                                  */
/*  RETURNS:                                                        */
/*      phandle       - Handle                                      */
/********************************************************************/
ATA_STATUS open_usb_drive (UINT32 drive, UINT32 *phandle, ata_notify_handler_t async_handler)
{
    ATA_STATUS rc = ATA_OK;
    if(EnumMassDev())
    ;

    return( rc );
}

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       cnxt_ide_ioctl                                                  
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       IOCTL entry point. Currently no IOCTLs supported.               
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
INT cnxt_usb_ioctl( UINT16 driveno, UINT16 command, VOID *buffer )
{
    driveno = driveno;
    command = command;
    buffer = buffer;
    return( NO );
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       cnxt_ide_raw_open                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Not Required (stub).                                            
*                                                                       
* INPUTS                                                                
*                                                                       
*       None.                                                           
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       None.                                                           
*                                                                       
*************************************************************************/
INT cnxt_usb_raw_open( UINT16 driveno )
{

    driveno = driveno;
    return( NO );
}



/************************************************************************
* FUNCTION                                                              
*                                                                       
*       cnxt_ide_open                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*      Utility function to open a drive and get it ready for use.       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             The number assigned to the  
*                                           Disk (not used, only 1 drive supported)
*
* OUTPUTS                                                               
*                                                                       
*       YES                                 Successful Completion.      
*       NO                                  Failed.
*                                                                       
*************************************************************************/
INT cnxt_usb_open( UINT16 driveno )
{
    if( ATA_OK != open_usb_drive( 0, &cnxt_usb_drive_handle, 0 ) )
	  {
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_open: open_drive failed line %d.\n",__LINE__);
	    return( NO );
    }
   
    return( YES );
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       cnxt_ide_close                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Close drive. If last close then free resources.
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             The number assigned to the  
*                                           Disk (not used, only 1 drive supported)        
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 Successful Completion.      
*       NO                                  Failed.
*                                                                       
*************************************************************************/
INT cnxt_usb_close( UINT16 driveno ) 
{
	   return( NO );
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       cnxt_ide_io                                                     
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       This function reads or writes data from and to the Disk         
*       based on the 'reading' parameter.                               
*                                                                       
* AUTHOR                                                                
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             The number assigned to the  
*                                            Disk (not used)            
*       block                               The lba number to read or   
*                                            write                      
*       buffer                              Pointer to the data to be   
*                                            placed from a read or      
*                                            stored on a write          
*       count                               Number of sectors to be read
*                                            or written                 
*       reading                             Indicates whether or not we 
*                                            are reading or writing     
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       YES                                 Successful Completion.      
*       NO                                  Block number is out of range.
*                                                                       
*************************************************************************/
INT cnxt_usb_io( UINT16 driveno, UINT32 block, void *buffer, UINT16 count, INT reading )  
{
  if( reading )
	{
//trace_new( TRACE_ATA | TRACE_LEVEL_4,"READ: block %d\n",block);
      if( ATA_OK != RBC_Read(block,count, (unsigned long *)buffer))
      {
          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_io: cnxt_ata_read failed lba %d line %d.\n",block,__LINE__);
		  return( NO );
	  }
	}
	else
	{
//trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"WRITE: block %d\n",block);

      if( ATA_OK != RBC_Write(block,count, (unsigned long *)buffer))
	  {
          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_io: cnxt_ata_write failed lba %d line %d.\n",block,__LINE__);
		  return( NO );
	  }
	}

    return( YES );
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  6    mpeg      1.5         6/9/04 6:40:13 PM      Adrian Kwong    CR(s) 
 *        9415 9416: A new parameter was added to the cnxt_ata_read, 
 *        cnxt_ata_write, and the asynchronous callback function.  The
 *        
 *        new parameter is a pointer to a structure to receive extended ATA 
 *        results as a result of an IO operation.
 *        
 *        This structure contains the standard ATA_STATUS, sectors successfully
 *         written, LBA Offset at which the error
 *        
 *        ocurred, the ATA Status register, and the ATA Error register.  If the
 *         parameter is NULL, extended ATA information
 *        
 *        is not provided.
 *        
 *        
 *  5    mpeg      1.4         4/3/04 1:53:50 AM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  4    mpeg      1.3         3/19/04 9:36:15 PM     Adrian Kwong    CR(s) 
 *        8601 : Extend cnxt_ata_read, cnxt_ata_write for future read/write 
 *        enhancements (LBA48/stream/etc).  cnxt_ata_setf modified to use 
 *        parameters in a structure.
 *        
 *        
 *  3    mpeg      1.2         10/15/03 4:56:25 PM    Tim White       CR(s): 
 *        7660 Remove ATA header files from NUP_FILE code.
 *        
 *  2    mpeg      1.1         8/22/03 6:47:32 PM     Dave Moore      SCR(s) 
 *        7350 :
 *        Fixed compiler warnings
 *        
 *        
 *  1    mpeg      1.0         8/22/03 5:23:22 PM     Dave Moore      
 * $
 ****************************************************************************/

