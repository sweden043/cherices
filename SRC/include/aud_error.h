/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2002                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       aud_error.h
 *
 *
 * Description:    MPEG Audio Error Handling Header file
 *
 *
 * Author:         Senthil Veluswamy
 *
 ****************************************************************************/
/* $Header: aud_error.h, 1, 10/11/02 6:19:56 PM, $
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"

/***********/
/* Aliases */
/***********/
typedef void (*pfnMPGAudioErrorCallback)(u_int32 uDSRRegValue);

/***********************/
/* Function prototypes */
/***********************/
void register_mpg_audio_error_callback(pfnMPGAudioErrorCallback pfnAudioErrorCb,
                                       u_int32 uDSRRegValue);
void gen_mpg_audio_error_callback(u_int32 uDSRRegValue);

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         10/11/02 6:19:56 PM    Senthil Veluswamy 
 * $
 * 
 *    Rev 1.0   11 Oct 2002 17:19:56   velusws
 * SCR(s) 4790 :
 * MPEG Audio Error Handling Header file
 *
 ****************************************************************************/
