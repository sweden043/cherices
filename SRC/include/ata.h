/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        ata.h
 *
 *
 * Description:     ATA Subsystem Interface Header File
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: ata.h, 10, 6/10/04 3:05:51 PM, Adrian Kwong$
 ****************************************************************************/

#ifndef _ATA_H
#define _ATA_H

/**
 ** rc definitions
 **/

typedef u_int32                 ATA_STATUS;

#define ATA_OK                  0x00000000
#define ATA_INVALID_HANDLE      0x00010000
#define ATA_UNAVAILABLE         0x00020000
#define ATA_NOT_RESPONDING      0x00030000
#define ATA_BUSY                0x00040000
#define ATA_IO_ERROR            0x00050000
#define ATA_INVALID_PARAMETER   0x00060000

/**
 ** read/write flag definitions
 **/

#define ATA_FLAGS_SYNC           0x00000000
#define ATA_FLAGS_ASYNC          0x00000001
#define ATA_FLAGS_FUA            0x00000002
#define ATA_FLAGS_STREAM         0x00000004
#define ATA_FLAGS_STREAM_URGENT  0x00000008
#define ATA_FLAGS_STREAMID_MASK  0xF0000000
#define ATA_FLAGS_STREAMID_PRES  0x80000000
#define ATA_FLAGS_STREAM_ID(id)  (((id&7)|8)<<28)


/**
 ** error code definitions
 **/

#define ATA_ERR_ICRC		0x80
#define ATA_ERR_UNC		0x40
#define ATA_ERR_MC		0x20
#define ATA_ERR_IDNF		0x10
#define ATA_ERR_MCR		0x08
#define ATA_ERR_ABRT		0x04
#define ATA_ERR_NM		0x02

/**
 ** Physical drive masks
 **/

#define GEN_DRIVE_TYPE_MASK           ((u_int32)0xF<<28)
#define GEN_DRIVE_TYPE_ATA            ((u_int32)0x0<<28)

#define ATA_DRIVE_CONTROLLER_MASK     ((u_int32)0xFF<<8)
#define ATA_DRIVE_CONTROLLER_SHIFT        8
#define ATA_DRIVE_DRIVE_MASK          ((u_int32)0xFF<<0)
#define ATA_DRIVE_DRIVE_SHIFT             0

/**
 ** Asynchronous events
 **/

typedef enum
{
    ATA_EVT_READ_COMPLETE = 1,
    ATA_EVT_WRITE_COMPLETE,
    ATA_EVT_READ_ERROR,
    ATA_EVT_WRITE_ERROR
} ata_event_t;

typedef struct _ata_extended_result_t
{
   ATA_STATUS status;     /* cnxt_ata status code                             */
   u_int32    size;       /* number of sectors written                        */
   u_int64    lba_error;  /* lba offset at which the drive reported an error  */
   u_int8     ata_error;  /* copy of the ATA ERROR register on cmd completion */
   u_int8     ata_status; /* copy of the ATA_STATUS register on cmd completion*/
} ata_extended_result_t, *pata_extended_result_t;

/**
 ** Notify handler
 **/

typedef void (*ata_notify_handler_t)(
   u_int32 ata_handle, 
   u_int32 tag, 
   ata_event_t event, 
   ata_extended_result_t *pdata);

/**
 ** Configuration, Initialization, Control
 **/

ATA_STATUS cnxt_ata_config (void);
ATA_STATUS cnxt_ata_init (void);
ATA_STATUS cnxt_ata_term (void);
ATA_STATUS cnxt_ata_open (u_int32 *ata_handle, u_int32 physical_drive,
                          ata_notify_handler_t ata_notify_handler);
ATA_STATUS cnxt_ata_close (u_int32 ata_handle);
ATA_STATUS cnxt_ata_flush (u_int32 ata_handle, u_int64 *lba_error64);


/**
 ** Read & Write
 **/

ATA_STATUS cnxt_ata_read (u_int32 ata_handle, u_int32 flags, u_int32 priority,
                          u_int64 lba, u_int32 *buffer, u_int32 *size, 
                          u_int32 tag, ata_extended_result_t *extended_result,
                          void *encr_ctrl);
ATA_STATUS cnxt_ata_write (u_int32 ata_handle, u_int32 flags, u_int32 priority,
                           u_int64 lba, u_int32 *buffer, u_int32 *size, 
                           u_int32 tag, ata_extended_result_t *extended_result,
                           void *encr_ctrl);

