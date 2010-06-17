/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        tvenc_priv.h
 *
 *
 * Description:     Private header file for TV encoder driver
 *
 *
 * Author:          Xin Golden (based on the current tvenc_priv.c in pvcs\osdlibc)
 *
 ****************************************************************************/
/* $Header: tvenc_priv.h, 5, 10/24/03 11:38:08 AM, Xin Golden$
 ****************************************************************************/

#ifndef _TVENC_PRIV_H_
#define _TVENC_PRIV_H_

/********************************/
/* Symbol and Macro definitions */
/********************************/
#define CNXT_TVENC_MAX_MODULE   5

#define CNXT_TVENC_NUM_UNITS  1  /* change this as needed */

#define CNXT_TVENC_MAX_HANDLES   5 /* !!!! change this as needed */

#define TRACE_TVENC (TRACE_OSD|TRACE_LEVEL_2)

#ifndef abs
#define abs(x) (((x) < 0) ? (-x):(x))
#endif

#define TVENC_PHASE_SHIFT  2

#define INITIAL_Y_OFFSET     0
#define INITIAL_X_OFFSET     0

#define DEFAULT_MULT_UU      0x7F
#define DEFAULT_MULT_VU      0
#define DEFAULT_MULT_UV      0
#define DEFAULT_MULT_VV      0x7F
                      
/* registers for BT860/861 */
#define MODULE_ID_REG               0x00 

#define BT861_HBLANK_REG_1          0x0B
#define BT861_HBLANK_REG_2          0x0C
#define BT861_HSYNC_OFF_REG_1       0x10
#define BT861_HSYNC_OFF_REG_2       0x11

#define BT861_HUE_REG               0x3B
#define BT861_CONTRAST_REG          0x22
#define BT861_SATURATION_CR_REG     0x20
#define BT861_SATURATION_CB_REG     0x21
#define BT861_BRIGHTNESS_REG        0x37
#define BT861_SHARPNESS_FIL_REG     0x3C
#define BT861_SHARPNESS_PKFIL_REG   0x1B
#define BT861_FIL_SEL_MASK          0x04
#define BT861_PKFIL_SEL_MASK        0x18
#define BT861_MULT_UU_REG           0x5C
#define BT861_MULT_UV_REG           0x5E
#define BT861_MULT_VU_REG           0x5D
#define BT861_MULT_VV_REG           0x5F

#define BT861_DAC_ENABLE_REG        0x18
#define BT861_CHROMA_BW_REG         0x17
#define BT861_M_COMP_D_REG          0x23
#define BT861_M_COMP_F_REG          0x24
#define BT861_M_COMP_E_REG          0x25

#define BT861_TTXHS_REG_1           0x4D
#define BT861_TTXHS_REG_2           0x4E
#define BT861_TTXHE_REG_1           0x4F
#define BT861_TTXHE_REG_2           0x50     

#define BT861_TTXBF1_REG_1          0x51
#define BT861_TTXBF1_REG_2          0x52
#define BT861_TTXEF1_REG_1          0x53
#define BT861_TTXEF1_REG_2          0x54
#define BT861_TTXBF2_REG_1          0x55
#define BT861_TTXBF2_REG_2          0x56
#define BT861_TTXEF2_REG_1          0x57
#define BT861_TTXEF2_REG_2          0x58
#define BT861_TTXDIS_REG_1          0x5A
#define BT861_TTXDIS_REG_2          0x5B
#define BT861_TTXE_REG              0x59

#define BT861_CC_TYPE_REG           0x48
#define BT861_CC_LINE_REG           0x49
#define BT861_CC_DATA_REG_1         0x42
#define BT861_CC_DATA_REG_2         0x43
#define BT861_XDS_DATA_REG_1        0x40
#define BT861_XDS_DATA_REG_2        0x41

#define BT861_WSS_REG_1             0x4A
#define BT861_WSS_REG_2             0x4B
#define BT861_WSS_REG_3             0x4C

