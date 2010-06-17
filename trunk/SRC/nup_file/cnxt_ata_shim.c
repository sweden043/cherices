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


/*  Global data definitions.  */
static bool   setf_called = FALSE, query_called = FALSE;
ata_query_t   q_buf;
static UINT32 drive_size = 0;
static UINT16 cnxt_opencount = 0;
static UINT32 cnxt_drive_handle = 0;

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
ATA_STATUS open_drive (UINT32 drive, UINT32 *phandle, ata_notify_handler_t async_handler)
{
    ATA_STATUS rc = ATA_OK;
    ata_setf_t setfparam;

    /*
     * Open the drive
     */
    rc = cnxt_ata_open( phandle, drive, async_handler );
    if(rc != ATA_OK)
    {
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "open_drive: cnxt_ata_open() failed!\n");
        return (rc);
    }

    if( !query_called )
    {
        /*
         * Query ATA subsystem for location and drive information.  Ideally for
         * product level code, there needs to be an intermediate level driver
         * or logical volume driver (or partition driver) which manages the
         * possible multiple drives and partitioning in order to have logical
         * drive information.  This allows the HD to be used for FAT filesystem
         * which could then be used for webpage caching, trace, etc...  For the
         * purposes of this demo, we simply assume we own the whole disk, there
         * is only one disk, we own it, period.
         */
        rc = cnxt_ata_query( *phandle, &q_buf );
        if( rc != ATA_OK )
        {
            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "open_drive: cnxt_ata_query() failed!\n");
            cnxt_ata_close( *phandle );
            phandle = NULL;
            return (rc);
        }

        /*
         * Set the drive size
         */
        drive_size = q_buf.size;
        query_called = TRUE;
    }

#ifdef DEBUG
    /*
     * Display drive information
     */
    
    printf("    ============= IDE DRIVE CAPS =============\n");
    printf("     drive = %lu       size  = %llu\n", q_buf.drive, q_buf.size);
    printf("     sectors1 = %d   sectors0 = %d\n",
        q_buf.param.sectors1, q_buf.param.sectors0);
    printf("     caps = 0x%04x    caps2 = 0x%04x\n",
        q_buf.param.capabilities, q_buf.param.capabilities2);
    printf("     pioMode = 0x%04x dmaMode = 0x%04x valid = 0x%04x\n",
        q_buf.param.pioMode, q_buf.param.dmaMode, q_buf.param.valid);
    printf("     mwdma = 0x%04x aPio = 0x%04x\n",
        q_buf.param.multiDma, q_buf.param.advancedPio);
    printf("     q_depth = %d  maj_ver = 0x%04x min_ver = 0x%04x\n",
        q_buf.param.queueDepth, q_buf.param.majorVers, q_buf.param.minorVers);
    printf("     cmdSet = %04x cmdSet2 = 0x%04x cndSetExt = 0x%04x\n\n     ",
        q_buf.param.cmdSet, q_buf.param.cmdSet2, q_buf.param.cmdSetExt);
    if(q_buf.param.valid & 4)
    {
        if(q_buf.param.ultraDmaMode & 16384)
            printf("ULTRA DMA Mode 6 is selected (UDMA-133)\n");
        else if(q_buf.param.ultraDmaMode & 8192)
            printf("ULTRA DMA Mode 5 is selected (UDMA-100)\n");
        else if(q_buf.param.ultraDmaMode & 4096)
            printf("ULTRA DMA Mode 4 is selected (UDMA-66)\n");
        else if(q_buf.param.ultraDmaMode & 2048)
            printf("ULTRA DMA Mode 3 is selected (UDMA-33)\n");
        else if(q_buf.param.ultraDmaMode & 1024)
            printf("ULTRA DMA Mode 2 is selected\n");
        else if(q_buf.param.ultraDmaMode & 512)
            printf("ULTRA DMA Mode 1 is selected\n");
        else if(q_buf.param.ultraDmaMode & 256)
            printf("ULTRA DMA Mode 0 is selected\n");
    }
    else if(q_buf.param.multiDma & 1024)
        printf("MWord DMA Mode 2 is selected\n");
    else if(q_buf.param.multiDma & 512)
        printf("MWord DMA Mode 1 is selected\n");
    else if(q_buf.param.multiDma & 256)
        printf("MWord DMA Mode 0 is selected\n");

    if(q_buf.param.cmdSet2 & 512)
        printf("     Recommended Acoustic Management Value: %d\n", (q_buf.param.acousticMgmtMode&0xFF00)>>8 );
    printf("         Current Acoustic Management Value: ");
    if((q_buf.param.acousticMgmtMode&0xFF) == 0xFE)
        printf("Maximize Performance\n");
    else
        printf("%d\n", q_buf.param.acousticMgmtMode&0xFF );
    printf("    ============= IDE DRIVE CAPS =============\n");