/**
 ** Power Management
 **/

/*
 * cmd definitions
 */
#define ATA_GET                        1
#define ATA_SET                        2
#define ATA_TIMER                      3

/*
 * arg definitions when cmd == ATA_GET or ATA_SET
 */
#define ATA_ACTIVE                     0xFF
#define ATA_IDLE                       0x80
#define ATA_STANDBY                    0x00
#define ATA_SLEEP                      0x01

/*
 * Values to use when cmd == ATA_TIMER for the arg field.
 *
 *     0x00     : No timeout
 *     0x01-0xF0: (Value * 5) sec
 *     0xF1-0xFB: ((Value - 0xF0) * 30) min
 *     0xFC     : 21 min
 *     0xFD     : 8-12 hours
 *     0xFE     : Reserved
 *     0xFF     : 21 min 15 sec
 */

ATA_STATUS cnxt_ata_pm (u_int32 ata_handle, u_int32 cmd, u_int32 *arg);

/**
 ** Query Interface
 **/

#define ATA_MAX_DRIVES                 2

typedef struct _ata_param_t
{
                                 /* Description                                          Word      ATA-5       ATA-7    */
                                 /*                                                      Number    Status      Status   */
    u_int16 config;              /* general configuration                                0         Current     Current  */
    u_int16 cylinders;           /* number of cylinders                                  1         Current     Obsolete */
    u_int16 specificConfig;      /* specific configuration                               2         Reserved    Current  */
    u_int16 heads;               /* number of heads                                      3         Current     Obsolete */
    u_int16 reserved4[2];        /* reserved / retired                                   4,5       Reserved    Retired  */
    u_int16 sectors;             /* number of sectors/track                              6         Current     Obsolete */
    u_int16 reserved7[2];        /* Reserved for assignment by CFA                       7,8       Reserved    Reserved */
    u_int16 retired9;            /* Retired                                              9         Reserved    Retired  */
    u_int8 serial[20];           /* controller serial number                             10-19     Current     Current  */
    u_int16 reserved20[2];       /* reserved / retired                                   20,21     Reserved    Retired  */
    u_int16 bytesEcc;            /* ecc bytes appended                                   22        Current     Obsolete */
    u_int8 rev[8];               /* firmware revision                                    23-26     Current     Current  */
    u_int8 model[40];            /* model name                                           27-46     Current     Current  */
    u_int16 multiSecs;           /* RW multiple support. bits 7-0 ia max secs            47        Current     Current  */
    u_int16 reserved48;          /* reserved                                             48        Reserved    Reserved */
    u_int16 capabilities;        /* capabilities                                         49        Current     Current  */
    u_int16 capabilities2;       /* capabilities 2                                       50        Current     Current  */
    u_int16 pioMode;             /* PIO data transfer cycle timing mode                  51        Current     Obsolete */
    u_int16 dmaMode;             /* single word DMA data transfer cycle timing           52        Current     Obsolete */
    u_int16 valid;               /* field validity                                       53        Current     Current  */
    u_int16 currentCylinders;    /* number of current logical cylinders                  54        Current     Obsolete */
    u_int16 currentHeads;        /* number of current logical heads                      55        Current     Obsolete */
    u_int16 currentSectors;      /* number of current logical sectors / track            56        Current     Obsolete */
    u_int16 capacity0;           /* current capacity in sectors                          57        Current     Obsolete */
    u_int16 capacity1;           /* current capacity in sectors                          58        Current     Obsolete */
    u_int16 multiSet;            /* multiple sector setting                              59        Current     Current  */
    u_int16 sectors0;            /* total number of user addressable sectors (lo word)   60        Current     Current  */
    u_int16 sectors1;            /* total number of user addressable sectors (hi word)   61        Current     Current  */
    u_int16 reserved62;          /* reserved / obsolete                                  62        Reserved    Obsolete */
    u_int16 multiDma;            /* multi word DMA transfer                              63        Current     Obsolete */
    u_int16 advancedPio;         /* flow control PIO transfer modes supported            64        Current     Obsolete */
    u_int16 cycletimeDma;        /* minimum multiword DMA transfer cycle time            65        Current     Current  */
    u_int16 cycletimeMulti;      /* recommended multiword DMA cycle time                 66        Current     Current  */
    u_int16 cycletimePioNoIordy; /* min PIO transfer cycle time wo flow ctl              67        Current     Current  */
    u_int16 cycletimePioIordy;   /* min PIO transfer cycle time w IORDY                  68        Current     Current  */
    u_int16 reserved69[2];       /* Reserved for Overlap and Queuing                     69,70     Reserved    Reserved */
    u_int16 timePacketCmd;       /* typ time (ns) from rec. of packet cmd to bus rel     71        Current     Current  */
    u_int16 timeServiceCmd;      /* typ time (ns) from rec. of service cmd to BSY=0      72        Current     Current  */
    u_int16 reserved73[2];       /* Reserved                                             73,74     Reserved    Reserved */
    u_int16 queueDepth;          /* maximum queue depth                                  75        Current     Current  */
    u_int16 reserved76[4];       /* Reserved for SerialATA                               76-79     Reserved    Reserved */
    u_int16 majorVers;           /* major version                                        80        Current     Current  */
    u_int16 minorVers;           /* minor version                                        81        Current     Current  */
    u_int16 cmdSet;              /* command set supported                                82        Current     Current  */
    u_int16 cmdSet2;             /* command set supported                                83        Current     Current  */
    u_int16 cmdSetExt;           /* command set supported extension                      84        Current     Current  */
    u_int16 cmdSetEnable;        /* command set enable (Maps to 82)                      85        Current     Current  */
    u_int16 cmdSetEnable2;       /* command set enable                                   86        Current     Current  */
    u_int16 cmdSetDefault;       /* command set enable extension                         87        Current     Current  */
    u_int16 ultraDmaMode;        /* ultra DMA mode                                       88        Current     Current  */
    u_int16 securityEraseTime;   /* security erase time req'd                            89        Current     Current  */
    u_int16 enhancedSecErase;    /* enhanced security erase time req'd                   90        Current     Current  */
    u_int16 advPwrMgt;           /* advanced power management value                      91        Current     Current  */
    u_int16 masterPWRevCode;     /* Master Password Revision Code                        92        Reserved    Current  */
    u_int16 hwResetResult;       /* Hardware Reset Result Codes Master/Slave Detect      93        Reserved    Current  */
    u_int16 acousticMgmtMode;    /* Acousting Management Mode / Value                    94        Reserved    Current  */
    u_int16 streamMinReqSize;    /* ATA Streaming Minimum Request Size                   95        Reserved    Current  */
    u_int16 streamDmaXferTime;   /* ATA Streaming DMA Transfer Time                      96        Reserved    Current  */
    u_int16 streamLatency;       /* ATA Streaming DMA/PIO Access Latency                 97        Reserved    Current  */
    u_int16 streamGranularityLo; /* ATA Streaming Granularity (lo word)                  98        Reserved    Current  */
    u_int16 streamGranularityHi; /* ATA Streaming Granularity (hi word)                  99        Reserved    Current  */
    u_int16 lba48sectors0;       /* total number of user addressable sectors (lo lo)     100       Reserved    Current  */
    u_int16 lba48sectors1;       /* total number of user addressable sectors (lo hi)     101       Reserved    Current  */
    u_int16 lba48sectors2;       /* total number of user addressable sectors (hi lo)     102       Reserved    Current  */
    u_int16 lba48sectors3;       /* total number of user addressable sectors (hi hi)     103       Reserved    Current  */
    u_int16 streamPioXferTime;   /* ATA Streaming PIO Transfer Time                      104       Reserved    Current  */
    u_int16 reserved105;         /* Reserved                                             105       Reserved    Current  */
    u_int16 logicalSectorSize;   /* Physical to Logical Sector Size Mapping              106       Reserved    Current  */
    u_int16 seekDelayISO7779;    /* interseek delay for ISO7779 acoustic testing (us)    107       Reserved    Current  */
    u_int16 worldWideName[4];    /* 64-bit World Wide Name (NAA+OUID+UniqueID)           108-111   Reserved    Current  */
    u_int16 worldWideNameExt[4]; /* 128-bit World Wide Name Extension                    112-115   Reserved    Current  */
    u_int16 techSupportCode;     /* Reserved for Technical Support                       116       Reserved    Current  */
    u_int16 logicalSectorSize0;  /* Words per Logical Sector (lo)                        117       Reserved    Current  */
    u_int16 logicalSectorSize1;  /* Words per Logical Sector (hi)                        118       Reserved    Current  */
    u_int16 reserved119[8];      /* Reserved                                             119-126   Reserved    Reserved */
    u_int16 removeMediaNotify;   /* removable media status notification                  127       Current     Current  */
    u_int16 securytStatus;       /* security status                                      128       Current     Current  */
    u_int16 vendor[31];          /* vendor specific                                      129-159   Vendor      Vendor   */
    u_int16 reserved160;         /* CFA Power Mode 1 Supported                           160       Reserved    Current  */
    u_int16 reserved161[15];     /* Reserved for Assignment by CFA                       161-175   Reserved    Reserved */
    u_int16 mediaSerialNum[30];  /* Current Media Serial Number                          176-205   Reserved    Reserved */
    u_int16 reserved206[49];     /* Reserved                                             206-254   Reserved    Reserved */
    u_int16 integrity;           /* Integrity Word and Checksum                          255       Reserved    Current  */
} ata_param_t, *pata_param_t;

