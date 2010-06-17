/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        hwconfig.h
 *
 *
 * Description:     Public header file containing labels for all the
 *                  hardware configuration values passed into code
 *                  during compilation using the config file mechanism
 *
 *
 * Author:          Dave Wilson
 *
 ****************************************************************************/
/* $Header: hwconfig.h, 164, 3/15/04 10:34:13 AM, Matt Korte$
 ****************************************************************************/

#ifndef __STBCFG_H__
   /* Should generate a "warning" here, but don't recall how */
   #include "stbcfg.h"
#endif   /*#ifndef __STBCFG_H__*/

#ifndef  __HWCONFIG_H__
#define  __HWCONFIG_H__

/********************************************************************************/
/********************************************************************************/
/**	                                                                           **/
/** It is ABSOLUTELY VITAL that definitions in this file match definitions of  **/
/** the same label in HWCONFIG.VXA and HWCONFIG.A!!!!!! If this is not the     **/
/** case, build inconsistencies will occur.                                    **/
/**                                                                            **/
/********************************************************************************/
/********************************************************************************/


/****************************************************************************/
/*                                                                          */
/* This file contains definitions for all the strings that may be passed in */
/* Sabine build process configuration files. Labels are passed to source    */
/* files using -D<label>=<value> command line switches and source files     */
/* may then use these labels to decide which sections of code or parameters */
/* are required for a given build, for example, using the #if macro as in:  */
/*                                                                          */
/* #if <label> == <value>                                                   */
/*                                                                          */
/* By defining macros for all the possible <value>s, these lines can be     */
/* made more readable, for example:                                         */
/*                                                                          */
/* #if RTOS == PSOS                                                           */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* NOTE:                                                                    */
/*                                                                          */
/* Please DO NOT use double-slash comments in this file. It is used by some */
/* OpenTV o-code applications and the o-code compiler falls over whenever   */
/* it hits a non-standard comment.                                          */
/*                                                                          */
/****************************************************************************/

#include "warnfix.h"

/****************************************************************************/
/*                                                                          */
/* Hardware register access macros                                          */
/*                                                                          */
/****************************************************************************/

#define RMO(y)                   ( ((y) & 0x00000001) ?  0 : \
                                   ( ((y) & 0x00000002) ?  1 : \
                                     ( ((y) & 0x00000004) ?  2 : \
                                       ( ((y) & 0x00000008) ?  3 : \
                                         ( ((y) & 0x00000010) ?  4 : \
                                           ( ((y) & 0x00000020) ?  5 : \
                                             ( ((y) & 0x00000040) ?  6 : \
                                               ( ((y) & 0x00000080) ?  7 : \
                                                 ( ((y) & 0x00000100) ?  8 : \
                                                   ( ((y) & 0x00000200) ?  9 : \
                                                     ( ((y) & 0x00000400) ? 10 : \
                                                       ( ((y) & 0x00000800) ? 11 : \
                                                         ( ((y) & 0x00001000) ? 12 : \
                                                           ( ((y) & 0x00002000) ? 13 : \
                                                             ( ((y) & 0x00004000) ? 14 : \
                                                               ( ((y) & 0x00008000) ? 15 : \
                                                                 ( ((y) & 0x00010000) ? 16 : \
                                                                   ( ((y) & 0x00020000) ? 17 : \
                                                                     ( ((y) & 0x00040000) ? 18 : \
                                                                       ( ((y) & 0x00080000) ? 19 : \
                                                                         ( ((y) & 0x00100000) ? 20 : \
                                                                           ( ((y) & 0x00200000) ? 21 : \
                                                                             ( ((y) & 0x00400000) ? 22 : \
                                                                               ( ((y) & 0x00800000) ? 23 : \
                                                                                 ( ((y) & 0x01000000) ? 24 : \
                                                                                   ( ((y) & 0x02000000) ? 25 : \
                                                                                     ( ((y) & 0x04000000) ? 26 : \
                                                                                       ( ((y) & 0x08000000) ? 27 : \
                                                                                         ( ((y) & 0x10000000) ? 28 : \
                                                                                           ( ((y) & 0x20000000) ? 29 : \
                                                                                             ( ((y) & 0x40000000) ? 30 : \
                                                                                               ( ((y) & 0x80000000) ? 31 : 0 ))))))))))))))))))))))))))))))))

/*
 * Access macros used to get, set/clear bits within a hardware register.
 * These macros *do not* perform any automatic shifting of bits and are
 * meant to be used with bit definitions which include their encoded bit
 * position within the register definition (e.g. an enable bit).
 */
#define CNXT_GET(reg,mask)               (*(LPREG)(reg) & (mask))
#define CNXT_SET(reg,mask,val)           (*(LPREG)(reg)) =                     \
                                  ((*(LPREG)(reg) & ~(mask)) | ((val) & (mask)))

/*
 * Access macros used to get & set a numerical value within a hardware
 * register.  These macros perform automatic shifting (based on the mask)
 * of the numerical value used.  These macros are useful for setting a
 * numerical value into a multi-bit contiguous field within a register.
 */
#define CNXT_GET_VAL(reg,mask)           ((*(LPREG)(reg) & (mask)) >> RMO(mask))
#define CNXT_SET_VAL(reg,mask,val)       (*(LPREG)(reg)) =                     \
                   ((*(LPREG)(reg) & ~(mask)) | (((unsigned long)(val) << RMO(mask)) & (mask)))

typedef unsigned int    BIT_FLD;
typedef unsigned long   HW_DWORD;      /* was u_int32; */
typedef unsigned short  HW_WORD;       /* was u_int16; */
typedef unsigned char   HW_BYTE;       /* was u_int8; */
typedef unsigned int    HW_BOOL;       /* was bool; */
typedef void            HW_VOID;

/********************************************************************************/
/* The following header file, generated automatically from HWCONFIG.CFG during  */
/* the build process, contains definitions of all the values that can be        */
/* taken by all config file keys.                                               */
/********************************************************************************/
#include "hwopts.h"

/*********************************************************************/
/* Used for any run-time chip detection code (i.e. download utility) */
/*********************************************************************/
extern unsigned long ChipID;
#define ISCOLORADO     (((ChipID&0xFFF0FFFF)==(PCI_VEND_DEV_ID_COLO_4900 & 0xFFF0FFFF))?1:0)
#define ISHONDO        (((ChipID&0xFFF0FFFF)==(PCI_VEND_DEV_ID_HONDO_2492 & 0xFFF0FFFF))?1:0)
#define ISWABASH       (((ChipID&0xFFF0FFFF)==(PCI_VEND_DEV_ID_WABASH_4430 & 0xFFF0FFFF))?1:0)
#define ISBRAZOS       (((ChipID&0xFFF0FFFF)==(PCI_VEND_DEV_ID_BRAZOS & 0xFFF0FFFF))?1:0)

/***************************************/
/* Include Vendor Specific Definitions */
/* For runtime selection, all files    */
/* must be included.  If a compile-time*/
/* solution is used, specific vendor   */
/* files can be added or removed here. */
/***************************************/
#include <conexant.h>
#if CUSTOMER == VENDOR_A
#include <vendor_a.h>
#endif
#if CUSTOMER == VENDOR_B
#include <vendor_b.h>
#endif
#if CUSTOMER == VENDOR_C
#include <vendor_c.h>
#endif
#if CUSTOMER == VENDOR_D
#include <vendor_d.h>
#endif

/****************************************************************************/
/* Here, include the BOXCFG file, translated from the CONFIG file           */
/*                                                                          */
/* NOTE:  All FEATURE type definitions must have been defined before this   */
/*    file is included.  The only thing after this line should be the       */
/*    assignment of default values                                          */
/*                                                                          */
/****************************************************************************/
#include "boxcfg.h"

/****************************************************************************/
/* Here, all newly defined FEATURE (from CONFIG files) must be assigned a   */
/* default value.                                                           */
/*                                                                          */
/* NOTE:  All default values should be assigned such that the behavior for  */
/*    the specific feature will revert to Colorado/Klondike type behavior   */
/*                                                                          */
/* There MUST not be ANY chip-specific default values defined here.         */
/* They should ONLY be defined in each chip header file.                    */
/****************************************************************************/

