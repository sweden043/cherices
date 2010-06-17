/*----------------------------------------------------------------------------;
; Project: Sabine
;   Title: Graphics Coprocessor Include File
;    File: gcpdefs.h
;
; $Header: gcpdefs.h, 2, 2/5/03 12:47:54 PM, Miles Bintz$
; $Log: 
;  2    mpeg      1.1         2/5/03 12:47:54 PM     Miles Bintz     SCR(s) 
;        5227 :
;        added blank line at end of file
;        
;        
;  1    mpeg      1.0         9/30/99 2:47:16 PM     Rob Tilton      
; $
 * 
 *    Rev 1.1   05 Feb 2003 12:47:54   bintzmf
 * SCR(s) 5227 :
 * added blank line at end of file
 * 
 * 
 *    Rev 1.0   30 Sep 1999 13:47:16   rtilton
 * Initial revision.
 * 
 *    Rev 1.0   03 Mar 1998 14:55:24   mustafa
 * Initial revision.
;
; Created: 23Jan98
;  Author: Matthew D. Bates (mbates@brooktree.com)
;
; Modification History:
; 23Jan98 - Initial revision
;----------------------------------------------------------------------------*/


// GCP registers
#define GCP_DST_ADDR        0
#define GCP_SRC_ADDR        1
#define GCP_PAT_ADDR        2
#define GCP_MSK_ADDR        3
#define GCP_FG_COL          4
#define GCP_BG_COL          5
#define GCP_DST_CMP_COL     6
#define GCP_SRC_CMP_COL     7
#define GCP_CONTROL         8
#define GCP_COUNT           9
#define GCP_WORD_COUNT      10
#define GCP_PAT_SIZE        11
#define GCP_PAT_LINE        12
#define GCP_TEXT_HEIGHT     13
#define GCP_TEXT_PITCH      14
#define GCP_TEXT_LINE       15

// control register
#define GCP_1BPP            0x000
#define GCP_2BPP            0x001
#define GCP_4BPP            0x002
#define GCP_8BPP            0x003
#define GCP_16BPP           0x004
#define GCP_32BPP           0x005
#define GCP_24BPP           0x007

#define GCP_EXPAND_SRC      0x010
#define GCP_EXPAND_PAT      0x020
#define GCP_DST_CC_EN       0x040
#define GCP_SRC_CC_EN       0x080

#define GCP_OP_DST          0x100
#define GCP_OP_SRC          0x200
#define GCP_OP_PAT          0x400
#define GCP_OP_MSK          0x800
#define GCP_FILL            0x000
#define GCP_ROP1            0x100
#define GCP_COPY            0x200
#define GCP_ROP2            0x300
#define GCP_PAT_FILL        0x400
#define GCP_PAT_ROP         0x500
#define GCP_PAT_SRC         0x600
#define GCP_ROP3            0x700
#define GCP_MSK_COPY        0xB00
#define GCP_ROP4            0xF00

#define GCP_TEXT_32         0x1000
#define GCP_TEXT_64         0x2000
#define GCP_TEXT_128        0x3000

// rop codes
#define GCP_SRCCOPY         0xCCCC0000
#define GCP_SRCPAINT        0xEEEE0000
#define GCP_SRCAND          0x88880000
#define GCP_SRCINVERT       0x66660000
#define GCP_SRCERASE        0x44440000
#define GCP_NOTSRCCOPY      0x33330000
#define GCP_NOTSRCERASE     0x11110000
#define GCP_MERGECOPY       0xC0C00000
#define GCP_MERGEPAINT      0xBBBB0000
#define GCP_PATCOPY         0xF0F00000
#define GCP_PATPAINT        0xFBFB0000
#define GCP_PATINVERT       0x5A5A0000
#define GCP_DSTINVERT       0x55550000
#define GCP_BLACKNESS       0x00000000
#define GCP_WHITENESS       0xFFFF0000
#define GCP_TRANSPARENT     0xCCAA0000


