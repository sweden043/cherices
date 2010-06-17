/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*************************************************************************
* FILE NAME                                     VERSION                 
*                                                                       
*       PC_PART.C                                 2.5              
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Analyze Partition record.
*       CNXT Disk Partition Handling.                                        
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       ext_partition_init                  Partition table interpretor.
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/



/************************************************************************
*                                                                       
*       Copyright (c) 2002 by Accelerated Technology              
*                                                                       
*  PROPRIETARY RIGHTS of Accelerated Technology are  involved in        
*  the subject matter of this material.  All manufacturing,             
*  reproduction, use, and sales rights pertaining to this subject       
*  matter are  governed by the license agreement.  The recipient of     
*     this software implicitly accepts the terms of the license.        
*                                                                       
*                                                                       
*************************************************************************/


#include "pcdisk.h"
#include "retcodes.h"
#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "ata.h"
#include "pcdisk.h"

extern _PC_BDEVSW   pc_bdevsw[];            /* Driver dispatch table.   */
extern INT          NUF_Fat_Type[];         /* FAT type list.           */

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       ext_partition_init                                              
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Partition table interpretor                                     
*       Given a physical drive number and the addresses of two tables,  
*       partition start and partition end, this routine interprets the  
*       partion tables on the physical drive and fills in the start and 
*       end blocks of each partition. Extended partitions are           
*       supported.                                                      
*       If there are more than max partitions, only max will be         
*       returned.                                                       
*                                                                       
*       Note: the physical drive must be in a raw state so no partition 
*             mapping takes place.                                      
* 
* WARNING
*       This function has two buffers totalling 768 bytes on stack.
*       Caller of this function should verify for stack overflow.                                                                 
*                                                                       
* INPUTS                                                                
*                                                                       
*       driveno                             Drive number.               
*       pstart                              Partition start sector      
*                                            string.                    
*       pend                                Partition end sector string.
*       max                                 Number of maximum partitions.
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       The number of partitions found on the drive.                    
*                                                                       
*************************************************************************/
INT16 ext_partition_init(INT16 driveno, UINT32 *pstart, UINT32 *pend, 
                         INT16 max)
{
INT16       nparts;
PTABLE      *ppart;
INT8        workbuf[256];
INT8        buf[512];
INT8        *pbuf;
UINT32      ltemp;
UINT32      ltemp2;
UINT16      stemp;
UINT16      i;
UINT32      partition_address;
UINT32      extpart_base_address;
INT16       extflg;
UINT32      rsec;
UINT32      psize;
UINT16      signature;


    /* Initialize retrun value, extpart flag and partition address. */
    nparts = extflg = 0;
    partition_address = extpart_base_address = 0L;

    while (YES)     /* Partition record loop. */
    {
        /* Check number of maximum partitions. */
        if (nparts == max)
            break;

        /* Read block zero. */
        if ( !pc_bdevsw[driveno].io_proc(driveno, partition_address, 
                                            buf, 1, YES))
            break;

        /* Copy the table to a word aligned buffer so some compilers 
           don't screw up. */
        pbuf = &buf[0];
        /* The info starts at buf[1be] */
        pbuf += 0x1be;
        copybuff(workbuf, pbuf, sizeof(PTABLE));
        /* Move to PTABLE pointer. */
        ppart = (PTABLE *) workbuf;
        /* Set signature. */
        stemp = ppart->signature;
        SWAP16(&signature, &stemp);

        /* Check signature. */
        if (signature != 0xAA55)
            break;

        /* Read through the partition table. Find the primary DOS 
           partition. */
        for (i = 0; i < 4; i++)
        {
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
            if ( (ppart->ents[i].p_typ == 0x01) ||
                 (ppart->ents[i].p_typ == 0x04) ||
                 (ppart->ents[i].p_typ == 0x06) ||
                 (ppart->ents[i].p_typ == 0x0E) ||
                 (ppart->ents[i].p_typ == 0x0B) ||
                 (ppart->ents[i].p_typ == 0x0C) )
            {
                /* Set the partition type. */
                NUF_Fat_Type[driveno + nparts + i] = ppart->ents[i].p_typ;

                /* Get the relative start. */
                SWAP32((UINT32 *)&ltemp, (UINT32 *)&ppart->ents[i].r_sec);
                ltemp += partition_address;
                /* Set the partition start sector. */
                *(pstart + nparts) = ltemp;

                /* Get the partition size. */
                SWAP32((UINT32 *)&rsec, (UINT32 *)&ppart->ents[i].r_sec);
                SWAP32((UINT32 *)&psize, (UINT32 *)&ppart->ents[i].p_size);

                /* Set the partition end sector. */
                ltemp2 = rsec + psize - 1L;
                ltemp2 += partition_address;
                *(pend + nparts) = ltemp2;

                /* Increment number of partitions. */
                nparts++;
                break;
            }
        }

        /* Now see if we have an extended partion. */
        for (i = 0; i < 4; i++)
        {
            /* ----------------- Partition Type values -----------------
               0x05     Extended MS-DOS Partition.
               0x0F     Same as PART_EXTENDED(05h),
                        but uses Logical Block Address Int 13h 
                        extensions.
               -------------------------------------------------------- */
            if ( (ppart->ents[i].p_typ == 0x05) ||
                 (ppart->ents[i].p_typ == 0x0F) )
            {
                /* Get the address of the extended partition. */
                SWAP32((UINT32 *)&ltemp, (UINT32 *)&ppart->ents[i].r_sec);
                /* Add extended partition base address. */
                ltemp += extpart_base_address;
                /* Set the next partition address. */
                partition_address = ltemp;

                /* First extended partition? */
                if (extflg == 0)
                {
                    /* Set extended partition base address. */
                    extpart_base_address = ltemp;
                    extflg = 1;
                }
                break;
            }
        }

        /* No extended partitions, bail. */
        if (i == 4)
            break;
    }

    return(nparts);
}

