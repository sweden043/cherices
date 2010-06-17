/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           OSDDEFS.H                                            */
/*                                                                          */
/* Description:        Mixed mode (o-code and native) header file including */
/*                     definitions of flags used in o-code to native API    */
/*                     for OSD manipulation.                                */
/*                                                                          */
/* Author:             Dave Wilson (using Rob Tilton's values)              */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1997                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: osddefs.h, 5, 2/5/03 12:41:16 PM, Miles Bintz$
$Log: 
 5    mpeg      1.4         2/5/03 12:41:16 PM     Miles Bintz     SCR(s) 5227 
       :
       Added new line to end of file
       
       
 4    mpeg      1.3         6/23/99 11:46:04 AM    Dave Wilson     Added 
       definitions to support ALPHABOTHVIDEO for Neches.
       
 3    mpeg      1.2         5/31/99 3:57:26 PM     Dave Wilson     Added DATA 
       definition to allow EVID window to be used for still images.
       
 2    mpeg      1.1         10/22/98 11:32:14 AM   Rob Tilton      Added Video 
       Decoder defines.
       
 1    mpeg      1.0         10/9/98 5:56:10 PM     Dave Wilson     
$
 * 
 *    Rev 1.4   05 Feb 2003 12:41:16   bintzmf
 * SCR(s) 5227 :
 * Added new line to end of file
 * 
 * 
 *    Rev 1.3   23 Jun 1999 10:46:04   dawilson
 * Added definitions to support ALPHABOTHVIDEO for Neches.
 * 
 *    Rev 1.2   31 May 1999 14:57:26   dawilson
 * Added DATA definition to allow EVID window to be used for still images.
 *
 *    Rev 1.1   22 Oct 1998 10:32:14   rtilton
 * Added Video Decoder defines.
 *
 *    Rev 1.0   09 Oct 1998 16:56:10   dawilson
 * Initial revision.
*/

#ifndef _OSDDEFS_H_
#define _OSDDEFS_H_

/**************************/
/* OSD Region Pixel Modes */
/**************************/
#define OSD_MODE_4ARGB           0
#define OSD_MODE_4AYUV           1
#define OSD_MODE_8ARGB           2
#define OSD_MODE_8AYUV           3
#define OSD_MODE_16ARGB          4
#define OSD_MODE_16AYUV          5
#define OSD_MODE_16RGB           6
#define OSD_MODE_16YUV655        7
#define OSD_MODE_16YUV422        8

/**********************/
/* OSD Region Options */
/**********************/
#define OSD_RO_LOADPALETTE       0x00000001
#define OSD_RO_BSWAPACCESS       0x00000002
#define OSD_RO_FFENABLE          0x00000004
#define OSD_RO_ALPHAENABLE       0x00000008
#define OSD_RO_COLORKEYENABLE    0x00000010
#define OSD_RO_ARCENABLE         0x00000020
#define OSD_RO_ALPHATOPVIDEO     0x00000040
#define OSD_RO_FORCEREGIONALPHA  0x00000080
#define OSD_RO_4BPPPALINDEX      0x00000100
#define OSD_RO_ENABLE            0x00000200
#define OSD_RO_FRAMEBUFFER       0x00000400
#define OSD_RO_FRAMESTRIDE       0x00000800
#define OSD_RO_MODE              0x00001000
#define OSD_RO_ALPHABOTHVIDEO    0x00002000

#define PALETTE_ALPHA 1
#define REGION_ALPHA  2

/*********************/
/* Video Plane Types */
/*********************/
#define MPG_VIDEO                1
#define LIVE_VIDEO               2

/************************************/
/* Video Decoder Input Select Types */
/************************************/
#define TUNER                    0
#define COMPOSITE                1
#define SVIDEO                   2
#define DATA                     3

/******************************/
/* Video Decoder Option Types */
/******************************/
#define LVOPTION_BRIGHTNESS      1
#define LVOPTION_CONTRAST        2
#define LVOPTION_SATURATION      3
#define LVOPTION_HUE             4
#define LVOPTION_LV_BUF_PTR      5
#define LVOPTION_BUF_STRIDE      6

#endif



