/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       OTVOSD.H
 *
 *
 * Description:    OpenTV OSD Mode Definition Header File
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: otvosd.h, 2, 10/31/02 1:47:50 PM, Dave Wilson$
 ****************************************************************************/

/*****************************************************************************/
/* Note: This header will be included in OpenTV 1.2 builds but only the OSD  */
/* scratchpad size definition is used in this case. The OpenTV 1.2 driver    */
/* uses hardcoded modes and colour handling. It has already passed BSkyB     */
/* certification so will not be reworked to use this data format (which is   */
/* designed to ease porting to new network's OpenTV Core 1.0 specifications) */
/*****************************************************************************/

#ifndef _OTVOSD_H_
#define _OTVOSD_H_

#include "opentvx.h"

/***********************/
/* OSD Scratchpad Size */
/***********************/

/* The OSD scratchpad is used by OpenTV when performing any OSD update.    */
/* Changes are drawn into the scratchpad then blitted from there to the    */
/* actual OSD image. Increasing the size can make a significant difference */
/* to OSD redraw performance. The size is defined in bytes.                */
#define OSD_SCRATCHPAD_SIZE 32768

#ifndef OPENTV_12

/*********************/
/* Colour Resolution */
/*********************/

/* Although our hardware supports a full 8:8:8 palette, some older IRDs only   */
/* support 4:6:4 or similar colour resolution. To allow this resolution to be  */
/* mimiced, use the following macros to truncate colour components if          */
/* necessary.                                                                  */

/* General OpenTV EN2/Core 1.0 case - no truncation */
#define TRUNCATE_PALETTE_LUMA(x)   (x)
#define TRUNCATE_PALETTE_CHROMA(x) (x)
#define TRUNCATE_PALETTE_RED(x)    (x)
#define TRUNCATE_PALETTE_GREEN(x)  (x)
#define TRUNCATE_PALETTE_BLUE(x)   (x)

/* Comment this line out to allow 8:8:8 palette support with no truncation code */
/* #define ENABLE_PALETTE_TRUNCATION */

/********************/
/* Alpha Resolution */
/********************/

/* Conexant hardware supports 256 alpha levels. Some networks require fewer  */
/* levels. Change this macro to define any special alpha truncation required */
/* in your implementation. Note that alpha definitions are reversed between  */
/* OpenTV and Conexant's display controller.                                 */

#define TRUNCATE_PALETTE_ALPHA(x)  (MAX_TRANSPARENCY-(x))

/* We actually support 256 alpha levels [0-255] but OpenTV only gives us 8 */
/* bits to store the number in so pretend we support 255 levels instead    */
#define ALPHA_LEVELS  255

/* To save typing later, define the alpha models used in all modes here    */
#define ALPHA_MODELS  ((1 << TRANSPARENCY_DEFAULT_MODEL)                   | \
                       (1 << TRANSPARENCY_ONE_SHARED_LEVEL_MODEL)          | \
                       (1 << TRANSPARENCY_ONE_SHARED_LEVEL_AND_ZERO_MODEL) | \
                       (1 << TRANSPARENCY_INDIVIDUAL_LEVEL_MODEL))

/**********************/
/* OSD Mode Structure */
/**********************/

/* We need to tie together the OpenTV OSD driver mode with an underlying */
/* OSDLIBC mode so we create a structure to allow this.                  */
typedef struct _cnxt_otv_osd_mode
{
  display_mode sOpenTVMode;
  u_int32      uNativeMode;
} cnxt_otv_osd_mode;


/* Populate the following array with values describing the modes to be    */
/* offered in your environment. The default set merely offers everything  */
/* that is supported. Note that you should try to mirror the exact modes  */
/* used in IRDs of your target network since we have found many cases     */
/* where o-code applications are less than rigorous in the way they       */
/* query and set modes (many just use a hardcoded mode number!).          */

/* The default set includes 4-, 8-bpp modes in YUV and RGB colour spaces. */
/* 16- and 32-bpp modes, although supported by the low level driver, are  */
/* not offered here since OpenTV does not currently support them.         */

/* Note that the height of the OSD will be updated at runtime depending   */
/* upon the video output mode in use. The default values here are for PAL */  
/* but these will be reduced from 576 to 480 if NTSC is in use.           */

