/*****************************************************************************/
/* File: vidlibn.h                                                           */
/*                                                                           */
/* Module: Live video resource library.                                      */
/*                                                                           */
/* Description:  API for programming the live video path of Sabine's DRM.    */
/*****************************************************************************/
/*****************************************************************************
$Header: vidlibn.h, 2, 2/1/00 5:30:34 PM, Dave Wilson$
$Log: 
 2    mpeg      1.1         2/1/00 5:30:34 PM      Dave Wilson     Changes for 
       OpenTV 1.2 support
       
 1    mpeg      1.0         1/5/00 10:20:20 AM     Rob Tilton      
$
 * 
 *    Rev 1.1   01 Feb 2000 17:30:34   dawilson
 * Changes for OpenTV 1.2 support
 * 
 *    Rev 1.0   05 Jan 2000 10:20:20   rtilton
 * Initial revision.
 * 
*****************************************************************************/
#ifndef _VIDLIBN_H_
#define _VIDLIBN_H_

#include "osdlib.h" // OSDRECT definition.

#ifndef OPENTV
#ifndef direct_memory_alloc
#define direct_memory_alloc malloc
#endif /* direct_memory_alloc */
#ifndef direct_memory_free
#define direct_memory_free  free
#endif /* direct_memory_free */
#endif /* OPENTV */

#if (CHIP_NAME != SABINE)
//#define USE_TRIPLE_BUFFER
#endif /* (CHIP_NAME != SABINE) */

#define CHECK_MAX_LOOPS                100

#define PAL_SOURCE_PIXELS              922 /* 1135 */
#define PAL_DELAY_PIXELS               186
#define TOTAL_H_PAL_PIXELS             864
#define TOTAL_V_PAL_LINES              625
#define MAX_HACTIVE_PAL_PIXELS         720
#define MAX_VACTIVE_PAL_LINES          576

#define NTSC_SOURCE_PIXELS             754 /* 910 */
#define NTSC_DELAY_PIXELS              135
#define TOTAL_H_NTSC_PIXELS            858
#define TOTAL_V_NTSC_LINES             525
#define MAX_HACTIVE_NTSC_PIXELS        720
#define MAX_VACTIVE_NTSC_PIXELS        480

#define HSCALE(MaxH, active) (((MaxH << 12) / active) - 4096)
#define VSCALE(src, dst) ((0x10000 - (((src << 9) / dst) - 512)) & 0x1FFF)

typedef enum {
   TUNER                               = 0,
   COMPOSITE                           = 1,
   SVIDEO                              = 2,
   DATA                                = 3
} INPUTSELECT;

typedef enum {
   LVOPTION_BRIGHTNESS                 = 1,
   LVOPTION_CONTRAST                   = 2,
   LVOPTION_SATURATION                 = 3,
   LVOPTION_HUE                        = 4,
   LVOPTION_LV_BUF_PTR                 = 5,
   LVOPTION_BUF_STRIDE                 = 6
} LVOPTIONS;

// Bt835 defines
#define BT835_I2C_ADDRESS              0x88
#define BT835_STATUS_REG               0x00
#define BT835_STATUS_PALLINES          0x10
#define BT835_INPUT_REG                0x01
#define BT835_VDELAY_LO_REG            0x02
#define BT835_VDELAY_HI_REG            0x03
#define BT835_VACTIVE_LO_REG           0x04
#define BT835_VACTIVE_HI_REG           0x05
#define BT835_HDELAY_LO_REG            0x06
#define BT835_HDELAY_HI_REG            0x07
#define BT835_HACTIVE_LO_REG           0x08
#define BT835_HACTIVE_HI_REG           0x09
#define BT835_HSCALE_LO_REG            0x0A
#define BT835_HSCALE_HI_REG            0x0B
#define BT835_VSCALE_LO_REG            0x0C
#define BT835_VSCALE_HI_REG            0x0D
#define BT835_TDEC_REG                 0x0F
#define BT835_BRIGHT_REG               0x10
#define BT835_CONTRAST_REG             0x11
#define BT835_SAT_U_REG                0x12
#define BT835_SAT_V_REG                0x13
#define BT835_HUE_REG                  0x14
#define BT835_CONTROL0_REG             0x15
#define BT835_CONTROL1_REG             0x16
#define BT835_CONTROL2_REG             0x17
#define BT835_CONTROL3_REG             0x18
#define BT835_AGC_DELAY_REG            0x1A
#define BT835_BG_DELAY_REG             0x1B
#define BT835_ADC_REG                  0x1C
#define BT835_TG_CTL_REG               0x24
#define BT835_SWRESET_REG              0xFF

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
// Exported live video function prototypes
void VIDLibInit(void);
bool CreateLiveVideoRgn(POSDRECT lpRect, bool bEnable);
bool DestroyLiveVideoRgn(void);
bool LiveVideoEnable(bool bEnable);
bool SetLVOptions(LVOPTIONS option, u_int32 dwVal);
u_int32 GetLVOptions(LVOPTIONS option);
bool SetLVPos(POSDRECT lpVideo);
bool GetLVPos(POSDRECT lpVideo);
bool SetLVConnection(INPUTSELECT inType);
INPUTSELECT GetLVConnection(void);
bool IsLiveVideoRgn(void);
bool SetLVClippedPos(POSDRECT lpClip, POSDRECT lpDest);
bool GetLVClippedPos(POSDRECT lpClip, POSDRECT lpDest);

#ifdef OPENTV_12
/* Functions and types from Colorado version used during OpenTV 1.2 bringup */
typedef u_int32 HVIDRGN;
typedef enum {
   TYPE_422,   // Live video data type.
   TYPE_420    // Mpg video data type.
} VIDRGNTYPE;

HVIDRGN vidAllocateVideoRgn(u_int32 dwSize, VIDRGNTYPE sType);
bool    vidDestroyVideoRgn(HVIDRGN hRgn);
bool    vidDestroyAllVideoRgn(void);
u_int32 vidQueryVideoRgnSize(u_int32 dwWidth, u_int32 dwHeight, VIDRGNTYPE sType);
bool vidSetVideoRgnDimensions(HVIDRGN hRgn, u_int32 dwWidth, u_int32 dwHeight);
#endif /* OPENTV_12 */

////////////////////////////////////////////////////////////////////////////////
// Internal function prototypes
int VIDIsrInit(u_int32 dwIsrId, PFNISR pfnHandler, PFNISR *pfnChain, bool bEnable);
bool UpdateLiveVideoSize(bool bDecodeUpdate);
int VIDIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
void DecoderUpdate(void *pParam);
void VIDDecoderInit(void);
bool CanAccessEvidRegs(void);

#endif /* _VIDLIBN_H_ */

