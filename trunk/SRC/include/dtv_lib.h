/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c)  2003                          */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       dtv_lib.h
 *
 *
 * Description:    Prototypes for the DirecTV Channel Object Management library
 *
 * Author:         Dave Moore
 *
 ****************************************************************************/
/* $Header: dtv_lib.h, 1, 5/2/03 12:07:24 PM, Dave Moore$
 ****************************************************************************/

bool cnxt_dtv_find_next_program( int *pNextProgramNumber );
bool cnxt_dtv_find_prev_program( int *pPrevProgramNumber );
bool cnxt_dtv_set_program_number( int ProgramNumber );
bool cnxt_dtv_get_next_or_prev_audio( bool bNext, u_int16* puCurrentAudioSCID );
bool cnxt_dtv_get_next_or_prev_video(bool bNext, u_int16* puCurrentVideoSCID, u_int16* puCurrentAudioSCID, 
                                     u_int16* puCurrentPCRSCID, u_int16* puCurrentDAVISSCID, u_int32* puCurrentNetwork );
bool cnxt_dtv_get_next_or_prev_data(bool bNext, u_int16* puCurrentVideoSCID, u_int16* puCurrentAudioSCID,
                                    u_int16* puCurrentPCRSCID, u_int16* puCurrentDAVISSCID, u_int32* puCurrentNetwork);
u_int32 cnxt_dtv_get_num_channel_entries( void );
void cnxt_dtv_select_first_program( void );
void cnxt_dtv_get_current_program_co_entry( DTV_APG_CHANNEL_OBJECT *pCurrentEntry, u_int32 *puCurrentVideo, u_int32 *puCurrentAudio, u_int32 *puCurrentDAVIS, u_int32 *puCurrentFreqIndex );
u_int32 cnxt_dtv_get_current_program( void );
u_int32 cnxt_dtv_get_program_number( u_int32 uProgram );
int32 cnxt_dtv_find_channel_index( DTV_APG_CHANNEL_OBJECT *CurrentEntry );


/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         5/2/03 12:07:24 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   02 May 2003 11:07:24   mooreda
 * SCR(s) 5972 :
 * Prototypes for DirecTV Channel Object Management 
 * 
 *
 ****************************************************************************/

