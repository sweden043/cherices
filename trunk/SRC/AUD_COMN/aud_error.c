/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2002                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       aud_error.c
 *
 *
 * Description:    Source file for MPEG Audio Error Handling
 *
 *
 * Author:         Senthil Veluswamy
 *
 ****************************************************************************/
/* $Header: aud_error.c, 6, 2/13/03 11:13:20 AM, Matt Korte$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "aud_error.h"
#include "video.h"
#include "globals.h"
#include "trace.h"

/***********/
/* Aliases */
/***********/
#define NUM_MPG_AUDIO_ERRORS 4

/***************/
/* Global Data */
/***************/
bool gbMPGAudioErrorCbRegistered = FALSE;
/* Callback Table */
pfnMPGAudioErrorCallback MPG_Aud_ErrCb_Table[NUM_MPG_AUDIO_ERRORS];
u_int32 gbNumMPGAudioErrCbs;

/************************/
/* Function definitions */
/************************/

/*********************************************************************/
/*  register_mpg_audio_error_callback()                              */
/*                                                                   */
/*  PARAMETERS:  pfnAudioErrorCb - Callback for a particular MPEG    */
/*                Audio Error                                        */
/*               uDSRRegValue - Value of the MPEG DSR Register       */
/*                                                                   */
/*  DESCRIPTION: Register an entry for the specified audio error in  */
/*                the callback table. If an entry already exists,    */
/*                overwrite the previous callback. Callbacl Chaining */
/*                is not supported.                                  */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
void register_mpg_audio_error_callback(pfnMPGAudioErrorCallback pfnAudioErrorCb,
                                       u_int32 uDSRRegValue)
{
   bool cs;
   u_int8 mpg_audio_error_num;

   cs = critical_section_begin();

   /* Map DSR Audio Error Value to Table offset */
   mpg_audio_error_num = (uDSRRegValue & MPG_DECODE_STATUS_AUDIOERROR_MASK)
                           >> MPG_DECODE_STATUS_AUDIOERROR_SHIFT;

   /* Populate Error callback */
   MPG_Aud_ErrCb_Table[mpg_audio_error_num] = pfnAudioErrorCb;

   if(!gbMPGAudioErrorCbRegistered)
   {
      /* Register the General Audio Error Callback with the Video Driver */
      gen_video_register_audio_error_callback(gen_mpg_audio_error_callback);
      /* Update State */
      gbMPGAudioErrorCbRegistered = TRUE;
   }

   critical_section_end(cs);

   return;
}

/*********************************************************************/
/*  gen_mpg_audio_error_callback()                                   */
/*                                                                   */
/*  PARAMETERS:  uDSRRegValue - Value of the MPEG DSR Register       */
/*                                                                   */
/*  DESCRIPTION: Call the callback if one is registered for this     */
/*                particular Audio error.                            */
/*                                                                   */
/*  RETURNS:     Nothing.                                            */
/*********************************************************************/
void gen_mpg_audio_error_callback(u_int32 uDSRRegValue)
{
   u_int8 mpg_audio_error_num;
#if SOFTWARE_AC3_SPDIF_FIX == YES && AUDIO_MICROCODE == UCODE_COLORADO
   bool bSPDIFState = FALSE;
#endif /* SOFTWARE_AC3_SPDIF_FIX */

   /* Map DSR Audio Error Value to Table offset */
   mpg_audio_error_num = (uDSRRegValue & MPG_DECODE_STATUS_AUDIOERROR_MASK)
                           >> MPG_DECODE_STATUS_AUDIOERROR_SHIFT;

   /* If a callback is registered */
   if(MPG_Aud_ErrCb_Table[mpg_audio_error_num])
   {
      (MPG_Aud_ErrCb_Table[mpg_audio_error_num])(uDSRRegValue);
   }
#if SOFTWARE_AC3_SPDIF_FIX == YES && AUDIO_MICROCODE == UCODE_COLORADO
   else
   {
      bSPDIFState = CNXT_GET_VAL(glpCtrl1, MPG_CONTROL1_SPDIFISAC3_MASK);
      if(bSPDIFState)
      {
         isr_trace_new(TRACE_AUD | TRACE_LEVEL_ALWAYS, 
            "MPGErrorCb: Unhandled audio err=%d\n", mpg_audio_error_num, 0);
      }
   }
#endif /* SOFTWARE_AC3_SPDIF_FIX */

   /* Update count */
   gbNumMPGAudioErrCbs++;

   return;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  6    mpeg      1.5         2/13/03 11:13:20 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  5    mpeg      1.4         11/15/02 12:41:10 PM   Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Wrapped Software AC3 fix for Colorado Builds
 *        
 *  4    mpeg      1.3         11/14/02 5:31:36 PM    Senthil Veluswamy SCR(s) 
 *        4935 :
 *        Wrapped error trace to print only if AC3 is being passed through.
 *        
 *  3    mpeg      1.2         11/14/02 2:36:22 PM    Dave Aerne      SCR(s) 
 *        4959 :
 *        conditionally excluded unsupported audio error interrupt messages 
 *        depending upon SOFTWARE_AC3_SPDIF_FIX.
 *        
 *  2    mpeg      1.1         10/21/02 4:10:52 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        Added error messages for unregistered audio errors
 *        
 *  1    mpeg      1.0         10/11/02 6:19:08 PM    Senthil Veluswamy 
 * $
 * 
 *    Rev 1.5   13 Feb 2003 11:13:20   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   15 Nov 2002 12:41:10   velusws
 * SCR(s) 4935 :
 * Wrapped Software AC3 fix for Colorado Builds
 * 
 *    Rev 1.3   14 Nov 2002 17:31:36   velusws
 * SCR(s) 4935 :
 * Wrapped error trace to print only if AC3 is being passed through.
 * 
 *    Rev 1.2   14 Nov 2002 14:36:22   aernedj
 * SCR(s) 4959 :
 * conditionally excluded unsupported audio error interrupt messages depending upon SOFTWARE_AC3_SPDIF_FIX.
 * 
 *    Rev 1.1   21 Oct 2002 15:10:52   velusws
 * SCR(s) 4790 :
 * Added error messages for unregistered audio errors
 * 
 *    Rev 1.0   11 Oct 2002 17:19:08   velusws
 * SCR(s) 4790 :
 * Source file for MPEG Audio Error Handling
 *
 ****************************************************************************/

