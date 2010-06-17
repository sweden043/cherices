/*****************************************************************************/
/* File: osdttx.h                                                            */
/*                                                                           */
/* Module: Conexant RC86000 OSD Teletext Library.                            */
/*                                                                           */
/* Description:  Teletext header file.                                       */
/*****************************************************************************/
/*****************************************************************************
$Header: osdttx.h, 8, 8/16/00 5:11:58 PM, Miles Bintz$
$Log: 
 8    mpeg      1.7         8/16/00 5:11:58 PM     Miles Bintz     changed 
       BlahEx functions to Blah.  Fixed funtion prototypes.
       Only Open has OpenEx2
       
 7    mpeg      1.6         8/14/00 6:02:44 PM     Miles Bintz     added 
       HWInit, SWInit, and Shutdown...
       
 6    mpeg      1.5         8/10/00 8:03:12 PM     Tim Ross        Rolled in 
       VENDOR_B changes.
       
 5    mpeg      1.4         8/9/00 5:01:38 PM      Miles Bintz     fixed error 
       in macro define for osdTTXOpen
       
 4    mpeg      1.3         8/9/00 12:56:12 PM     Miles Bintz     added 
       defines for return codes. See osdttxc.c rev 1.7-1.8
       
 3    mpeg      1.2         4/3/00 5:33:56 PM      Rob Tilton      Added a flag
        to open to specify scrambling.
       
 2    mpeg      1.1         2/11/00 2:47:42 PM     Rob Tilton      Added the 
       data unit ID in osdTTXOpen().
       
 1    mpeg      1.0         9/30/99 2:40:44 PM     Rob Tilton      
$
 * 
 *    Rev 1.7   16 Aug 2000 16:11:58   bintzmf
 * changed BlahEx functions to Blah.  Fixed funtion prototypes.
 * Only Open has OpenEx2
 * 
 *    Rev 1.6   14 Aug 2000 17:02:44   bintzmf
 * added HWInit, SWInit, and Shutdown...
 * 
 *    Rev 1.5   10 Aug 2000 19:03:12   rossst
 * Rolled in VENDOR_B changes.
 * 
 *    Rev 1.4   09 Aug 2000 16:01:38   bintzmf
 * fixed error in macro define for osdTTXOpen
 * 
 *    Rev 1.3   09 Aug 2000 11:56:12   bintzmf
 * added defines for return codes. See osdttxc.c rev 1.7-1.8
 * 
 *    Rev 1.2   03 Apr 2000 16:33:56   rtilton
 * Added a flag to open to specify scrambling.
 * 
 *    Rev 1.1   11 Feb 2000 14:47:42   rtilton
 * Added the data unit ID in osdTTXOpen().
 * 
 *    Rev 1.0   30 Sep 1999 13:40:44   rtilton
 * Initial revision.
 * 
 *    Rev 1.0   19 May 1999 12:55:26   rtilton
 * Initial revision.
 * 
*****************************************************************************/
#ifndef _OSDTTX_H_
#define _OSDTTX_H_

#define OSD_TELETEXT_ID          0x02
#define OSD_SUBTITLE_ID          0x03

// Exported function declarations.
void osdTTXInit(void);
void osdTTXHWInit( void );
void osdTTXSWInit( void );

u_int32 osdTTXShutdown(void);

u_int32 osdTTXOpenEx2(u_int32 dwPid, u_int8 byTTXDataUnitID, bool bScramble, u_int32 ui32BuffSize);
u_int32 osdTTXClose(void);
u_int32 osdTTXPlay(void);
u_int32 osdTTXStop(void);

#define osdTTXOpen(Pid, DataUnitID)          osdTTXOpenEx2(Pid, DataUnitID, TRUE,  4096)
#define osdTTXOpenEx(Pid, DataUnitID, Scram) osdTTXOpenEx2(Pid, DataUnitID, Scram, 4096)


#define OSD_TTX_SUCCESS          0x00

#define OSD_TTX_ALREADY_OPEN     0x01
#define OSD_TTX_NOT_OPEN         0x02

#define OSD_TTX_MALLOC_ERROR     0x03
#define OSD_TTX_DMX_ERROR        0x04

#define OSD_TTX_ALREADY_PLAYING  0x05
#define OSD_TTX_NOT_PLAYING      0x06

#define OSD_TTX_SHUTDOWN_FAIL    0x07

#define OSD_TTX_UNKNOWN_ERROR    0xFF



#endif /* _OSDTTX_H_ */

