/***************************************************************************
                                                                          
   Filename: CONTEXT.H

   Description: Colorado Graphics Accelerator (GXA) Hardware
                Code to set drawing attributes and context

   Created: 9/22/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: context.h, 1, 8/23/00 12:17:30 PM, Lucy C Allevato$
$Log: 
 1    mpeg      1.0         8/23/00 12:17:30 PM    Lucy C Allevato 
$
 * 
 *    Rev 1.0   23 Aug 2000 11:17:30   eching
 * Initial revision.
 * 
 *    Rev 1.3   31 Mar 2000 16:50:36   eching
 * Updates for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.2   23 Mar 2000 15:48:20   eching
 * Updates to private context routines. Some routines moved into gfxlib.h
 * since theay are now public library routines.
 * 
 *    Rev 1.1   21 Mar 2000 16:30:12   eching
 * Changed function prefix from Hw to Gxa for context register access routines.
 * 
 *    Rev 1.0   06 Mar 2000 14:26:18   eching
 * Initial revision.
****************************************************************************/

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "kal.h"
#include "genmac.h"

/**************
 * Defines *
 **************/

/* Drawing Context Configuration Defaults */

#ifdef PCI_GXA
#define GXA_CFG_DEFAULT       TRANSP_ENABLE|MONO_FLIP
#define GXA_FIFO_DEFAULT      0x00000000l
#else
#define GXA_CFG_DEFAULT       TRANSP_ENABLE|MONO_FLIP
#define GXA_CFG2_DEFAULT      GXA_EN_RFF_BUSY
#endif

#define LINE_CONTROL_DEFAULT  NT_REVERSIBLE|SKIP_LAST

/* Bitmap Context Types */
#define BM_TYPE_COLOR   (0x00 << 24)
#define BM_TYPE_MONO    (0x01 << 24)
//#define BM_TYPE_HOST    (0x02 << 24)    // Not used in Colorado
#define BM_TYPE_PATT    (0x04 << 24)
#define BM_TYPE_SOLID   (0x08 << 24)
#define BM_TYPE_PATSZ8  (0x10 << 24)
#define BM_TYPE_PATSZ16 (0x20 << 24)
#define BM_TYPE_PATSZ32 (0x30 << 24)

/* Bitmap Context Formats */
#define BM_FMT_8BPP     (0x01 << 16)
#define BM_FMT_16BPP    (0x02 << 16)
#define BM_FMT_32BPP    (0x03 << 16)
#define BM_FMT_YCC      (0x04 << 16)

#define BM_FMT_RGB8     (0x01 << 16)
#define BM_FMT_YCC8     (0x05 << 16)
#define BM_FMT_ARGB32   (0x03 << 16)
#define BM_FMT_AYCC32   (0x07 << 16)
#define BM_FMT_ARGB4444 (0x02 << 16)
#define BM_FMT_AYCC4444 (0x06 << 16)
#define BM_FMT_ARGB0565 (0x0A << 16)
#define BM_FMT_AYCC0655 (0x0E << 16)
#define BM_FMT_AYCC2644 (0x16 << 16)
#define BM_FMT_ARGB1555 (0x12 << 16)

/* Bitmap Context Pitches */
#define BM_PITCH_512         0x200
#define BM_PITCH_640         0x280
#define BM_PITCH_720         0x2D0
#define BM_PITCH_800         0x320
#define BM_PITCH_912         0x390
#define BM_PITCH_1024        0x400
#define BM_PITCH_1280        0x500
#define BM_PITCH_1600        0x640
#define BM_PITCH_2048        0x800

// These are the defines to be ORed into commands sent to
// specify the source and destination context registers
// involved in the command.

#ifdef PCI_GXA

// BMAP0
#define BMAP0_SRC  0x0000
#define BMAP0_DST  0x0000

// BMAP1
#define BMAP1_SRC  0x0800
#define BMAP1_DST  0x0100

// BMAP2
#define BMAP2_SRC  0x1000
#define BMAP2_DST  0x0200

// BMAP3
#define BMAP3_SRC  0x1800
#define BMAP3_DST  0x0300

// BMAP4
#define BMAP4_SRC  0x2000
#define BMAP4_DST  0x0400

// BMAP5
#define BMAP5_SRC  0x2800
#define BMAP5_DST  0x0500

// BMAP6
#define BMAP6_SRC  0x3000

// BMAP7
#define BMAP7_SRC  0x3800

// Masks for the SRC and DST fields
#define SRC_FIELD  0x3800
#define DST_FIELD  0x0700

// Shift values for the SRC and DST within the command
#define SRC_RJ_AMT 11
#define DST_RJ_AMT 8

#else