/* Retrieve cylinder from the packed cylinder/sector field  */
UINT16 getPackedCylinder( const UINT8* b )
{
   return b[1]+((b[0] & 0xC0) << 2);
}

/* Retrieve sector from the packed cylinder/sector field */
UINT16 getPackedSector(const UINT8* b)
{
   return (*b) & (0x3F);
}

/* Encode Cylinder/Sector in the packed field */
UINT16 CylSecEncode( UINT16 Cylinder, UINT16 Sector )
{
     return( (Cylinder << 8) | ( (Cylinder & 0x0300) >> 2 )  | (Sector & 0x3F) );
}


/* For Testing.. */
void wipe_drive_config( INT16 driveno )
{
    UINT8        *buf;
    INT8         i;

	trace_new(TRACE_ATA|TRACE_LEVEL_ALWAYS,"wipe_drive_config.\n" );
	buf = (UINT8 *)mem_nc_malloc( 512 ); /* Get a non-cached sector buffer */
	if( !buf )
    {
		trace_new(TRACE_ATA|TRACE_LEVEL_ALWAYS,"wipe_drive_config: malloc, line %d\n", __LINE__ );
		return;
    }
    /* Zero-Out allocated buffer.. */
    pc_memfill(buf, (INT)512, (UINT8)0);

    for( i=0; i<64; i++ )
	{
       /* Write zero block to Disk */
       if ( !pc_bdevsw[driveno].io_proc(driveno, i, buf, 1, NO) ) 
       {
	  	  trace_new(TRACE_ATA|TRACE_LEVEL_ALWAYS,"wipe_drive_config: write of block %d failed!\n",i );
       }
	}

    mem_nc_free( buf );
}


void create_primary_partition( INT16 driveno, UINT8 *buf )
{
    extern ata_query_t  q_buf;
    UINT32              capacity;
    UINT32              tmp;
   
	trace_new(TRACE_ATA|TRACE_LEVEL_4,"create_primary_partition: creating DOS32 LBA Primary Partition.\n" );
    
    NUF_Fat_Type[driveno] = 0xC;

    /* Zero-Out allocated buffer.. */
    pc_memfill(buf, (INT)512, (UINT8)0);

    /* Calculate Primary Partition size based on Drive Query results and selected FAT32 Cluster Size.. */
    
    /* LBA */
    capacity = (u_int32)(q_buf.param.sectors1 << 16) | (u_int32)(q_buf.param.sectors0) ;
	capacity -= q_buf.param.sectors;

	trace_new(TRACE_ATA|TRACE_LEVEL_4,"create_primary_partition: drive capacity 0x%x sectors.\n",capacity );
    
    /* The partition table starts at buf[1be] */
    
    buf[0x1C2] = (UINT8)0xC;   /* Set Partition Type to DOS32 LBA  */ 
   
    /* Set the relative start. */
    tmp = q_buf.param.sectors;
    SWAP32((UINT32 *)&buf[0x1C6], (UINT32 *)&tmp);
 
    /* Set the Partition size (sectors) */
    SWAP32((UINT32 *)&buf[0x1CA], (UINT32 *)&capacity);
    
    /* Set the MBR signature. */
    buf[0x1fe] = 0x55;
    buf[0x1ff] = 0xaa;
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         4/2/04 8:36:39 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  2    mpeg      1.1         1/7/04 4:17:53 PM      Tim Ross        CR(s) 
 *        8181 : Correct interpretation of ATA drive parameters when reading 
 *        total sector
 *        count, correct oversized read of ATA sec/track parameter, and add MBR
 *        signature to MBR when drive is partitioned.
 *  1    mpeg      1.0         8/22/03 5:41:22 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:41:22   mooreda
 * SCR(s) 7350 :
 * Modified Nucleus File code for Partition Management
 * 
 *
 ****************************************************************************/

