/***************************************************************************
                                                                          
   Filename: GENMAC.H

   Description: Colorado Graphics Accelerator (GXA)
                General Definition and Macro Utilities
                Common shared macros and defines that
                are used by other components

   Created: 9/22/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: genmac.h, 2, 11/17/00 2:55:58 PM, Lucy C Allevato$
$Log: 
 2    mpeg      1.1         11/17/00 2:55:58 PM    Lucy C Allevato Fixed 
       missing volatile modifier in macros by using LPREG type.
       
 1    mpeg      1.0         8/23/00 12:18:10 PM    Lucy C Allevato 
$
 * 
 *    Rev 1.1   17 Nov 2000 14:55:58   eching
 * Fixed missing volatile modifier in macros by using LPREG type.
 * 
 *    Rev 1.0   23 Aug 2000 11:18:10   eching
 * Initial revision.
 * 
 *    Rev 1.1   31 Mar 2000 16:51:10   eching
 * Updates for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.0   06 Mar 2000 14:26:50   eching
 * Initial revision.
****************************************************************************/

#ifndef _GENMAC_H_
#define _GENMAC_H_

/****************************************************************************
 * mWaitForIdle
 ***************************************************************************/
#define mWaitForIdle \
{ \
   LPREG GxaFifoDepth = (LPREG)(GXA_NON_QUEUED|GXA_DEPTH_REG); \
   while( (*GxaFifoDepth & 0x7FFFFF) != 0 ); \
}

/****************************************************************************
 * mWaitForFifoEmpty
 ***************************************************************************/
#ifndef OFF_CHIP_QUEUE
#define mWaitForFifoEmpty \
{ \
   LPREG GxaFifoEmpty = (LPREG)(GXA_NON_QUEUED|GXA_DEPTH_REG); \
   while( (*GxaFifoEmpty & 0x10000) != 0 ); \
}

#else

#define mWaitForFifoEmpty \
{ \
   LPREG GxaFifoEmpty = (LPREG)(GXA_NON_QUEUED|GXA_DEPTH_REG); \
   while( (*GxaFifoEmpty & 0xFFFF) != 0 ); \
}
#endif

/****************************************************************************
 * mCheckFifo
 ***************************************************************************/
#define mCheckFifo(NumDwords) \
{ \
   LPREG GxaFifoDepth = (LPREG)(GXA_NON_QUEUED|GXA_DEPTH_REG); \
   while( (GXA_GQ_ENTRIES-((*GxaFifoDepth & GUI_GQUE_DEPTH)>>25)) < NumDwords ); \
}

/****************************************************************************
 * mLoadReg
 ***************************************************************************/
#define mLoadReg(HwReg, Val) *(LPREG)(HwReg) = (u_int32)(Val);

/****************************************************************************
 * mCmd macros for zero, one, two, and three param cmds
 ***************************************************************************/
#define mCmd0(Cmd, P0) *(LPREG)(Cmd) = (u_int32)P0;

#define mCmd1(Cmd, P0, P1) \
   *(LPREG)(Cmd) = (u_int32)P0; \
   *(LPREG)(Cmd) = (u_int32)P1;

#define mCmd2(Cmd, P0, P1, P2) \
   *(LPREG)(Cmd) = (u_int32)P0; \
   *(LPREG)(Cmd) = (u_int32)P1; \
   *(LPREG)(Cmd) = (u_int32)P2;

#define mCmd3(Cmd, P0, P1, P2, P3) \
   *(LPREG)(Cmd) = (u_int32)P0; \
   *(LPREG)(Cmd) = (u_int32)P1; \
   *(LPREG)(Cmd) = (u_int32)P2; \
   *(LPREG)(Cmd) = (u_int32)P3;

/****************************************************************************
 * mRepColor macros for 8 and 16 bpp formats
 ***************************************************************************/
  
#define mRepColor8(Color, RepColor) \
{ \
unsigned int TempColor; \
   TempColor = ((Color & 0xFF) << 8) | (Color & 0xFF); \
   RepColor = (TempColor << 16) | TempColor; \
}

#define mRepColor16(Color, RepColor) \
{ \
   RepColor = (Color << 16) | Color; \
}

/****************************************************************************
 * mMakeXY
 ***************************************************************************/
#define mMakeXY(X,Y) (((unsigned int)Y << 16) | (unsigned int)X)

/****************************************************************************
 * mMakeRGB565
 ***************************************************************************/
#define mMakeRGB565(R,G,B) \
((unsigned int)(B&0xF8)<<11)|((unsigned int)(G&0xFC)<<5)|(unsigned int)(R&0xF8)

/****************************************************************************
 * mMakeRGB555
 ***************************************************************************/
#define mMakeRGB555(R,G,B) \
((unsigned int)(B&0xF8)<<10)|((unsigned int)(G&0xF8)<<5)|(unsigned int)(R&0xF8)

/****************************************************************************
 * mMakeRGB888
 ***************************************************************************/
#define mMakeRGB888(R,G,B) \
((unsigned int)(B&0xFF)<<16)|((unsigned int)(G&0xFF)<<8)|(unsigned int)(R&0xFF)

#endif //_GENMAC_H_