// BMAP0
#define BMAP0_SRC  0x0000
#define BMAP0_DST  0x0000

// BMAP1
#define BMAP1_SRC  0x0100
#define BMAP1_DST  0x0020

// BMAP2
#define BMAP2_SRC  0x0200
#define BMAP2_DST  0x0040

// BMAP3
#define BMAP3_SRC  0x0300
#define BMAP3_DST  0x0060

// BMAP4
#define BMAP4_SRC  0x0400
#define BMAP4_DST  0x0080

// BMAP5
#define BMAP5_SRC  0x0500
#define BMAP5_DST  0x00A0

// BMAP6
#define BMAP6_SRC  0x0600

// BMAP7
#define BMAP7_SRC  0x0700

// Masks for the SRC and DST fields
#define SRC_FIELD  0x0700
#define DST_FIELD  0x00E0

// Shift values for the SRC and DST within the command
#define SRC_RJ_AMT 8
#define DST_RJ_AMT 5

#endif // PCI_GXA

/**************
 *   Macros   *
 **************/

/***************************************************************************
 mSetGXAConfig
***************************************************************************/
#define mSetGXAConfig(Value) \
mCheckFifo(1) \
mLoadReg(GXA_CFG_REG, Value)

/***************************************************************************
 mGetGXAConfig
***************************************************************************/
#define mGetGXAConfig(Value) \
mWaitForFifoEmpty \
Value = *(u_int32 *)(GXA_NON_QUEUED | GXA_CFG_REG);

/***************************************************************************
 mSetGXAConfig2
***************************************************************************/
#define mSetGXAConfig2(Value) \
mCheckFifo(1) \
mLoadReg(GXA_CFG2_REG, Value)

/***************************************************************************
 mGetGXAConfig2
***************************************************************************/
#define mGetGXAConfig2(Value) \
mWaitForFifoEmpty \
Value = *(u_int32 *)(GXA_NON_QUEUED | GXA_CFG2_REG);

/***************************************************************************
 mSetFGColor
***************************************************************************/
#define mSetFGColor(Color) \
mCheckFifo(1) \
mLoadReg(GXA_FG_COLOR_REG,Color)

/***************************************************************************
 mGetFGColor
***************************************************************************/
#define mGetFGColor(Color) \
mWaitForFifoEmpty \
Color = *(u_int32 *)(GXA_NON_QUEUED | GXA_FG_COLOR_REG);

/***************************************************************************
 mSetBGColor
***************************************************************************/
#define mSetBGColor(Color) \
mCheckFifo(1) \
mLoadReg(GXA_BG_COLOR_REG,Color)

/***************************************************************************
 mGetBGColor
***************************************************************************/
#define mGetBGColor(Color) \
mWaitForFifoEmpty \
Color = *(u_int32 *)(GXA_NON_QUEUED | GXA_BG_COLOR_REG);

/***************************************************************************
 mSetLineControl
***************************************************************************/
#define mSetLineControl(Value) \
mCheckFifo(1) \
mLoadReg(GXA_LINE_CONTROL_REG,Value)

/***************************************************************************
 mGetLineControl
***************************************************************************/
#define mGetLineControl(Value) \
mWaitForFifoEmpty \
Value = *(u_int32 *)(GXA_NON_QUEUED | GXA_LINE_CONTROL_REG);

/***************************************************************************
 mSetLinePattern
***************************************************************************/
#define mSetLinePattern(Value) \
mCheckFifo(1) \
mLoadReg(GXA_LINE_PATT_REG,Value)

/***************************************************************************
 mGetLinePattern
***************************************************************************/
#define mGetLinePattern(Value) \
mWaitForFifoEmpty \
Value = *(u_int32 *)(GXA_NON_QUEUED | GXA_LINE_PATT_REG);

/***************************************************************************
 mSetBltControl
***************************************************************************/
#define mSetBltControl(Value) \
mCheckFifo(1) \
mLoadReg(GXA_BLT_CONTROL_REG,Value)

/***************************************************************************
 mGetBltControl
***************************************************************************/
#define mGetBltControl(Value) \
mWaitForFifoEmpty \
Value = *(u_int32 *)(GXA_NON_QUEUED | GXA_BLT_CONTROL_REG);

/**************
 * Prototypes *
 **************/

void GxaSetGxaConfig(u_int32 Value);
u_int32 GxaGetGxaConfig(void);
u_int32 GxaGetLineControl(void);
void GxaSetBltControl(u_int32 Value);
u_int32 GxaGetBltControl(void);
void GxaSetupBitmapContext( u_int8 BMIdx,
                           u_int32 Type,
                           u_int32 Format,
                           u_int32 Pitch,
                           u_int32 Addr);

#endif // _CONTEXT_H_

