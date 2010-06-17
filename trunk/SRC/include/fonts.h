/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       fonts.h
 *
 *
 * Description:    Public Header For Bitmap Font Library
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: fonts.h, 4, 10/10/02 11:48:22 AM, Dave Wilson$
 ****************************************************************************/

#ifndef _FONTS_H_
#define _FONTS_H_

/* Freeware fonts */
extern GFX_FONT blue_highway_15pt;
extern GFX_FONT blue_highway_15pt_bold;
extern GFX_FONT blue_highway_15pt_full;
extern GFX_FONT blue_highway_15pt_bold_full;
extern GFX_FONT blue_highway_20pt;
extern GFX_FONT blue_highway_20pt_bold;
extern GFX_FONT blue_highway_20pt_full;
extern GFX_FONT blue_highway_20pt_bold_full;
extern GFX_FONT blue_highway_28pt_bold;
extern GFX_FONT blue_highway_28pt_bold_full;
extern GFX_FONT zekton_10pt_bold;
extern GFX_FONT zekton_10pt_bold_full;
extern GFX_FONT zekton_15pt_bold;
extern GFX_FONT zekton_15pt_bold_full;
extern GFX_FONT zekton_20pt_bold;
extern GFX_FONT zekton_20pt_bold_full;
extern GFX_FONT baramond_20pt;
extern GFX_FONT baramond_20pt_bold;
extern GFX_FONT baramond_20pt_bold_italic;
extern GFX_FONT baramond_15pt_bold_italic;
extern GFX_FONT baramond_15pt_bold;
extern GFX_FONT baramond_15pt_bold_italic_full;
extern GFX_FONT baramond_15pt_bold_full;
extern GFX_FONT sui_generis_10pt;
extern GFX_FONT sui_generis_10pt_full;
extern GFX_FONT sui_generis_15pt;
extern GFX_FONT sui_generis_15pt_full;
extern GFX_FONT sui_generis_15pt_italic;
extern GFX_FONT sui_generis_15pt_italic_full;
extern GFX_FONT sui_generis_20pt_italic;
extern GFX_FONT sui_generis_20pt;

#ifdef DRIVER_INCL_FONTS2
extern GFX_FONT junegull_14pt;
extern GFX_FONT junegull_30pt_caps;
extern GFX_FONT vibrocentric_24pt_bold;
extern GFX_FONT vibrocentric_24pt;
extern GFX_FONT neuropol_20pt;
extern GFX_FONT neuropol_20pt_full;
extern GFX_FONT neuropol_15pt;
extern GFX_FONT neuropol_15pt_full;
extern GFX_FONT neuropol_10pt_bold;
extern GFX_FONT neuropol_10pt_bold_full;
#endif

#ifdef DRIVER_INCL_AAFONTS
extern GFX_FONT blue_highway_15pt_grey;
extern GFX_FONT blue_highway_24pt_grey;
extern GFX_FONT baramond_15pt_bold_italic_grey;
#endif

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         10/10/02 11:48:22 AM   Dave Wilson     SCR(s) 
 *        4772 :
 *        Added extern definitions for new anti-aliased fonts.
 *        
 *  3    mpeg      1.2         8/19/02 6:02:40 PM     Dave Wilson     SCR(s) 
 *        4432 :
 *        Added a couple more 224 character versions of existing fonts.
 *        Moved the weirder fonts to the FONTS2 component and fixed things up 
 *        in the 
 *        header so that their prototypes are only included if FONTS2 is part 
 *        of the
 *        build.
 *        
 *  2    mpeg      1.1         8/5/02 4:09:00 PM      Dave Wilson     SCR(s) 
 *        4333 :
 *        Added a few new fonts (new sizes or full versions of existing 
 *        typefaces)
 *        
 *  1    mpeg      1.0         7/31/02 5:43:40 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.3   10 Oct 2002 10:48:22   dawilson
 * SCR(s) 4772 :
 * Added extern definitions for new anti-aliased fonts.
 * 
 *    Rev 1.2   19 Aug 2002 17:02:40   dawilson
 * SCR(s) 4432 :
 * Added a couple more 224 character versions of existing fonts.
 * Moved the weirder fonts to the FONTS2 component and fixed things up in the 
 * header so that their prototypes are only included if FONTS2 is part of the
 * build.
 * 
 *    Rev 1.1   05 Aug 2002 15:09:00   dawilson
 * SCR(s) 4333 :
 * Added a few new fonts (new sizes or full versions of existing typefaces)
 * 
 *    Rev 1.0   31 Jul 2002 16:43:40   dawilson
 * SCR(s) 3044 :
 * Public header containing extern definitions for all the fonts held in the 
 * FONTS library.
 *
 ****************************************************************************/