typedef struct _ata_query_t
{
    u_int32     drive;
    u_int64     size;
    ata_param_t param;
} ata_query_t, *pata_query_t;

ATA_STATUS cnxt_ata_query (u_int32 ata_handle, ata_query_t *query);

/**
 ** Set Features
 **/

/*
 * Feature list (set in 'feature' parameter below)
 */

#define ATA_SETF_ENABLE_WRITE_CACHE                    0x02
#define ATA_SETF_SET_TRANSFER_MODE                     0x03
/*      obsolete                                       0x04 */
#define ATA_SETF_ENABLE_ADVANCED_POWER_MANAGEMENT      0x05
#define ATA_SETF_ENABLE_POWERUP_IN_STANDBY             0x06
#define ATA_SETF_POWERUP_IN_STANDBY_SPIN_UP_NOW        0x07
/*      Reserved for Address Offset Tech Report        0x09 */
/*      Enable CFA Power Mode 1                        0x0A */
/*      Reserved For Serial ATA Command                0x10 */
/*      Reserved for Technical Report                  0x20 */
/*      Reserved for Technical Report                  0x21 */
#define ATA_SETF_DISABLE_MEDIA_STATUS_NOTIFICATION     0x31
/*      obsolete                                       0x33 */
#define ATA_SETF_ENABLE_AUTOMATIC_ACOUSTIC_MANAGEMENT  0x42
#define ATA_SETF_SET_MAX_HOST_INTERFACE_SECTOR_TIMES   0x43
/*      obsolete                                       0x44 */
/*      obsolete                                       0x54 */
#define ATA_SETF_DISABLE_READ_LOOK_AHEAD               0x55
#define ATA_SETF_ENABLE_RELEASE_INTERRUPT              0x5D
#define ATA_SETF_ENABLE_SERVICE_INTERRUPT              0x5E
#define ATA_SETF_DISABLE_REVERT_TO_POWER_ON_DEFAULTS   0x66
/*      obsolete                                       0x77 */
/*      Disable 8-bit PIO transfer mode (CFA)          0x81 */
#define ATA_SETF_DISABLE_WRITE_CACHE                   0x82
/*      obsolete                                       0x84 */
#define ATA_SETF_DISABLE_ADVANCED_POWER_MANAGEMENT     0x85
#define ATA_SETF_DISABLE_POWERUP_IN_STANDBY            0x86
/*      obsolete                                       0x88 */
/*      Reserved for Address Offset Tech Report        0x89 */
/*      Disable CFA Power Mode 1                       0x8A */
/*      Reserved for SerialATA                         0x90 */
#define ATA_SETF_ENABLE_MEDIA_STATUS_NOTIFICATION      0x95
/*      obsolete                                       0x99 */
/*      obsolete                                       0x9A */
#define ATA_SETF_ENABLE_READ_LOOK_AHEAD                0xAA
/*      obsolete                                       0xAB */
/*      obsolete                                       0xBB */
#define ATA_SETF_DISABLE_AUTOMATIC_ACOUSTIC_MANAGEMENT 0xC2
#define ATA_SETF_ENABLE_REVERT_TO_POWER_ON_DEFAULTS    0xCC
#define ATA_SETF_DISABLE_RELEASE_INTERRUPT             0xDD
#define ATA_SETF_DISABLE_SERVICE_INTERRUPT             0xDE
/*      obsolete                                       0xE0 */