#define BT861_WSS_ENABLE_MASK       0x40

#define BT861_CGMS_ENABLE_MASK      0xC0

#define DEFAULT_BT861_TTX_ENABLE    0x03       /* ttx is enabled and TXRM is 1 */
#define DEFAULT_BT861_TTX_DISABLE   0x02       /* ttx is disabled and TXRM is 1 */

#define DEFAULT_BT861_CC_TYPE       0x07       /* ECC, EXDS, ECCGATE are all 1 */
#define DEFAULT_BT861_CC_LINE       0x44

#define BT861_PAL_HBLANK            0x108    
#define BT861_NTSC_HBLANK           0x108    

#ifdef BT861_PAL_VBLANK
#undef BT861_PAL_VBLANK
#endif
#define BT861_PAL_VBLANK            0x16     
#define BT861_NTSC_VBLANK           0x13     
#define BT861_PAL_HSYNC             0
#define BT861_NTSC_HSYNC            0

  
/* get n bits from position p at val, assuming bit position 0 at the right end */     
#define getbits(val,pos,nbits)  ( ( ((val) >> ((pos)+1-(nbits))) & ~(~0 << (nbits)) ) << ((pos)+1-(nbits)) ) 
 
#define LMO(y)                   ( ((y) & 0x80000000) ? 31 : \
                                   ( ((y) & 0x40000000) ? 30 : \
                                     ( ((y) & 0x20000000) ? 29 : \
                                       ( ((y) & 0x10000000) ? 28 : \
                                         ( ((y) & 0x08000000) ? 27 : \
                                           ( ((y) & 0x04000000) ? 26 : \
                                             ( ((y) & 0x02000000) ? 25 : \
                                               ( ((y) & 0x01000000) ? 24 : \
                                                 ( ((y) & 0x00800000) ? 23 : \
                                                   ( ((y) & 0x00400000) ? 22 : \
                                                     ( ((y) & 0x00200000) ? 21 : \
                                                       ( ((y) & 0x00100000) ? 20 : \
                                                         ( ((y) & 0x00080000) ? 19 : \
                                                           ( ((y) & 0x00040000) ? 18 : \
                                                             ( ((y) & 0x00020000) ? 17 : \
                                                               ( ((y) & 0x00010000) ? 16 : \
                                                                 ( ((y) & 0x00008000) ? 15 : \
                                                                   ( ((y) & 0x00004000) ? 14 : \
                                                                     ( ((y) & 0x00002000) ? 13 : \
                                                                       ( ((y) & 0x00001000) ? 12 : \
                                                                         ( ((y) & 0x00000800) ? 11 : \
                                                                           ( ((y) & 0x00000400) ? 10 : \
                                                                             ( ((y) & 0x00000200) ?  9 : \
                                                                               ( ((y) & 0x00000100) ?  8 : \
                                                                                 ( ((y) & 0x00000080) ?  7 : \
                                                                                   ( ((y) & 0x00000040) ?  6 : \
                                                                                     ( ((y) & 0x00000020) ?  5 : \
                                                                                       ( ((y) & 0x00000010) ?  4 : \
                                                                                         ( ((y) & 0x00000008) ?  3 : \
                                                                                           ( ((y) & 0x00000004) ?  2 : \
                                                                                             ( ((y) & 0x00000002) ?  1 : \
                                                                                               ( ((y) & 0x00000001) ?  0 : 0 ))))))))))))))))))))))))))))))))

 
 
                                                                    
#define CRC_WIDTH   6
#define CRC_POLY    0x30
#define CRC_INIT    0x3F   
#define CGMS_DATALEN    14
#define BITMASK(X) (1 << (X))

/*****************/
/* Data Types    */
/*****************/