#ifdef INSTANTIATE_OSD_MODES
cnxt_otv_osd_mode gOpenTVOSDModes[] =
{
    /*************************/
    /* 4 bit ARGB palettised */
    /*************************/
    {
     {4,                               /* depth               */
      RGB_MODEL,                       /* color_mode          */
      ALPHA_LEVELS,                    /* transparency_levels */
      ALPHA_MODELS,                    /* transparency_models */
      0x0403,                          /* aspect_ratio        */
      0,                               /* left                */
      0,                               /* top                 */
      OSD_MAX_WIDTH,                   /* right               */
      OSD_MAX_HEIGHT,                  /* bottom              */
      OSD_MODE_INTERLACED |            /* flags               */
      OSD_MODE_PALETTE_CAPABLE |
      OSD_MODE_CURSOR_CAPABLE,
      0                                /* reserved_2          */
     }, 
     OSD_MODE_4ARGB
    },
    /*************************/
    /* 4 bit AYUV palettised */
    /*************************/
    {
     {4,                               /* depth               */
      YUV_MODEL,                       /* color_mode          */
      ALPHA_LEVELS,                    /* transparency_levels */
      ALPHA_MODELS,                    /* transparency_models */
      0x0403,                          /* aspect_ratio        */
      0,                               /* left                */
      0,                               /* top                 */
      OSD_MAX_WIDTH,                   /* right               */
      OSD_MAX_HEIGHT,                  /* bottom              */
      OSD_MODE_INTERLACED |            /* flags               */
      OSD_MODE_PALETTE_CAPABLE |
      OSD_MODE_CURSOR_CAPABLE,
      0                                /* reserved_2          */
     }, 
     OSD_MODE_4AYUV
    },
    /*************************/
    /* 8 bit ARGB palettised */
    /*************************/
    {
     {8,                               /* depth               */
      RGB_MODEL,                       /* color_mode          */
      ALPHA_LEVELS,                    /* transparency_levels */
      ALPHA_MODELS,                    /* transparency_models */
      0x0403,                          /* aspect_ratio        */
      0,                               /* left                */
      0,                               /* top                 */
      OSD_MAX_WIDTH,                   /* right               */
      OSD_MAX_HEIGHT,                  /* bottom              */
      OSD_MODE_INTERLACED |            /* flags               */
      OSD_MODE_PALETTE_CAPABLE |
      OSD_MODE_CURSOR_CAPABLE,
      0                                /* reserved_2          */
     }, 
     OSD_MODE_8ARGB
    },
    /*************************/
    /* 8 bit AYUV palettised */
    /*************************/
    {
     {8,                               /* depth               */
      YUV_MODEL,                       /* color_mode          */
      ALPHA_LEVELS,                    /* transparency_levels */
      ALPHA_MODELS,                    /* transparency_models */
      0x0403,                          /* aspect_ratio        */
      0,                               /* left                */
      0,                               /* top                 */
      OSD_MAX_WIDTH,                   /* right               */
      OSD_MAX_HEIGHT,                  /* bottom              */
      OSD_MODE_INTERLACED |            /* flags               */
      OSD_MODE_PALETTE_CAPABLE |
      OSD_MODE_CURSOR_CAPABLE,
      0                                /* reserved_2          */
     }, 
     OSD_MODE_8AYUV
    }
};

/* How many modes do we have? */
#define NUM_OPENTV_OSD_MODES (sizeof(gOpenTVOSDModes)/sizeof(cnxt_otv_osd_mode))

#else
extern cnxt_otv_osd_mode gOpenTVOSDModes[];
#endif /* INSTANTIATE_OSD_MODES */

#endif /* OPENTV_12 */
#endif /* _OTVOSD_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         10/31/02 1:47:50 PM    Dave Wilson     SCR(s) 
 *        2035 :
 *        Enabled all valid alpha handling models for all modes. Now that the 
 *        OSD
 *        driver supports all the modes, we may as well make them visible to 
 *        the 
 *        world.
 *        
 *  1    mpeg      1.0         10/30/02 4:39:54 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.1   31 Oct 2002 13:47:50   dawilson
 * SCR(s) 2035 :
 * Enabled all valid alpha handling models for all modes. Now that the OSD
 * driver supports all the modes, we may as well make them visible to the 
 * world.
 * 
 *    Rev 1.0   30 Oct 2002 16:39:54   dawilson
 * SCR(s) 4823 :
 * New header file which contains definitions of all the platform/network
 * dependent OpenTV OSD modes and capabilities.
 *
 ****************************************************************************/

