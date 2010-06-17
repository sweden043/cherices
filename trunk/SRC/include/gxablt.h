/***************************************************************************
                                                                          
   Filename: GXABLT.H

   Description: Colorado Graphics Accelerator (GXA)
                Definition and Macro Utilities
                Higher level configuration and hardware programming
                defines and macros are intended to go here. The
                base level hardware definitions and bit fields are
                defined in a separate header file for inclusion into
                the global chip header file.

   Created: 9/15/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: gxablt.h, 1, 8/23/00 12:17:54 PM, Lucy C Allevato$
$Log: 
 1    mpeg      1.0         8/23/00 12:17:54 PM    Lucy C Allevato 
$
 * 
 *    Rev 1.0   23 Aug 2000 11:17:54   eching
 * Initial revision.
 * 
 *    Rev 1.0   06 Mar 2000 13:35:24   eching
 * Initial revision.
****************************************************************************/

#ifndef _GXABLT_H_
#define _GXABLT_H_

#include "genmac.h"

/* GUI Commands */

// Command Types and Parameter Count defines
// Source and Destination Context defines are in context.h

#ifdef PCI_GXA

#define RW_REG_CMD           0x0000000
//#define RW_GUI_DATA_CMD      0x0010000
#define QMARK_CMD            0x0020000
#define TEXTBLT_COPY_CMD     0x0230000
#define TEXTBLT_COPY_T_CMD   0x0270000
#define TEXTBLT_ROP_CMD      0x02B0000
#define TEXTBLT_ROP_T_CMD    0x02F0000
#define LINE_COPY_CMD        0x0320000
#define BLT_COPY_CMD         0x0330000
#define LINE_COPY_T_CMD      0x0360000
#define BLT_COPY_T_CMD       0x0370000
#define LINE_ROP_CMD         0x03A0000
#define BLT_ROP_CMD          0x03B0000
#define LINE_ROP_T_CMD       0x03E0000
#define BLT_ROP_T_CMD        0x03F0000

#define ROP_CMD              0x0080000
#define T_CMD                0x0040000
#define ONE_PARAM_CMD        0x0000020
#define TWO_PARAMS_CMD       0x0000040
#define THREE_PARAMS_CMD     0x0000060
#define FOUR_PARAMS_CMD      0x0000080
#define NUM_PARAMS_CMD       0x00000E0

#else

#define RW_REG_CMD           0x0000000
//#define RW_GUI_DATA_CMD      0x0000800
#define QMARK_CMD            0x0001000
#define PALETTE_FETCH_CMD    0x0001800
#define TEXTBLT_ALPHA_CMD    0x0010800
#define TEXTBLT_COPY_CMD     0x0011800
#define TEXTBLT_ALPHA_T_CMD  0x0012800
#define TEXTBLT_COPY_T_CMD   0x0013800
#define TEXTBLT_ROP_CMD      0x0015800
#define TEXTBLT_ROP_T_CMD    0x0017800
#define LINE_ALPHA_CMD       0x0018000
#define LINE_COPY_CMD        0x0019000
#define BLT_ALPHA_CMD        0x0018800
#define BLT_COPY_CMD         0x0019800
#define LINE_ALPHA_T_CMD     0x001A000
#define LINE_COPY_T_CMD      0x001B000
#define BLT_ALPHA_T_CMD      0x001A800
#define BLT_COPY_T_CMD       0x001B800
#define LINE_ROP_CMD         0x001D000
#define BLT_ROP_CMD          0x001D800
#define LINE_ROP_T_CMD       0x001F000
#define BLT_ROP_T_CMD        0x001F800

#define BLT_CMD              0x0000800
#define NOT_ALPHA_CMD        0x0001000
#define T_CMD                0x0002000
#define ROP_CMD              0x0004000
#define NOT_TEXT_CMD         0x0008000
#define RENDER_CMD           0x0010000
#define ONE_PARAM_CMD        0x0000004
#define TWO_PARAMS_CMD       0x0000008
#define THREE_PARAMS_CMD     0x000000C
#define FOUR_PARAMS_CMD      0x0000010
#define NUM_PARAMS_CMD       0x000001C

#endif // PCI_GXA