typedef struct _cnxt_tvenc_inst
{
   CNXT_HANDLE_PREFACE      Preface;   
   CNXT_TVENC_PFNNOTIFY     pNotifyFn; /* Function to handle Inst notifications */
   void                    *pUserData; /* Appl data needed by Inst */
   u_int32                  uUnitNumber;
} CNXT_TVENC_INST;

typedef struct
{
   CNXT_TVENC_CAPS              *pCaps;
   bool                         bExclusive;
   CNXT_TVENC_INST              *pFirstInst;  /* Pointer to list of open Inst */
   sem_id_t                     UnitSem;

   CNXT_TVENC_MODULE            module;      /* the TV encoder type */
    
   CNXT_TVENC_VIDEO_STANDARD    VideoStandard; 
   u_int8                       uOutputConnection ;

   u_int8                       uExtTVENCIICAddr;    /* IIC address of Bt860/861 or Bt864A/865A */
   u_int8                       uExtTVENCBusAddr;    /* Bus address of Bt860/861 or Bt864A/865A */

   int32                        iHSyncInitial;  
   u_int32                      uHBlankInitial;
   u_int32                      uVBlankInitial;
   int32                        iHSync;
   u_int32                      uHBlank;
   u_int32                      uVBlank;

   int8                         XOffset;        /* X screen start position offset */
   int8                         YOffset;        /* Y screen start position offset */

   int32                        picctrl_brightness;
   int32                        picctrl_contrast;
   int32                        picctrl_saturation;
   int32                        picctrl_hue;
   int32                        picctrl_sharpness;

   u_int32                      uTTXBF1;    /* start teletext line for field 1 */
   u_int32                      uTTXEF1;    /* end teletext line for field 1 */
   u_int32                      uTTXBF2;    /* start teletext line for field 2 */
   u_int32                      uTTXEF2;    /* end teletext line for field 2 */

   bool                         bCCEnabled; /* indicate if closed captioning has been enabled or not */

   CNXT_TVENC_WSS_SETTINGS      WSS_Settings;  /* wide screen signaling configuration set by users */ 

   CNXT_TVENC_CGMS_SETTINGS     CGMS_Settings;  /* CGMS configuration set by users */ 
} CNXT_TVENC_UNIT_INST;

typedef struct
{
   CNXT_TVENC_INST       *pInstList;     /* Memory pool to get inst */
   bool                   bInit;
   sem_id_t               DriverSem;
   CNXT_TVENC_UNIT_INST  *pUnitInst;

   /* Add all other driver specific 'global' data should be declared here */

} CNXT_TVENC_DRIVER_INST;

typedef struct
{
   u_int32 uReg;
   u_int32 uMask;
   u_int32 uValue;
} TVENC_REG_T;


/* private functions */
CNXT_TVENC_STATUS get_unitinst( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_UNIT_INST *pUnitInst );
u_int8 cal_crc_8(u_int8 crc_value, u_int16 data);
u_int32 reflect (u_int32 uBase, u_int8 uBits);
#endif   /* _TVENC_PRIV_H_ */

/* Note: make sure you restart the following LOG section!!! */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         10/24/03 11:38:08 AM   Xin Golden      CR(s): 
 *        7463 Please ignore the last log.  Add CGMS support in tvenc driver.
 *  4    mpeg      1.3         10/24/03 11:31:10 AM   Xin Golden      CR(s): 
 *        7463 Run test# 10 in the TVENC test case.  To execute every test 
 *        cases in test# 10, we need to use a video analyser or a DVD player 
 *        supporting CGMS.  For now I used the video analyser to test it, and 
 *        it seems fine.
 *  3    mpeg      1.2         9/17/03 4:00:04 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the TV encoder driver to use the new handle lib.
 *        
 *  2    mpeg      1.1         8/26/03 7:15:24 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        remove definitions from handle.h
 *        
 *  1    mpeg      1.0         7/30/03 4:00:02 PM     Lucy C Allevato 
 * $
 ****************************************************************************/