/* This file is generated automatically from HWCONFIG.CFG during the */
/* build process.                                                    */
#include "hwdefault.h"

/************************************************************************/
/* Some helper macros that depend upon definitions from the main config */
/* file and the current hardware config file.                           */
/************************************************************************/
#define IRD_HAS_EXTERNAL_SAT_DEMOD ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_SAT_24106_24110) ||  \
                                    (EXTERNAL_TS_SRC1 == EXT_TS_SRC_SAT_24106_24110) ||  \
                                    (EXTERNAL_TS_SRC2 == EXT_TS_SRC_SAT_24106_24110) ||  \
                                    (EXTERNAL_TS_SRC3 == EXT_TS_SRC_SAT_24106_24110) ||  \
                                    (EXTERNAL_TS_SRC0 == EXT_TS_SRC_SAT_24121_24130) ||  \
                                    (EXTERNAL_TS_SRC1 == EXT_TS_SRC_SAT_24121_24130) ||  \
                                    (EXTERNAL_TS_SRC2 == EXT_TS_SRC_SAT_24121_24130) ||  \
                                    (EXTERNAL_TS_SRC3 == EXT_TS_SRC_SAT_24121_24130))

#define IRD_HAS_INTERNAL_SAT_DEMOD ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_SAT_CX2415X) ||  \
                                    (EXTERNAL_TS_SRC1 == EXT_TS_SRC_SAT_CX2415X) ||  \
                                    (EXTERNAL_TS_SRC2 == EXT_TS_SRC_SAT_CX2415X) ||  \
                                    (EXTERNAL_TS_SRC3 == EXT_TS_SRC_SAT_CX2415X))
    
#define IRD_HAS_SAT_DEMOD ( IRD_HAS_EXTERNAL_SAT_DEMOD | IRD_HAS_INTERNAL_SAT_DEMOD )

#define IRD_HAS_INTERNAL_CABLE_DEMOD  ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_CAB_24430) ||  \
                                       (EXTERNAL_TS_SRC1 == EXT_TS_SRC_CAB_24430) ||  \
                                       (EXTERNAL_TS_SRC2 == EXT_TS_SRC_CAB_24430) ||  \
                                       (EXTERNAL_TS_SRC3 == EXT_TS_SRC_CAB_24430))
    
/* Now we support the external thomson cable demods */
#define IRD_HAS_EXTERNAL_CABLE_DEMOD  ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_CAB_DCF8722) || \
                                       (EXTERNAL_TS_SRC1 == EXT_TS_SRC_CAB_DCF8722) || \
                                       (EXTERNAL_TS_SRC2 == EXT_TS_SRC_CAB_DCF8722) || \
                                       (EXTERNAL_TS_SRC3 == EXT_TS_SRC_CAB_DCF8722))

#define IRD_HAS_CABLE_DEMOD   (IRD_HAS_INTERNAL_CABLE_DEMOD | IRD_HAS_EXTERNAL_CABLE_DEMOD)

#define IRD_HAS_TER_DEMOD ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_TER_CX22702_TDLB7)   ||  \
                             (EXTERNAL_TS_SRC1 == EXT_TS_SRC_TER_CX22702_TDLB7) ||  \
                             (EXTERNAL_TS_SRC2 == EXT_TS_SRC_TER_CX22702_TDLB7) ||  \
                             (EXTERNAL_TS_SRC3 == EXT_TS_SRC_TER_CX22702_TDLB7) ||  \
                             (EXTERNAL_TS_SRC0 == EXT_TS_SRC_TER_CX22702_T7575) ||  \
                             (EXTERNAL_TS_SRC1 == EXT_TS_SRC_TER_CX22702_T7575) ||  \
                             (EXTERNAL_TS_SRC2 == EXT_TS_SRC_TER_CX22702_T7575) ||  \
                             (EXTERNAL_TS_SRC3 == EXT_TS_SRC_TER_CX22702_T7575) ||  \
                             (EXTERNAL_TS_SRC0 == EXT_TS_SRC_TER_CX22702_T7579) ||  \
                             (EXTERNAL_TS_SRC1 == EXT_TS_SRC_TER_CX22702_T7579) ||  \
                             (EXTERNAL_TS_SRC2 == EXT_TS_SRC_TER_CX22702_T7579) ||  \
                             (EXTERNAL_TS_SRC3 == EXT_TS_SRC_TER_CX22702_T7579) ||  \
                             (EXTERNAL_TS_SRC0 == EXT_TS_SRC_TER_CX22702_T7580) ||  \
                             (EXTERNAL_TS_SRC1 == EXT_TS_SRC_TER_CX22702_T7580) ||  \
                             (EXTERNAL_TS_SRC2 == EXT_TS_SRC_TER_CX22702_T7580) ||  \
                             (EXTERNAL_TS_SRC3 == EXT_TS_SRC_TER_CX22702_T7580))

#define IRD_HAS_DVB_DEMOD ((EXTERNAL_TS_SRC0 == EXT_TS_SRC_BASEBAND_DVB) ||  \
                             (EXTERNAL_TS_SRC1 == EXT_TS_SRC_BASEBAND_DVB) ||  \
                             (EXTERNAL_TS_SRC2 == EXT_TS_SRC_BASEBAND_DVB) ||  \
                             (EXTERNAL_TS_SRC3 == EXT_TS_SRC_BASEBAND_DVB))

#define  FRONT_PANEL_IS_SCANBTN  \
 ( (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_KLONDIKE) || \
   (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_ABILENE)  || \
   (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_ATHENS)   || \
   (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_BRONCO)   || \
   (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_EUREKA)   || \
   (FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_VEND_D_PROD_1) )

#if (EXTERNAL_ENCODER == DETECT_BT861_BT865) || (EXTERNAL_ENCODER == BT861) || (INTERNAL_ENCODER == INTERNAL_BT861_LIKE)
  #define VIDEO_ENCODER_INCLUDE_BT861
#endif

#if (EXTERNAL_ENCODER == DETECT_BT861_BT865) || (EXTERNAL_ENCODER == BT865)
  #define VIDEO_ENCODER_INCLUDE_BT865
#endif

/***************************************************************/
/* Include the automatically generated file which pulls in the */
/* appropriate chip header file.                               */
/*                                                             */
/* This MUST follow the box configuration file inclusion AND   */
/* the default configuration definitions.                      */
/*                                                             */
/* There MUST not be ANY chip-specific default values defined  */
/* here. They should ONLY be defined in each chip header file. */
/***************************************************************/
#include "chiphdr.h"

#endif   /* #ifndef  __HWCONFIG_H__ */