#endif

    if( !setf_called )
    {
        /*
         * Enable read look-ahead feature
         */
        memset(&setfparam, 0, sizeof(setfparam));
        setfparam.features = ATA_SETF_ENABLE_READ_LOOK_AHEAD;
        rc = cnxt_ata_setf( *phandle, &setfparam );
        if( rc != ATA_OK )
        {
            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "open_drive: cnxt_ata_setf() failed line %d !\n",__LINE__);
            cnxt_ata_close( *phandle );
            phandle = NULL;
            return (rc);
        }

        /*
         * Enable write cache feature
         */
        memset(&setfparam, 0, sizeof(setfparam));
        setfparam.features = ATA_SETF_ENABLE_WRITE_CACHE;
        rc = cnxt_ata_setf( *phandle, &setfparam );
        if( rc != ATA_OK )
        {
            trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "open_drive: cnxt_ata_setf() failed line %d !\n",__LINE__);
            cnxt_ata_close( *phandle );
            phandle = NULL;
            return( rc );
        }
        setf_called = TRUE;
    }
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
INT cnxt_ide_ioctl( UINT16 driveno, UINT16 command, VOID *buffer )
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
INT cnxt_ide_raw_open( UINT16 driveno )
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
INT cnxt_ide_open( UINT16 driveno )
{

    if( !cnxt_opencount )
	{
      if( ATA_OK != open_drive( 0, &cnxt_drive_handle, 0 ) )
	  {
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_open: open_drive failed line %d.\n",__LINE__);
	    return( NO );
      }
    }

    cnxt_opencount++;
    
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
INT cnxt_ide_close( UINT16 driveno ) 
{

    if( cnxt_opencount )
        --cnxt_opencount;

    if( !cnxt_opencount )
	{
      if( ATA_OK != cnxt_ata_close( cnxt_drive_handle ) )
	  {
        trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_close: cnxt_ata_close failed line %d.\n",__LINE__);
	    return( NO );
	  }
    }
    return( YES );
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
INT cnxt_ide_io( UINT16 driveno, UINT32 block, void *buffer, UINT16 count, INT reading )  
{

    UINT32  size  = (UINT32)count;
    u_int64 lba64 = (u_int64)block;


#ifdef DEBUG
    if ( (unsigned int)buffer & 3 )
	{
         trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_io: alignment 0x%x line %d.\n",(UINT32)buffer,__LINE__);
	}
#endif

    if( reading )
	{
//trace_new( TRACE_ATA | TRACE_LEVEL_4,"READ: block %d\n",block);
      if( ATA_OK != cnxt_ata_read ( cnxt_drive_handle, ATA_FLAGS_SYNC, 0, 
        lba64, (unsigned long *)buffer, &size, 0, NULL, NULL ) )
      {
          trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS, "cnxt_ide_io: cnxt_ata_read failed lba %d line %d.\n",block,__LINE__);
		  return( NO );
	  }
	}
	else
	{
//trace_new( TRACE_ATA | TRACE_LEVEL_ALWAYS,"WRITE: block %d\n",block);

      if( ATA_OK != cnxt_ata_write( cnxt_drive_handle, ATA_FLAGS_SYNC, 0, 
        lba64, (unsigned long *)buffer, &size, 0, NULL, NULL ) ) 
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