/* HIGHER LEVEL GUI CMDS */

// Common BLT LINE and TEXTBLT command types and parameter counts
// ORed together and given shorter names

#define QMARK                QMARK_CMD|ONE_PARAM_CMD

#define BLT_COPY             BLT_COPY_CMD|THREE_PARAMS_CMD
#define BLT_COPY_T           BLT_COPY_T_CMD|THREE_PARAMS_CMD
#define BLT_ROP              BLT_ROP_CMD|THREE_PARAMS_CMD
#define BLT_ROP_T            BLT_ROP_T_CMD|THREE_PARAMS_CMD
#define SOLID_BLT_COPY       BLT_COPY_CMD|TWO_PARAMS_CMD
#define SOLID_BLT_ROP        BLT_ROP_CMD|TWO_PARAMS_CMD
#define SOLID_BLT_ALPHA      BLT_ALPHA_CMD|TWO_PARAMS_CMD

#define LINE_COPY            LINE_COPY_CMD|TWO_PARAMS_CMD
#define POLYLINE_COPY        LINE_COPY_CMD|ONE_PARAM_CMD
#define LINE_COPY_T          LINE_COPY_T_CMD|TWO_PARAMS_CMD
#define POLYLINE_COPY_T      LINE_COPY_T_CMD|ONE_PARAM_CMD
#define LINE_ROP             LINE_ROP_CMD|TWO_PARAMS_CMD
#define POLYLINE_ROP         LINE_ROP_CMD|ONE_PARAM_CMD
#define LINE_ROP_T           LINE_ROP_T_CMD|TWO_PARAMS_CMD
#define POLYLINE_ROP_T       LINE_ROP_T_CMD|ONE_PARAM_CMD

#define TEXT_COPY            TEXTBLT_COPY_CMD|THREE_PARAMS_CMD
#define TEXT_COPY_T          TEXTBLT_COPY_T_CMD|THREE_PARAMS_CMD
#define TEXT_ROP             TEXTBLT_ROP_CMD|THREE_PARAMS_CMD
#define TEXT_ROP_T           TEXTBLT_ROP_T_CMD|THREE_PARAMS_CMD

#ifndef PCI_GXA

#define BLT_ALPHA            BLT_ALPHA_CMD|THREE_PARAMS_CMD
#define BLT_ALPHA_T          BLT_ALPHA_T_CMD|THREE_PARAMS_CMD

#define LINE_ALPHA           LINE_ALPHA_CMD|TWO_PARAMS_CMD
#define POLYLINE_ALPHA       LINE_ALPHA_CMD|ONE_PARAM_CMD
#define LINE_ALPHA_T         LINE_ALPHA_T_CMD|TWO_PARAMS_CMD
#define POLYLINE_ALPHA_T     LINE_ALPHA_T_CMD|ONE_PARAM_CMD

#define TEXT_ALPHA           TEXTBLT_ALPHA_CMD|THREE_PARAMS_CMD
#define TEXT_ALPHA_T         TEXTBLT_ALPHA_T_CMD|THREE_PARAMS_CMD

#define PALETTE_FETCH        PALETTE_FETCH_CMD|ONE_PARAM_CMD

/***************************************************************************
 Palette fetch macro
***************************************************************************/

#define mLoadPalette(SrcContext,PalAddr) \
mCheckFifo(1) \
mCmd0(GXA_BASE | PALETTE_FETCH | SrcContext,PalAddr)

#endif // PCI_GXA

/***************************************************************************
 Bitblt macros
***************************************************************************/

#define mFillOpaqueRect(SrcContext,DstContext,DestXY,DestExt,Color) \
mCheckFifo(4) \
mLoadReg(BG_COLOR_REG,Color) \
mCmd1(GXA_BASE | SOLID_BLT_COPY | SrcContext | DstContext,DestXY,DestExt)

/***************************************************************************
 QMark macros and defines
***************************************************************************/
#define QMARK_MASK         0x000001FFl
#define QMARK_WAIT4_VERT   0x00000100l

#define mQMark(MarkerVal) \
mCheckFifo(1) \
mCmd0(GXA_BASE | QMARK,(MarkerVal & QMARK_MASK))

#endif //_GXABLT_H_
