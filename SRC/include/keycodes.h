/****************************************************************************/
/*                 Conexant Systems Inc. - SABINE                           */
/****************************************************************************/
/*                                                                          */
/* Filename:           KEYCODES.H                                           */
/*                                                                          */
/* Description:        Definitions of key codes for special keys on the     */
/*                     IR remote.                                           */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: keycodes.h, 8, 2/21/02 3:59:42 PM, Dave Wilson$
*****************************************************************************/

#ifndef _KEYCODES_H_
#define _KEYCODES_H_

/* Key mappings used by O-code apps but not defined (or incorrectly defined) */
/* in OPENTV.H                                                               */
#ifdef __ocod__
#define OPTV_RED_CODE           0x001C
#define OPTV_GREEN_CODE         0x001D
#define OPTV_YELLOW_CODE        0x001F
#define OPTV_BLUE_CODE          0x001E
#define OPTV_STOP_CODE          0x0010
#define OPTV_PLAY_CODE          0x0011
#define OPTV_PAUSE_CODE         0x0012
#define OPTV_CHANNEL_DOWN_CODE  0x0500
#define OPTV_CHANNEL_UP_CODE    0x0501
#define OPTV_EXIT_CODE          0x0801

#endif

/* New application keys - handled by o-code */
#if defined(__ocod__) && (CUSTOMER==VENDOR_D)
/* NOTE that VENDOR_D o-code applications use KEY_* key names. */
#define KEY_AB_CODE             0x0090
#else
#define OPTV_AB_CODE            0x0090
#endif
#define OPTV_SLEEP_CODE         0x0094
#define OPTV_INPUT_CODE         0x0095
#define OPTV_BUY_CODE           0x0096
#define OPTV_WWW_CODE           0x0097
#define OPTV_PC_CODE            0x0098

/* New system keys - handled by Control Task */
#define OPTV_REVERSE_CODE       0x0A96
#define OPTV_FORWARD_CODE       0x0A97
#define OPTV_RECORD_CODE        0x0A98
#define OPTV_INFOR_CODE         OPTV_MENU_CODE

/* Old remote uses these buttons for teletext colour keys */
#define OPTV_PIP_CODE           OPTV_RED_CODE
#define OPTV_CHCTRL_CODE        OPTV_GREEN_CODE
#define OPTV_SIZE_CODE          OPTV_YELLOW_CODE
#define OPTV_SWAP_CODE          OPTV_BLUE_CODE

/* New remote uses these buttons for teletext colour keys */
#define OPTV_LAST_CODE          OPTV_RED_CODE
#define OPTV_LOCK_CODE          OPTV_GREEN_CODE
#define OPTV_MENUR_CODE         OPTV_YELLOW_CODE
#define OPTV_TV_VCR_CODE        OPTV_BLUE_CODE

/* Additional keycodes used by some networks but undefined in OPENTVX.H */
#define VIA_DIGITAL_KEY_ATRAS   0x0080
#define VIA_DIGITAL_KEY_AUDIO   0x1001
#define VIA_DIGITAL_KEY_SERV    0x1000
#define VIA_DIGITAL_KEY_INIC    0x0081

#endif /* _KEYCODES_H_ */

/*********************************************************************************
 * $Log: 
 *  8    mpeg      1.7         2/21/02 3:59:42 PM     Dave Wilson     SCR(s) 
 *        3227 :
 *        
 *        
 *        
 *        y
 *        Added some ViaDigital-specific keycode definitions to allow us to run
 *         some 
 *        of their apps on our Klondike EN2 build.
 *        
 *  7    mpeg      1.6         2/16/01 5:14:58 PM     Dave Wilson     DCS1217: 
 *        Changes for Vendor D rev 7 board.
 *        
 *  6    mpeg      1.5         11/8/99 5:37:26 PM     Dave Wilson     Added 
 *        some remapped codes for new remote
 *        
 *  5    mpeg      1.4         10/27/99 12:00:28 PM   Dave Wilson     Added 
 *        codes for some keys on the new remote.
 *        
 *  4    mpeg      1.3         9/21/99 12:08:58 PM    Dave Wilson     Added 
 *        code for PAUSE key.
 *        
 *  3    mpeg      1.2         5/28/99 9:46:52 AM     Dave Wilson     Added 
 *        some more codes for keys not supported in OPENTV.H
 *        
 *  2    mpeg      1.1         5/27/99 2:18:16 PM     Dave Wilson     Added two
 *         keycodes incorrectly defined in OPENTV.H.
 *        
 *  1    mpeg      1.0         5/26/99 6:16:44 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.7   21 Feb 2002 15:59:42   dawilson
 * SCR(s) 3227 :
 * 
 * 
 * 
 * y
 * Added some ViaDigital-specific keycode definitions to allow us to run some 
 * of their apps on our Klondike EN2 build.
 * 
 *    Rev 1.6   16 Feb 2001 17:14:58   dawilson
 * DCS1217: Changes for Vendor D rev 7 board.
 * 
 *    Rev 1.5   08 Nov 1999 17:37:26   dawilson
 * Added some remapped codes for new remote
 * 
 *    Rev 1.4   27 Oct 1999 11:00:28   dawilson
 * Added codes for some keys on the new remote.
 * 
 *    Rev 1.3   21 Sep 1999 11:08:58   dawilson
 * Added code for PAUSE key.
 * 
 *    Rev 1.2   28 May 1999 08:46:52   dawilson
 * Added some more codes for keys not supported in OPENTV.H
 *
 *    Rev 1.1   27 May 1999 13:18:16   dawilson
 * Added two keycodes incorrectly defined in OPENTV.H.
 *
 *    Rev 1.0   26 May 1999 17:16:44   dawilson
 * Initial revision.
 *********************************************************************************/