/* 
How to translate ATA-4 CHS Register Nomenclature to ATA-7 Defined LBA Register Nomenclature:

   +---------------------------+----------------------------+
   | ATA-4 CHS Register Layout | ATA-7 LBA Register Layout  |
   +---------------------------+----------------------------+
   | 0 Features                | 0 Features                 |
   | 1 Sector Count            | 1 Sector Count             |
   | 2 Sector Number           | 2 LBA Low                  |
   | 3 Cylinder Low            | 3 LBA Mid                  |
   | 4 Cylinder High           | 4 LBA High                 |
   | 5 Device | Head           | 5 Device  | LBA MSNybble   |
   | 6 Command / Status        | 6 Command / Status         |
   +---------------------------+----------------------------+
*/

typedef struct _ata_setf_t
{
   u_int8   features;
   u_int8   seccount;
   u_int8   lba_low;
   u_int8   lba_mid;
   u_int8   lba_high;
   u_int8   device;
   u_int8   status;
} ata_setf_t, *pata_setf_t;

ATA_STATUS cnxt_ata_setf (u_int32 ata_handle, ata_setf_t *setf);

#endif /* _ATA_H */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   STB1.8.0  1.8.1.0     6/10/04 3:05:51 PM     Adrian Kwong    CR(s) 
 *        9416 : A new parameter was added to the cnxt_ata_read, 
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
 *  9    mpeg      1.8         3/19/04 7:58:56 PM     Adrian Kwong    CR(s) 
 *        8601 : Extend cnxt_ata_read, cnxt_ata_write for future read/write 
 *        enhancements (LBA48/stream/etc).  cnxt_ata_setf modified to use 
 *        parameters in a structure.
 *  8    mpeg      1.7         8/15/02 3:54:44 PM     Tim White       SCR(s) 
 *        4398 :
 *        Modified the cnxt_ata_setf() API.  This does not impact any customers
 *         nor the Vendor_C ATA
 *        driver interface since this function is not called from it.
 *        
 *        
 *  7    mpeg      1.6         6/26/02 1:53:48 PM     Tim White       SCR(s) 
 *        4095 :
 *        New ATA driver API for new driver.
 *        
 *        
 *  6    mpeg      1.5         1/4/01 4:00:06 PM      Tim White       Added 
 *        ata_sef() function, modified ata_query() function.
 *        
 *  5    mpeg      1.4         1/3/01 3:24:54 PM      Tim White       Revamped 
 *        ATA subsystem for XTV use.
 *        
 *  4    mpeg      1.3         11/20/00 5:08:04 PM    Tim White       Added 
 *        comments regarding standby timer values.
 *        
 *  3    mpeg      1.2         11/20/00 9:15:12 AM    Tim White       Added 
 *        power management interface.
 *        
 *  2    mpeg      1.1         10/20/00 10:40:50 AM   Tim White       Added 
 *        multiple drive capability.
 *        
 *  1    mpeg      1.0         6/22/00 5:33:32 PM     Tim White       
 * $
 * 
 *    Rev 1.7   15 Aug 2002 14:54:44   whiteth
 * SCR(s) 4398 :
 * Modified the cnxt_ata_setf() API.  This does not impact any customers nor the Vendor_C ATA
 * driver interface since this function is not called from it.
 * 
 * 
 *    Rev 1.6   26 Jun 2002 12:53:48   whiteth
 * SCR(s) 4095 :
 * New ATA driver API for new driver.
 * 
 * 
 *    Rev 1.5   04 Jan 2001 16:00:06   whiteth
 * Added ata_sef() function, modified ata_query() function.
 * 
 *    Rev 1.4   03 Jan 2001 15:24:54   whiteth
 * Revamped ATA subsystem for XTV use.
 * 
 *    Rev 1.3   20 Nov 2000 17:08:04   whiteth
 * Added comments regarding standby timer values.
 * 
 *    Rev 1.2   20 Nov 2000 09:15:12   whiteth
 * Added power management interface.
 * 
 *    Rev 1.1   20 Oct 2000 09:40:50   whiteth
 * Added multiple drive capability.
 * 
 *    Rev 1.0   22 Jun 2000 16:33:32   whiteth
 * Initial revision.
 * 
 *    Rev 1.3   13 Apr 2000 17:59:24   whiteth
 * Added ATA error handling for bad blocks.
 * 
 *    Rev 1.2   22 Mar 2000 16:42:08   whiteth
 * Added the Colorado Internal ATA Subsystem support and turned development
 * driver into production driver.
 *
 ****************************************************************************/