/****************************************************************************
 * $Log: 
 *  164  mpeg      1.163       3/15/04 10:34:13 AM    Matt Korte      CR(s) 
 *        8566 : Add support for Thomson Cable Tuner/Demod
 *  163  mpeg      1.162       6/19/03 10:34:38 AM    Ian Mitchell    SCR(s): 
 *        6773 
 *        IRD_HAS_TER_DEMOD macro modified to check for new terrestrial nim 
 *        types.
 *        
 *  162  mpeg      1.161       3/25/03 1:51:40 PM     Bobby Bradford  SCR(s) 
 *        5864 :
 *        Added EUREKA keypad type to list of FRONT_PANEL_IS_SCANBTN definition
 *        
 *  161  mpeg      1.160       1/24/03 8:21:56 AM     Steven Jones    SCR(s): 
 *        5308 
 *        Add useful VIDEO_ENCODER macros.
 *        
 *  160  mpeg      1.159       1/21/03 11:42:26 PM    Dave Wilson     SCR(s) 
 *        5099 :
 *        Corrected ISBRAZOS and other chip identification macros.
 *        
 *  159  mpeg      1.158       1/13/03 1:57:02 PM     Bobby Bradford  SCR(s) 
 *        5103 :
 *        Added BRONCO front panel to list that will define 
 *        FRONT_PANEL_IS_SCANBTN
 *        
 *  158  mpeg      1.157       12/12/02 5:15:48 PM    Tim White       SCR(s) 
 *        5157 :
 *        Added all run-time chip type macros which can be used for any 
 *        run-time chip type
 *        detection code (e.g. download).
 *        
 *        
 *  157  mpeg      1.156       12/5/02 4:11:12 PM     Miles Bintz     SCR(s) 
 *        5074 :
 *        changed int_ts_src to ext_ts_src
 *        
 *        
 *  156  mpeg      1.155       12/5/02 4:05:14 PM     Miles Bintz     SCR(s) 
 *        5074 :
 *        Added a few more macros to differentiate between internal and 
 *        external demods
 *        
 *        
 *  155  mpeg      1.154       11/20/02 1:24:52 PM    Dave Wilson     SCR(s) 
 *        4904 :
 *        Removed extraneous definitions of supported RTOSs which had been 
 *        added to
 *        debug a problem since fixed.
 *        
 *  154  mpeg      1.153       11/20/02 9:36:22 AM    Dave Wilson     SCR(s) 
 *        4904 :
 *        Replaced previous header with a wrapper which pulls in the 
 *        automatically
 *        generated config files headers.
 *        
 *  153  mpeg      1.152       11/6/02 4:09:02 PM     Brendan Donahe  SCR(s) 
 *        4912 :
 *        Added "SMCD" task info for SC12 (8004) smart card driver
 *        
 *        
 *  152  mpeg      1.151       11/6/02 10:18:22 AM    Dave Wilson     SCR(s) 
 *        4896 :
 *        Added Brazos options
 *        
 *  151  mpeg      1.150       10/30/02 1:37:32 PM    Bobby Bradford  SCR(s) 
 *        4866 :
 *        Add support for BURNET keypad type (single GPIO)
 *        
 *  150  mpeg      1.149       10/22/02 1:43:30 PM    Miles Bintz     SCR(s) 
 *        4819 :
 *        Added definitions for QAM Demod IF types
 *        
 *        
 *  149  mpeg      1.148       10/18/02 4:07:30 PM    Larry Wang      SCR(s) 
 *        4811 :
 *        Remove a space from the end of name strings for some tasks.
 *        
 *  148  mpeg      1.147       10/11/02 12:20:26 PM   Miles Bintz     SCR(s) 
 *        4431 :
 *        Added config option for hsx arbiter type
 *        
 *        
 *  147  mpeg      1.146       9/17/02 5:53:24 PM     Carroll Vance   SCR(s) 
 *        4613 :
 *        Added EXTERNAL_PCI_IO_MODE and it's possible values.
 *        
 *  146  mpeg      1.145       9/12/02 12:37:48 PM    Bobby Bradford  SCR(s) 
 *        4111 :
 *        Added default defintion of REDIRECTOR_PORT to the list of HWCONFIG 
 *        values.  This value is used by the REDIRECTOR task (for modem test 
 *        applications) to determine which serial port to connect/redirect the 
 *        modem to.
 *        
 *  145  mpeg      1.144       9/9/02 4:09:42 PM      Bob Van Gulick  SCR(s) 
 *        4403 :
 *        Raise priority of PCM tasks to be higher than cable modem.  Lower 
 *        priority
 *        was causing glitches with PCM audio when running in a release build
 *        
 *        
 *  144  mpeg      1.143       8/30/02 6:35:24 PM     Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Defines for using the New IIC interface
 *        
 *  143  mpeg      1.142       8/30/02 3:01:00 PM     Larry Wang      SCR(s) 
 *        4499 :
 *        Add the definitions of INTEXP_WABASH, INTEXP_COLORADO and 
 *        INTEXP_HONDO.
 *        
 *  142  mpeg      1.141       8/22/02 4:06:32 PM     Larry Wang      SCR(s) 
 *        4459 :
 *        Define PCI_ROM_MODE_REG_TYPE_COLORADO, PCI_ROM_MODE_REG_TYPE_HONDO 
 *        and PCI_ROM_MODE_REG_TYPE_WABASH.
 *        
 *  141  mpeg      1.140       8/15/02 7:24:52 PM     Tim Ross        SCR(s) 
 *        4409 :
 *        Removed inclusion of chip header from boxcfg and placed it at end of 
 *        file after box default definitions so that CHIP_REV can bve used 
 *        within the chip header. Also removed all defrault eature definitions 
 *        for chip-specific features and placed them in the chip header files.
 *        
 *  140  mpeg      1.139       8/15/02 3:55:16 PM     Tim White       SCR(s) 
 *        4398 :
 *        Added Wabash Rev_B identifier.
 *        
 *        
 *  139  mpeg      1.138       8/2/02 3:50:42 PM      Senthil Veluswamy SCR(s) 
 *        4315 :
 *        Reduced NDS Smart card task's priority from above the NDS Verifier 
 *        tasks to below the NDS Verifier tasks as that high a priority was not
 *         necessary. This task is still time critical. Should be kept above 
 *        all the other "normal" system tasks. 
 *        
 *  138  mpeg      1.137       7/24/02 3:52:04 PM     Senthil Veluswamy SCR(s) 
 *        3280 :
 *        Increased the NDS task priorities to be above the NDS Verifier tasks 
 *        which are now > 218
 *        
 *  137  mpeg      1.136       7/22/02 6:07:42 PM     Senthil Veluswamy SCR(s) 
 *        4253 :
 *        Increased NDS Smart Card task's priority to be near the other NDS 
 *        tasks
 *        
 *  136  mpeg      1.135       7/16/02 7:47:48 PM     Miles Bintz     SCR(s) 
 *        4209 :
 *        Added IRD_HAS_DVB_DEMOD macro
 *        
 *        
 *  135  mpeg      1.134       7/12/02 8:17:02 AM     Steven Jones    SCR(s): 
 *        4176 
 *        Implement / use config for Brady and other boxes with a terrestrial 
 *        NIM.
 *        
 *  134  mpeg      1.133       7/1/02 4:57:10 PM      Dave Moore      SCR(s) 
 *        3889 :
 *        Added additional Cable Modem task, lowered priority of CMCT task.
 *        
 *        
 *  133  mpeg      1.132       7/1/02 4:11:58 PM      Tim White       SCR(s) 
 *        4103 :
 *        Hondo video ucode does not support bit 18 in the mpeg video size 
 *        register waiting
 *        for the 2nd frame to decode.  Therefore on Hondo, we simply proceed 
 *        without waiting
 *        in WaitForLiveVideoToStart() with bInSync == TRUE.
 *        
 *        
 *  132  mpeg      1.131       6/21/02 3:02:08 PM     Dave Moore      SCR(s) 
 *        4069 :
 *        Added Cable Modem Tasks
 *        
 *        
 *  131  mpeg      1.130       6/21/02 11:52:54 AM    Matt Korte      SCR(s) 
 *        4072 :
 *        Bump priority of PCMM and SSRC tasks to 1 above default.
 *        
 *        
 *  130  mpeg      1.129       6/21/02 11:27:08 AM    Dave Wilson     SCR(s) 
 *        4061 :
 *        Added default for chip feature DRM_FILTER_PHASE_0_PROBLEM
 *        
 *  129  mpeg      1.128       6/18/02 12:00:00 PM    Steven Jones    SCR(s): 
 *        3960 
 *        Update Demod API.
 *        
 *  128  mpeg      1.127       6/17/02 7:48:20 PM     Miles Bintz     SCR(s) 
 *        4043 :
 *        Added definitions and default values for MPG_SYNC_BIT_POSITION
 *        
 *        
 *  127  mpeg      1.126       6/14/02 11:57:58 AM    Craig Dry       SCR(s) 
 *        4002 :
 *        Re-enable BIT_FLD for next 2 months.  Give customers more time.
 *        
 *  126  mpeg      1.125       6/13/02 1:17:38 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        removed CX24943 from has_cable_demod define...
 *        
 *  125  mpeg      1.124       6/13/02 12:43:12 PM    Craig Dry       SCR(s) 
 *        4002 :
 *        Remove BIT_FLD type
 *        
 *  124  mpeg      1.123       6/6/02 5:46:36 PM      Dave Wilson     SCR(s) 
 *        3952 :
 *        Moved new/old remote selection flags from here to CONFMGR.H
 *        
 *  123  mpeg      1.122       5/16/02 3:58:24 PM     Steve Glennon   SCR(s): 
 *        3807 
 *        Changed priority of SSRC and PCMM "PCM Audio"
 *        tasks back to default. This should allow tuning to work now.
 *        
 *        
 *  122  mpeg      1.121       5/15/02 3:18:44 AM     Steve Glennon   SCR(s): 
 *        2438 
 *        Added another set of task ID's etc. for PCM Mixing task. Adjusted the
 *         
 *        priorities of PCMM and SSRC to be above default to ensure no audio 
 *        breakup.
 *        
 *        
 *  121  mpeg      1.120       5/14/02 12:12:48 PM    Larry Wang      SCR(s) 
 *        3585 1238924 :
 *        Move the definitions of GPIO_EXTEND_IIC and GPIO_EXTEND_IO out of the
 *         condition.  Change the value of GPIO_EXTEND_IO from 0 to 2.
 *        
 *  120  mpeg      1.119       5/7/02 5:09:36 PM      Tim White       SCR(s) 
 *        3721 :
 *        Remove CNXT_REG_SET & CNXT_REG_GET macros.
 *        Added (unsigned long) cast in the CNXT_SET_VAL macro.
 *        
 *        
 *  119  mpeg      1.118       5/1/02 3:30:54 PM      Tim White       SCR(s) 
 *        3673 :
 *        Remove PLL_ bitfields usage from codebase.
 *        
 *        
 *  118  mpeg      1.117       4/26/02 3:20:10 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F
 *        
 *        
 *  117  mpeg      1.116       4/26/02 2:28:56 PM     Larry Wang      SCR(s) 
 *        3638 :
 *        Remove definition of CABLE_MODEM_TUNER_TYPE_NONE.
 *        
 *  116  mpeg      1.115       4/25/02 12:43:58 AM    Tim Ross        SCR(s) 
 *        3619 :
 *        Changed names of cable-modem tuner definitions, added a default 
 *        definition, and a definition for no tuner type.
 *        
 *  115  mpeg      1.114       4/22/02 3:11:50 PM     Tim White       SCR(s) 
 *        3585 :
 *        Add definition of NUM_OF_GPIO for COLORADO, HONDO and WABASH chips.  
 *        Define the default GPIO extender type in hwconfig.h
 *        
 *  114  mpeg      1.113       4/12/02 7:50:10 PM     Tim White       SCR(s) 
 *        3558 :
 *        Add new hardware register access macro definitions.  One set of 
 *        macros now...
 *        
 *        
 *  113  mpeg      1.112       4/11/02 3:15:14 PM     Tim White       SCR(s) 
 *        3543 :
 *        Changed BITFIELD #define to be in the <chipname>.h header files since
 *        nothing in these files can depend on config options from config files
 *         or
 *        hw/swconfig.h
 *        
 *        
 *  112  mpeg      1.111       4/10/02 5:15:18 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  111  mpeg      1.110       4/3/02 2:53:44 AM      Ian Mitchell    SCR(s): 
 *        3363 
 *        Removed APP_RESERVED_PRIO_X from file because they are unused
 *        
 *  110  mpeg      1.109       4/2/02 4:50:08 PM      Miles Bintz     SCR(s) 
 *        3469 :
 *        removed a reference to front_panel_keypad_zapata 
 *        
 *        
 *  109  mpeg      1.108       4/2/02 4:39:20 PM      Miles Bintz     SCR(s) 
 *        3469 :
 *        added defines for vendor_d's front panel
 *        
 *        
 *  108  mpeg      1.107       4/2/02 12:25:32 PM     Dave Wilson     SCR(s) 
 *        3488 :
 *        Sorted out video encoder labels and added default for 
 *        EXTERNAL_ENCODER.
 *        
 *  107  mpeg      1.106       3/27/02 3:43:24 PM     Dave Wilson     SCR(s) 
 *        3460 :
 *        Fixed problem in IRD_HAS_SAT_DEMOD macro.
 *        
 *  106  mpeg      1.105       3/26/02 3:50:02 PM     Dave Wilson     SCR(s) 
 *        3447 :
 *        Removed SATELLITE_DEMOD and renamed CABLE_DEVICE as CABLE_MODEM. 
 *        Reworked
 *        the existing EXT_TS_SRCn definitions to be more meaninful and added a
 *         couple
 *        of macros to make it easier to determine if a given configuration has
 *         a 
 *        cable and/or satellite demod installed.
 *        
 *  105  mpeg      1.104       3/26/02 8:43:12 AM     Dave Wilson     SCR(s) 
 *        3422 :
 *        Ensured that information in C and assembler versions of HWCONFIG 
 *        headers
 *        is consistent and complete. 
 *        
 *  104  mpeg      1.103       3/25/02 10:28:00 AM    Tim White       SCR(s) 
 *        3433 :
 *        Removed NEW definition which may conflict with RTOS include files.
 *        
 *        
 *  103  mpeg      1.102       3/15/02 10:13:54 AM    Ian Mitchell    SCR(s): 
 *        3329 
 *        I didn't actually make the last changes... I have now!
 *        
 *        
 *  102  mpeg      1.101       3/15/02 10:11:02 AM    Ian Mitchell    SCR(s): 
 *        3329 
 *        Added the MAUX_TASK settings changed IFCT and IFCB_TASK_PRIORITY's
 *        
 *  101  mpeg      1.100       3/15/02 9:50:28 AM     Tim White       SCR(s) 
 *        3352 :
 *        Added "NEW" label for swconfig DMXVER.
 *        
 *        
 *  100  mpeg      1.99        3/12/02 5:49:00 PM     Tim White       SCR(s) 
 *        3360 :
 *        Use PLL_TYPE_COLORADO for Hondo since it has the same PLL setup as 
 *        Colorado.
 *        
 *        
 *  99   mpeg      1.98        3/11/02 10:56:40 AM    Ian Mitchell    SCR(s): 
 *        3342 
 *        added new defines
 *        
 *        
 *  98   mpeg      1.97        3/8/02 4:16:36 PM      Dave Wilson     SCR(s) 
 *        3290 :
 *        Added task priority, stack size and name #defines for the 2 tasks 
 *        created
 *        by IOFUNCS.
 *        
 *  97   mpeg      1.96        3/8/02 4:59:28 AM      Ian Mitchell    SCR(s): 
 *        3330 
 *        Add definitions for the stack, name and priority to file
 *        
 *  96   mpeg      1.95        3/8/02 4:31:04 AM      Ian Mitchell    SCR(s): 
 *        3329 
 *        added APP_RESERVED_PRI_X's for p93test
 *        
 *  95   mpeg      1.94        3/6/02 7:07:36 AM      Ian Mitchell    SCR(s): 
 *        3312 
 *        changed the defined vale of APP_RESERVED_PRIO_3
 *        
 *  94   mpeg      1.93        3/5/02 4:40:34 PM      Miles Bintz     SCR(s) 
 *        3306 :
 *        Added the default number of button columns and rows
 *        
 *        
 *  93   mpeg      1.92        3/5/02 7:33:22 AM      Ganesh Banghar  SCR(s): 
 *        3301 
 *        ramoved UCOS definitions and placed into a header file ucosconf.h 
 *        which is part of the UCOS BSP module.
 *        
 *  92   mpeg      1.91        3/4/02 5:50:06 PM      Tim White       SCR(s) 
 *        3299 :
 *        Add ATA_TYPE definitions used in the chip header files.
 *        
 *        
 *  91   mpeg      1.90        3/4/02 11:11:42 AM     Billy Jackman   SCR(s) 
 *        3237 :
 *        Double the size of the teletext task stack to avoid reports of stack 
 *        overflow.
 *        
 *  90   mpeg      1.89        2/28/02 2:59:14 PM     Ray Mack        SCR(s) 
 *        3270 :
 *        added default values for when the USB power control isn't specified.
 *        
 *  89   mpeg      1.88        2/28/02 2:29:46 PM     Ray Mack        SCR(s) 
 *        3270 :
 *        added USB power labels
 *        
 *  88   mpeg      1.87        2/28/02 10:34:24 AM    Ganesh Banghar  SCR(s): 
 *        3268 
 *        added APP_RESERVED_PRIO_X for applications to ppickup definitions.
 *        
 *  87   mpeg      1.86        2/28/02 9:44:02 AM     Ian Mitchell    SCR(s): 
 *        3266 
 *        added OCTP and SOAR drivers
 *        
 *  86   mpeg      1.85        2/25/02 9:04:52 AM     Bobby Bradford  SCR(s) 
 *        3201 :
 *        Assorted changes to support the use of Software Configuration Files
 *        
 *  85   mpeg      1.84        2/11/02 11:15:56 AM    Ganesh Banghar  SCR(s): 
 *        3155 
 *        added definitions to be used by e2pdrv.c
 *        
 *  84   mpeg      1.83        2/8/02 11:45:14 AM     Miles Bintz     SCR(s) 
 *        3158 :
 *        added default value for mmu_cache_disable (default is 0)
 *        
 *        
 *  83   mpeg      1.82        2/8/02 11:25:32 AM     Miles Bintz     SCR(s) 
 *        3153 :
 *        changed the frequency from units of MHz to units of Hz (ie change 
 *        from a fraction number to an int)
 *        
 *        
 *  82   mpeg      1.81        2/8/02 10:22:28 AM     Miles Bintz     SCR(s) 
 *        3153 :
 *        Added default crystal_frequency definitions to hwconfig.h and .a
 *        
 *  81   mpeg      1.80        2/4/02 10:44:24 AM     Senthil Veluswamy SCR(s) 
 *        2982 :
 *        New define for 32KB EEPROM
 *        
 *  80   mpeg      1.79        2/1/02 9:53:22 AM      Ian Mitchell    SCR(s): 
 *        3101 
 *        Added the definition of UCOS and if the RTOS is UCOS the priorities 
 *        of the drivers are different
 *        
 *        
 *  79   mpeg      1.78        1/23/02 4:09:42 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Added definitions for Transport Stream sources and TS Mux types
 *        
 *  78   mpeg      1.77        1/16/02 10:09:52 AM    Dave Moore      SCR(s) 
 *        3006 :
 *        Added defines for Cable Modem types
 *        
 *        
 *  77   mpeg      1.76        12/20/01 4:29:58 PM    Tim Ross        SCR(s) 
 *        2933 :
 *        Removed incorrect TDA and LEGACY SCART_TYPE definitions. Changed 
 *        default to TDK SCART_TYPE.
 *        
 *  76   mpeg      1.75        12/20/01 11:58:50 AM   Tim Ross        SCR(s) 
 *        2933 :
 *        Added default settings for SERIAL1, SERIAL2, and TRACE_PORT.
 *        
 *  75   mpeg      1.74        12/20/01 10:36:30 AM   Tim Ross        SCR(s) 
 *        2933 :
 *        CHanged NONE to UART_NONE.
 *        
 *  74   mpeg      1.73        12/18/01 4:47:52 PM    Miles Bintz     SCR(s) 
 *        2933 :
 *        more semantic changes related to trace port and serial port 
 *        definitions
 *        
 *        
 *  73   mpeg      1.72        12/18/01 3:09:16 PM    Miles Bintz     SCR(s) 
 *        2933 :
 *        (semantics) Changed debug port to Hardware port 
 *        
 *  72   mpeg      1.71        12/18/01 11:50:30 AM   Bobby Bradford  SCR(s) 
 *        2933 :
 *        Incorporating WabashBranch changes
 *        
 *  71   mpeg      1.70        12/5/01 9:23:44 AM     Bobby Bradford  SCR(s) 
 *        2933 :
 *        Modified the definitions for RAM types ... made them more 
 *        verbose/descriptive.
 *        
 *  70   mpeg      1.69        11/26/01 4:55:30 PM    Bobby Bradford  SCR(s) 
 *        2921 :
 *        Added defines for ZAPATA and HAPPY (Emulation) keypad types, fixed a 
 *        SCART
 *        typo.
 *        
 *  69   mpeg      1.68        11/21/01 5:14:44 PM    Tim Ross        SCR(s) 
 *        2918 :
 *        Changed SCART labels from previous enum to new #defines.
 *        
 *  68   mpeg      1.67        11/20/01 11:22:46 AM   Quillian Rutherford 
 *        SCR(s) 2754 :
 *        Updates for new build system and Wabash
 *        
 *  67   mpeg      1.66        11/15/01 9:00:02 AM    Bobby Bradford  SCR(s) 
 *        2878 :
 *        Added definitions for Hondo/Wabash PCI ID and REV Number
 *        
 *  66   mpeg      1.65        11/8/01 3:42:24 PM     Bobby Bradford  SCR(s) 
 *        2753 :
 *        Defined default values for RAM, PLL and KEYPAD configurations
 *        
 *        
 *  65   mpeg      1.64        11/6/01 9:19:46 AM     Bobby Bradford  SCR(s) 
 *        2753 :
 *        Added support for MANUAL configuration of SDRAM, including extra
 *        RAM configurations available to WABASH
 *        
 *  64   mpeg      1.63        10/29/01 2:07:46 PM    Bobby Bradford  SCR(s) 
 *        2828 :
 *        Now including "boxcfg.h" at the end of the file.
 *        Also added new typedefs (e.g. HW_DWORD) to be used in "chip.h" (e.g. 
 *        colorado.h)
 *        
 *  63   mpeg      1.62        10/23/01 4:46:26 PM    Angela          SCR(s) 
 *        2718 :
 *        increased OCTP_TASK_STACK_SIZE from 0x800 to 0x1000 to work for 
 *        octopus2
 *        
 *  62   mpeg      1.61        10/12/01 2:58:20 PM    Bobby Bradford  SCR(s) 
 *        2753 :
 *        Added definition for WABASH (from Wabash Verilog code ... pci_cfg.v 
 *        Device ID)
 *        
 *  61   mpeg      1.60        9/6/01 2:51:30 PM      Tim White       SCR(s) 
 *        2592 :
 *        Modified gendemux PSI/PES handling for the new Per-Slot-PSI/PES
 *        buffering microcode.  This included some minor gendemux interface
 *        changes as well as a complete overhaul of the PSI/PES handling
 *        path internally.  Added a new PES Task.
 *        
 *        
 *  60   mpeg      1.59        6/28/01 6:11:22 PM     Amy Pratt       SCR(s) 
 *        1642 :
 *        Merged changes from 1.54.1.0 (Hondo branch) into tip.
 *        This is a one-line change, correcting the definition of HONDO.
 *        
 *        
 *  59   mpeg      1.58        6/7/01 3:33:04 PM      Tim White       SCR(s) 
 *        1958 1990 :
 *        Modified system-wide task priorities to allow IR higher priority than
 *        teletext, keep video above SI and teletext etc...
 *        
 *        
 *  58   mpeg      1.57        6/7/01 4:11:48 AM      Dave Wilson     SCR(s) 
 *        1928 1929 :
 *        Increased video task priority. This has a beneficial effect on 
 *        channel
 *        change performance.
 *        
 *  57   mpeg      1.56        5/17/01 6:28:16 PM     Tim White       
 *        DCS#1910,1911 - Raised the priority of the NDS Task to 192 since it 
 *        calls the verifier
 *        callbacks and needs to be higher priority than the NDS tasks (see 
 *        comments in file).
 *        
 *  56   mpeg      1.55        5/15/01 2:00:40 PM     Senthil Veluswamy upped 
 *        priority of ndsc_task (NSCT_TASK_PRIORITY) to fix DCS#1851
 *        
 *  55   mpeg      1.54        4/19/01 5:36:52 PM     Steve Glennon   SCR 1740:
 *        Added #defines for a new task - the PSI Buffer Full Task
 *        
 *        
 *  54   mpeg      1.53        4/19/01 5:24:54 PM     Miles Bintz     added 
 *        tasks to break apart teletext reinsertion/extraction
 *        
 *  53   mpeg      1.52        4/18/01 5:33:34 PM     Amy Pratt       DCS1642 
 *        Added chip feature definitions in preparation for new Hondo code.
 *        
 *  52   mpeg      1.51        4/6/01 2:14:24 PM      Angela          DCS#1643-
 *         increased stack size of SOAR task to 0x600
 *        
 *  51   mpeg      1.50        4/6/01 2:07:46 PM      Ray Mack        DCS 1445 
 *        -- Added new task to handle baseband input of signals
 *        
 *  50   mpeg      1.49        3/22/01 4:42:30 PM     Tim White       DCS#1421:
 *         New otv demux.  Dynamic memory allocation, task priority leveling, 
 *        stack size changes.
 *        DCS#1468:
 *        DCS#1469:
 *        
 *  49   mpeg      1.48        3/20/01 11:13:30 AM    Ismail Mustafa  DCS 
 *        #1453-1456. NDS Task size was too small. Stack overflows occured
 *        running NDSTESTS with stub verifier.
 *        
 *  48   mpeg      1.47        3/16/01 2:05:16 PM     Joe Kroesche    #1418 - 
 *        moved modem task attributes to this file
 *        
 *  47   mpeg      1.46        3/3/01 11:24:50 PM     Steve Glennon   Increased
 *         the default stack sizes of all stacks that were 0x200 or 0x300 to
 *        twice that amount.
 *        After playing with the EPG a while, yet another task (clk_rec_task) 
 *        which had
 *        a stack of 0x200 caused a stack overflow.
 *        - Yes we lose some memory, but lets be safe.
 *        
 *  46   mpeg      1.45        3/2/01 11:07:18 PM     Steve Glennon   Increased
 *         size of some stacks which were overflowing
 *        
 *  45   mpeg      1.44        3/2/01 4:04:06 PM      Tim White       DCS#1365:
 *         Globally decrease stack memory usage in the system.
 *        DCS#1366: Globally decrease stack memory usage in the system.
 *        
 *  44   mpeg      1.43        12/12/00 10:51:24 AM   Dave Wilson     DCS 850: 
 *        Added IDs for VENDOR_F and VENDOR_G.
 *        
 *  43   mpeg      1.42        10/2/00 3:55:44 PM     Dave Wilson     
 *        
 *        
 *        
 *        
 *        Added #define for VENDOR_E
 *        
 *  42   mpeg      1.41        8/29/00 7:40:18 PM     Miles Bintz     added nup
 *         for real this time
 *        
 *  41   mpeg      1.40        8/29/00 7:38:26 PM     Miles Bintz     added NUP
 *         RTOS define
 *        
 *  40   mpeg      1.39        8/15/00 3:53:40 PM     Dave Moore      added 
 *        serial port SQ&C index defines.
 *        
 *  39   mpeg      1.38        7/31/00 12:59:40 PM    Dave Moore      Modified 
 *        modem config defs to make mutually exclusive.
 *        
 *  38   mpeg      1.37        7/6/00 6:42:28 PM      Dave Moore      Added 
 *        defines for modem types.
 *        
 *  37   mpeg      1.36        6/21/00 5:32:06 PM     Tim White       Added 
 *        Colorado Rev_C changes (pin muxing changes).
 *        
 *  36   mpeg      1.35        6/2/00 12:01:30 PM     Tim White       Allow 
 *        building with specific CUSTOMER=VENDOR_X without needing
 *        to include other Vendors' include files (vendor_x.h and vendor_x.a).
 *        
 *  35   mpeg      1.34        4/27/00 6:37:06 PM     Tim White       Corrected
 *         Chip Rev ID definitions.
 *        
 *  34   mpeg      1.33        4/27/00 5:39:00 PM     Tim White       Fixed bug
 *         in board specific code.
 *        
 *  33   mpeg      1.32        4/27/00 4:35:48 PM     Dave Wilson     Removed a
 *         double-slash comment that caused o-code app builds to break
 *        
 *  32   mpeg      1.31        4/24/00 10:32:36 PM    Tim White       Removed 
 *        the RSO_ definitions from chip specific header file and
 *        placed into vendor/board specific header file.
 *        
 *  31   mpeg      1.30        4/24/00 11:17:24 AM    Tim White       Moved 
 *        strapping CONFIG0 from chip header file(s) to board header file(s).
 *        
 *  30   mpeg      1.29        4/13/00 10:35:58 AM    Dave Wilson     Added 
 *        #define CNXT as an alias for CONEXANT to save having to change the
 *        default build directory
 *        
 *  29   mpeg      1.28        3/28/00 6:00:30 PM     Dave Wilson     Added 
 *        board ID definitions for Klondike variants
 *        
 *  28   mpeg      1.27        3/14/00 5:35:54 PM     Tim Ross        Added 
 *        VENDOR_A, VENDOR_B, and VENDOR_C to represent 
 *        value returned for vendor ID field from config 0 register.
 *        
 *  27   mpeg      1.26        3/3/00 1:38:10 PM      Tim Ross        Added the
 *         NOOS option for the RTOS switch.
 *        
 *  26   mpeg      1.25        11/29/99 3:04:06 PM    Ismail Mustafa  Added 
 *        Colorado & removed CPU stuff.
 *        
 *  25   mpeg      1.24        11/1/99 4:10:14 PM     Tim Ross        Added 
 *        frequency scanning definitions.
 *        
 *  24   mpeg      1.23        11/1/99 3:08:38 PM     Dave Wilson     Moved 
 *        MAGIC definition from BSP.H to HWCONFIG.H
 *        
 *  23   mpeg      1.22        10/27/99 5:09:36 PM    Dave Wilson     Changed 
 *        NONE to HW_NONE since NONE is defined in VxWorks headers.
 *        Removed processor version labels for same reason.
 *        
 *  22   mpeg      1.21        10/14/99 7:11:06 PM    Dave Wilson     Added 
 *        device_flags info.
 *        
 *  21   mpeg      1.20        10/6/99 4:43:32 PM     Dave Wilson     Replaced 
 *        NECHES, REV_A and REV_B labels with true hardware version numbers.
 *        
 *  20   mpeg      1.19        9/2/99 12:41:32 PM     Tim Ross        Removed 
 *        flash vendor type. It is no longer needed.
 *        
 *  19   mpeg      1.18        9/1/99 2:09:16 PM      Dave Wilson     Fixed 
 *        typo in previous revision
 *        
 *  18   mpeg      1.17        9/1/99 11:12:12 AM     Dave Wilson     Added 
 *        definitions of various IRD board versions.
 *        
 *  17   mpeg      1.16        6/8/99 3:12:10 PM      Tim Ross        Added 
 *        Orion2 and Thor2 boards.
 *        
 *  16   mpeg      1.15        4/13/99 4:10:44 PM     Tim Ross        Changes 
 *        for Neches UART.
 *        
 *  15   mpeg      1.14        1/29/99 5:47:04 PM     Dave Moore      Added 
 *        HWMODEM as Serial Port Type.
 *        
 *  14   mpeg      1.13        12/18/98 9:15:42 AM    Tim Ross        Added 
 *        CHIP_NAME and CHIP_REV definitions.
 *        
 *  13   mpeg      1.12        11/11/98 10:49:24 AM   Rob Tilton      Added 
 *        define for BtFin BT861.
 *        
 *  12   mpeg      1.11        10/6/98 3:49:44 PM     Dave Wilson     Added 
 *        board IDs for PID7T-based GIR and SMC test boards
 *        
 *  11   mpeg      1.10        9/21/98 12:14:26 PM    Steve Glennon   Added 
 *        #include of warnfix.h to try to damp down some of the benign warnings
 *         
 *        in the error logs.
 *        
 *  10   mpeg      1.9         8/27/98 3:14:26 PM     Tim Ross        Added 
 *        Sabine UART support.
 *        
 *  9    mpeg      1.8         7/27/98 3:42:56 PM     Dave Wilson     Compiles 
 *        and works with o-code to native wrappers.
 *        
 *  8    mpeg      1.7         6/19/98 2:22:24 PM     Anzhi Chen      Added 
 *        definitons form ROM types.
 *        
 *  7    mpeg      1.6         6/4/98 10:34:48 AM     Dave Wilson     Added 
 *        FROM_CONFIG label to tell code to read relevant parameters from the
 *        configration block (yes, I know we have yet to define the 
 *        configuration
 *        block).
 *        
 *  6    mpeg      1.5         5/14/98 2:07:00 AM     Tim Ross        Moved CPU
 *         definitions from bsp.h to here.
 *        
 *  5    mpeg      1.4         5/12/98 12:05:20 PM    Dave Wilson     Added 
 *        NEGATIVE and POSITIVE definitions.
 *        
 *  4    mpeg      1.3         4/27/98 5:51:46 PM     Tim Ross        Added 
 *        PID7T and ARM as permissible values for board and vendor
 *        codes to allow PHASE2 & PID7T to build properly.
 *        
 *  3    mpeg      1.2         4/24/98 1:41:42 PM     Tim Ross        Added 
 *        vendor and board codes/IDs to define manufacturer and board
 *        to various code modules.
 *        
 *  2    mpeg      1.1         4/14/98 12:02:12 PM    Dave Wilson     Added new
 *         labels after config file update.
 *        
 *  1    mpeg      1.0         3/27/98 9:23:02 AM     Dave Wilson     
 * $
 * 
 *    Rev 1.162   19 Jun 2003 09:34:38   mitchei
 * SCR(s): 6773 
 * IRD_HAS_TER_DEMOD macro modified to check for new terrestrial nim types.
 * 
 *    Rev 1.161   25 Mar 2003 13:51:40   bradforw
 * SCR(s) 5864 :
 * Added EUREKA keypad type to list of FRONT_PANEL_IS_SCANBTN definition
 * 
 *    Rev 1.160   24 Jan 2003 08:21:56   joness
 * SCR(s): 5308 
 * Add useful VIDEO_ENCODER macros.
 * 
 *    Rev 1.159   21 Jan 2003 23:42:26   dawilson
 * SCR(s) 5099 :
 * Corrected ISBRAZOS and other chip identification macros.
 * 
 *    Rev 1.158   13 Jan 2003 13:57:02   bradforw
 * SCR(s) 5103 :
 * Added BRONCO front panel to list that will define FRONT_PANEL_IS_SCANBTN
 * 
 *    Rev 1.157   12 Dec 2002 17:15:48   whiteth
 * SCR(s) 5157 :
 * Added all run-time chip type macros which can be used for any run-time chip type
 * detection code (e.g. download).
 * 
 * 
 *    Rev 1.156   05 Dec 2002 16:11:12   bintzmf
 * SCR(s) 5074 :
 * changed int_ts_src to ext_ts_src
 * 
 * 
 *    Rev 1.155   05 Dec 2002 16:05:14   bintzmf
 * SCR(s) 5074 :
 * Added a few more macros to differentiate between internal and external demods
 * 
 * 
 *    Rev 1.154   20 Nov 2002 13:24:52   dawilson
 * SCR(s) 4904 :
 * Removed extraneous definitions of supported RTOSs which had been added to
 * debug a problem since fixed.
 * 
 *    Rev 1.153   20 Nov 2002 09:36:22   dawilson
 * SCR(s) 4904 :
 * Replaced previous header with a wrapper which pulls in the automatically
 * generated config files headers.
 * 
 *    Rev 1.152   06 Nov 2002 16:09:02   donaheb
 * SCR(s) 4912 :
 * Added "SMCD" task info for SC12 (8004) smart card driver
 * 
 * 
 *    Rev 1.151   06 Nov 2002 10:18:22   dawilson
 * SCR(s) 4896 :
 * Added Brazos options
 * 
 *    Rev 1.150   30 Oct 2002 13:37:32   bradforw
 * SCR(s) 4866 :
 * Add support for BURNET keypad type (single GPIO)
 * 
 *    Rev 1.149   22 Oct 2002 12:43:30   bintzmf
 * SCR(s) 4819 :
 * Added definitions for QAM Demod IF types
 * 
 * 
 *    Rev 1.148   18 Oct 2002 15:07:30   wangl2
 * SCR(s) 4811 :
 * Remove a space from the end of name strings for some tasks.
 * 
 *    Rev 1.147   11 Oct 2002 11:20:26   bintzmf
 * SCR(s) 4431 :
 * Added config option for hsx arbiter type
 * 
 * 
 *    Rev 1.146   17 Sep 2002 16:53:24   vancec
 * SCR(s) 4613 :
 * Added EXTERNAL_PCI_IO_MODE and it's possible values.
 * 
 *    Rev 1.145   12 Sep 2002 11:37:48   bradforw
 * SCR(s) 4111 :
 * Added default defintion of REDIRECTOR_PORT to the list of HWCONFIG values.  This value is used by the REDIRECTOR task (for modem test applications) to determine which serial port to connect/redirect the modem to.
 * 
 *    Rev 1.144   09 Sep 2002 15:09:42   vangulr
 * SCR(s) 4403 :
 * Raise priority of PCM tasks to be higher than cable modem.  Lower priority
 * was causing glitches with PCM audio when running in a release build
 * 
 * 
 *    Rev 1.143   30 Aug 2002 17:35:24   velusws
 * SCR(s) 4502 :
 * Defines for using the New IIC interface
 * 
 *    Rev 1.142   30 Aug 2002 14:01:00   wangl2
 * SCR(s) 4499 :
 * Add the definitions of INTEXP_WABASH, INTEXP_COLORADO and INTEXP_HONDO.
 * 
 *    Rev 1.141   22 Aug 2002 15:06:32   wangl2
 * SCR(s) 4459 :
 * Define PCI_ROM_MODE_REG_TYPE_COLORADO, PCI_ROM_MODE_REG_TYPE_HONDO and PCI_ROM_MODE_REG_TYPE_WABASH.
 * 
 *    Rev 1.140   15 Aug 2002 18:24:52   rossst
 * SCR(s) 4409 :
 * Removed inclusion of chip header from boxcfg and placed it at end of file after box default definitions so that CHIP_REV can bve used within the chip header. Also removed all defrault eature definitions for chip-specific features and placed them in the chip header files.
 * 
 *    Rev 1.139   15 Aug 2002 14:55:16   whiteth
 * SCR(s) 4398 :
 * Added Wabash Rev_B identifier.
 * 
 * 
 *    Rev 1.138   02 Aug 2002 14:50:42   velusws
 * SCR(s) 4315 :
 * Reduced NDS Smart card task's priority from above the NDS Verifier tasks to below the NDS Verifier tasks as that high a priority was not necessary. This task is still time critical. Should be kept above all the other "normal" system tasks. 
 * 
 *    Rev 1.137   24 Jul 2002 14:52:04   velusws
 * SCR(s) 3280 :
 * Increased the NDS task priorities to be above the NDS Verifier tasks which are now > 218
 * 
 *    Rev 1.136   22 Jul 2002 17:07:42   velusws
 * SCR(s) 4253 :
 * Increased NDS Smart Card task's priority to be near the other NDS tasks
 * 
 *    Rev 1.135   16 Jul 2002 18:47:48   bintzmf
 * SCR(s) 4209 :
 * Added IRD_HAS_DVB_DEMOD macro
 * 
 * 
 *    Rev 1.134   12 Jul 2002 07:17:02   joness
 * SCR(s): 4176 
 * Implement / use config for Brady and other boxes with a terrestrial NIM.
 *
 *    Rev 1.133   01 Jul 2002 15:57:10   mooreda
 * SCR(s) 3889 :
 * Added additional Cable Modem task, lowered priority of CMCT task.
 * 
 * 
 *    Rev 1.132   01 Jul 2002 15:11:58   whiteth
 * SCR(s) 4103 :
 * Hondo video ucode does not support bit 18 in the mpeg video size register waiting
 * for the 2nd frame to decode.  Therefore on Hondo, we simply proceed without waiting
 * in WaitForLiveVideoToStart() with bInSync == TRUE.
 * 
 * 
 *    Rev 1.131   21 Jun 2002 14:02:08   mooreda
 * SCR(s) 4069 :
 * Added Cable Modem Tasks
 *
 *
 *    Rev 1.130   21 Jun 2002 10:52:54   kortemw
 * SCR(s) 4072 :
 * Bump priority of PCMM and SSRC tasks to 1 above default.
 *
 *
 *    Rev 1.129   21 Jun 2002 10:27:08   dawilson
 * SCR(s) 4061 :
 * Added default for chip feature DRM_FILTER_PHASE_0_PROBLEM
 *
 *    Rev 1.128   18 Jun 2002 11:00:00   joness
 * SCR(s): 3960
 * Update Demod API.
 *
 *    Rev 1.123   06 Jun 2002 16:46:36   dawilson
 * SCR(s) 3952 :
 * Moved new/old remote selection flags from here to CONFMGR.H
 *
 *    Rev 1.122   16 May 2002 14:58:24   glennon
 * SCR(s): 3807
 * Changed priority of SSRC and PCMM "PCM Audio"
 * tasks back to default. This should allow tuning to work now.
 *
 *
 *    Rev 1.121   15 May 2002 02:18:44   glennon
 * SCR(s): 2438
 * Added another set of task ID's etc. for PCM Mixing task. Adjusted the
 * priorities of PCMM and SSRC to be above default to ensure no audio breakup.
 *
 *
 *    Rev 1.120   14 May 2002 11:12:48   wangl2
 * SCR(s) 3585 1238924 :
 * Move the definitions of GPIO_EXTEND_IIC and GPIO_EXTEND_IO out of the condition.  Change the value of GPIO_EXTEND_IO from 0 to 2.
 *
 *    Rev 1.119   07 May 2002 16:09:36   whiteth
 * SCR(s) 3721 :
 * Remove CNXT_REG_SET & CNXT_REG_GET macros.
 * Added (unsigned long) cast in the CNXT_SET_VAL macro.
 *
 *
 *    Rev 1.118   01 May 2002 14:30:54   whiteth
 * SCR(s) 3673 :
 * Remove PLL_ bitfields usage from codebase.
 *
 *
 *    Rev 1.117   26 Apr 2002 14:20:10   whiteth
 * SCR(s) 3562 :
 * Add support for Colorado Rev_F
 *
 *
 *    Rev 1.116   26 Apr 2002 13:28:56   wangl2
 * SCR(s) 3638 :
 * Remove definition of CABLE_MODEM_TUNER_TYPE_NONE.
 *
 *    Rev 1.115   24 Apr 2002 23:43:58   rossst
 * SCR(s) 3619 :
 * Changed names of cable-modem tuner definitions, added a default definition, and a definition for no tuner type.
 *
 *    Rev 1.114   22 Apr 2002 14:11:50   whiteth
 * SCR(s) 3585 :
 * Add definition of NUM_OF_GPIO for COLORADO, HONDO and WABASH chips.  Define the default GPIO extender type in hwconfig.h
 *
 *    Rev 1.113   12 Apr 2002 18:50:10   whiteth
 * SCR(s) 3558 :
 * Add new hardware register access macro definitions.  One set of macros now...
 *
 *    Rev 1.112   11 Apr 2002 14:15:14   whiteth
 * SCR(s) 3543 :
 * Changed BITFIELD #define to be in the <chipname>.h header files since
 * nothing in these files can depend on config options from config files or
 * hw/swconfig.h
 *
 *
 *    Rev 1.111   10 Apr 2002 16:15:18   whiteth
 * SCR(s) 3509 :
 * Eradicate external bus (PCI) bitfield usage.
 *
 *
 *    Rev 1.110   03 Apr 2002 02:53:44   mitchei
 * SCR(s): 3363
 * Removed APP_RESERVED_PRIO_X from file because they are unused
 *
 *    Rev 1.109   02 Apr 2002 16:50:08   bintzmf
 * SCR(s) 3469 :
 * removed a reference to front_panel_keypad_zapata
 *
 *
 *    Rev 1.108   02 Apr 2002 16:39:20   bintzmf
 * SCR(s) 3469 :
 * added defines for vendor_d's front panel
 *
 *
 *    Rev 1.107   02 Apr 2002 12:25:32   dawilson
 * SCR(s) 3488 :
 * Sorted out video encoder labels and added default for EXTERNAL_ENCODER.
 *
 *    Rev 1.106   27 Mar 2002 15:43:24   dawilson
 * SCR(s) 3460 :
 * Fixed problem in IRD_HAS_SAT_DEMOD macro.
 *
 *    Rev 1.105   26 Mar 2002 15:50:02   dawilson
 * SCR(s) 3447 :
 * Removed SATELLITE_DEMOD and renamed CABLE_DEVICE as CABLE_MODEM. Reworked
 * the existing EXT_TS_SRCn definitions to be more meaninful and added a couple
 * of macros to make it easier to determine if a given configuration has a
 * cable and/or satellite demod installed.
 *
 *    Rev 1.104   26 Mar 2002 08:43:12   dawilson
 * SCR(s) 3422 :
 * Ensured that information in C and assembler versions of HWCONFIG headers
 * is consistent and complete.
 *
 *    Rev 1.103   25 Mar 2002 10:28:00   whiteth
 * SCR(s) 3433 :
 * Removed NEW definition which may conflict with RTOS include files.
 *
 *
 *    Rev 1.102   15 Mar 2002 10:13:54   mitchei
 * SCR(s): 3329
 * I didn't actually make the last changes... I have now!
 *
 *
 *    Rev 1.100   15 Mar 2002 09:50:28   whiteth
 * SCR(s) 3352 :
 * Added "NEW" label for swconfig DMXVER.
 *
 *
 *    Rev 1.99   12 Mar 2002 17:49:00   whiteth
 * SCR(s) 3360 :
 * Use PLL_TYPE_COLORADO for Hondo since it has the same PLL setup as Colorado.
 *
 *
 *    Rev 1.98   11 Mar 2002 10:56:40   mitchei
 * SCR(s): 3342
 * added new defines
 *
 *
 *    Rev 1.97   08 Mar 2002 16:16:36   dawilson
 * SCR(s) 3290 :
 * Added task priority, stack size and name #defines for the 2 tasks created
 * by IOFUNCS.
 *
 *    Rev 1.96   08 Mar 2002 04:59:28   mitchei
 * SCR(s): 3330
 * Add definitions for the stack, name and priority to file
 *
 *    Rev 1.95   08 Mar 2002 04:31:04   mitchei
 * SCR(s): 3329
 * added APP_RESERVED_PRI_X's for p93test
 *
 *    Rev 1.94   06 Mar 2002 07:07:36   mitchei
 * SCR(s): 3312
 * changed the defined vale of APP_RESERVED_PRIO_3
 *
 *    Rev 1.93   05 Mar 2002 16:40:34   bintzmf
 * SCR(s) 3306 :
 * Added the default number of button columns and rows
 *
 *
 *    Rev 1.92   05 Mar 2002 07:33:22   banghag
 * SCR(s): 3301
 * ramoved UCOS definitions and placed into a header file ucosconf.h which is part of the UCOS BSP module.
 *
 *    Rev 1.91   04 Mar 2002 17:50:06   whiteth
 * SCR(s) 3299 :
 * Add ATA_TYPE definitions used in the chip header files.
 *
 *
 *    Rev 1.90   04 Mar 2002 11:11:42   jackmaw
 * SCR(s) 3237 :
 * Double the size of the teletext task stack to avoid reports of stack overflow.
 *
 *    Rev 1.89   28 Feb 2002 14:59:14   raymack
 * SCR(s) 3270 :
 * added default values for when the USB power control isn't specified.
 *
 *    Rev 1.88   28 Feb 2002 14:29:46   raymack
 * SCR(s) 3270 :
 * added USB power labels
 *
 *    Rev 1.87   28 Feb 2002 10:34:24   banghag
 * SCR(s): 3268
 * added APP_RESERVED_PRIO_X for applications to ppickup definitions.
 *
 *    Rev 1.85   Feb 25 2002 09:04:52   bradforw
 * SCR(s) 3201 :
 * Assorted changes to support the use of Software Configuration Files
 *
 ****************************************************************************/

