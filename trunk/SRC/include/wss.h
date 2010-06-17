/*****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                   */
/*                       SOFTWARE FILE/MODULE HEADER                         */
/*                      Conexant Systems Inc. (c) 2003                       */
/*                             Bristol, UK                                   */
/*                         All Rights Reserved                               */
/*****************************************************************************/
/*
 * Filename:    wss.h
 *
 *
 * Description: API and chip definitions for WSS register control.
 *              This allows the video encoder chip to send detailed information
 *              about the aspect ratio of the picture, to the television.
 *
 * Author:      Steve Jones
 *
 *****************************************************************************/
/* $Header:
 *****************************************************************************/

/* Check that the file has not already been included *************************/
#ifndef WSS_H
#define WSS_H

#include "kal.h"

#define NO_OF_REGS            (1)

#define WSS_ENCODER_UNKNOWN       (0x0)
#define WSS_BT861_LIKE            (0x1)
#define WSS_ENCODER_INTERNAL      (0x2)

#define WSS_UNKNOWN_INTERFACE     (0xFFF0)
#define WSS_I2C_INTERFACE         (0xFFF1)
#define WSS_MEM_MAP_INTERFACE     (0xFFF2)


#define WSS_ENCODER_TYPE      WSS_ENCODER_UNKNOWN
#define WSS_INTERFACE_TYPE    WSS_UNKNOWN_INTERFACE

#if VIDEO_ENCODER_0 == NOT_PRESENT
/* Internal encoder */

#undef WSS_ENCODER_TYPE
#define WSS_ENCODER_TYPE      WSS_ENCODER_INTERNAL

#undef WSS_INTERFACE_TYPE
#define WSS_INTERFACE_TYPE    WSS_MEM_MAP_INTERFACE

#else

#ifdef VIDEO_ENCODER_INCLUDE_BT861

#undef WSS_ENCODER_TYPE
#define WSS_ENCODER_TYPE   WSS_BT861_LIKE

#undef WSS_INTERFACE_TYPE
#define WSS_INTERFACE_TYPE    WSS_I2C_INTERFACE

#endif
#endif

#if (WSS_ENCODER_TYPE == WSS_BT861_LIKE)
#define ENCODER_ADDR I2C_ADDR_BT861
#define ENCODER_BUS  I2C_BUS_BT861
#undef NO_OF_REGS
#define NO_OF_REGS   (3)
#endif

#if (WSS_ENCODER_TYPE) != (WSS_ENCODER_UNKNOWN)
#define ENCODER_SUPPORTED  (1)
#else
#define ENCODER_SUPPORTED  (0)
#endif

#define DTG_FULL_FORMAT_4_3      (0x1)
#define DTG_BOX_14_9_CENTRE      (0x8)
#define DTG_BOX_14_9_TOP         (0x4)
#define DTG_BOX_16_9_CENTRE      (0xd) /*changed*/
#define DTG_BOX_16_9_TOP         (0x2)
#define DTG_BOX_GRTR_16_9        (0xb) /*changed*/
#define DTG_FF_4_3_WITH_14_9_SP  (0x7)
#define DTG_FULL_FORMAT_16_9     (0xe) /*changed*/

typedef enum
{
   WSS_SUCCESS                = 0,
   WSS_ENCODER_NOT_SUPPORTED
}
WSS_STATUS; /* ("ws" Prefix) */

const char *cnxt_wss_string(u_int32 ui32Code);
WSS_STATUS  cnxt_wss(u_int32 ui32NoRegs, ...);

#endif /* WSS_H */

/****************************************************************************
* Modifications:
* $Log: 
*  5    mpeg      1.4         2/27/04 11:42:33 AM    Steven Jones    CR(s) 8433
*         8436 : Add support for internal video encoders (really this time).
*  4    mpeg      1.3         2/27/04 10:51:31 AM    Steven Jones    CR(s) 8433
*         8436 : Add support for internal video encoders.
*  3    mpeg      1.2         2/3/03 10:43:54 AM     Steven Jones    SCR(s): 
*        5378 
*        Change some minor typos.
*        
*  2    mpeg      1.1         1/28/03 8:21:48 AM     Steven Jones    SCR(s): 
*        5331 
*        Move cnxt_wss_string() from here to wss.c.
*        
*  1    mpeg      1.0         1/24/03 8:21:58 AM     Steven Jones    
* $
/* 
/* 8     4/02/03 16:03 Yelena.konyukh
/* fixed the defines of WSS values
 * 
 *    Rev 1.1   28 Jan 2003 08:21:48   joness
 * SCR(s): 5331 
 * Move cnxt_wss_string() from here to wss.c.
 * 
 *    Rev 1.0   24 Jan 2003 08:21:58   joness
 * SCR(s): 5308 
 * API, defines and types associated with new WSS driver.
* 
*
*****************************************************************************/

