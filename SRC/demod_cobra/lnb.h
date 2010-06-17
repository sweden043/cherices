#ifndef __LNB
#define __LNB

/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                    Conexant Systems Inc. (c) 2002-2004                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: lnb.h
 *
 * Description: This file contains the interface specifications used by the
 *              LNB portion of the Cobra satellite demod driver module.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/*
 $Id: lnb.h,v 1.2, 2004-03-22 19:10:23Z, Billy Jackman$
 *
 ****************************************************************************/

DEMOD_STATUS cnxt_lnb_set_parameters( LNB_SETTINGS *lnb );
DEMOD_STATUS cnxt_lnb_get_parameters( LNB_SETTINGS *lnb );
DEMOD_STATUS cnxt_set_lnb( NIM *pNIM, TUNING_SPEC *tuning, u_int32 *freq );
DEMOD_STATUS cnxt_lnb_init( NIM *pNIM );
DEMOD_STATUS cnxt_lnb_set_output_enable( NIM *pNIM, bool enable );
DEMOD_STATUS cnxt_lnb_get_output_enable( NIM *pNIM, bool *enable );
DEMOD_STATUS cnxt_lnb_set_polarization( NIM *pNIM,
        NIM_SATELLITE_POLARISATION polarization );
DEMOD_STATUS cnxt_lnb_get_polarization( NIM *pNIM,
        NIM_SATELLITE_POLARISATION *polarization );
DEMOD_STATUS cnxt_lnb_set_tone_enable( NIM *pNIM, bool enable );
DEMOD_STATUS cnxt_lnb_get_tone_enable( NIM *pNIM, bool *enable );

 /****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  3    mpeg      1.2         3/22/04 1:10:23 PM     Billy Jackman   CR(s) 
 *        8585 : Modified keywords to new StarTeam format.
 *        Changed cnxt_set_lnb function to not pass back indications for how to
 *         set polarization voltage and tone but to set them directly.
 *        Added cnxt_lnb_init function for Cobra driver to call.
 *        Added APIs cnxt_lnb_set_output_enable, cnxt_lnb_get_output_enable, 
 *        cnxt_lnb_set_polarization, cnxt_lnb_get_polarization, 
 *        cnxt_lnb_set_tone_enable and cnxt_lnb_get_tone_enable to support 
 *        NDSCORE requirements.
 *        Added LNB of type LNB_MANUAL to allow NDSCORE manual control of LNB 
 *        signalling.
 *        
 *  2    mpeg      1.1         11/27/02 1:25:38 PM    Billy Jackman   SCR(s) 
 *        4989 :
 *        Modified handling of LNB polarization and tone to allow correct 
 *        setting
 *        by the channel change API.
 *        
 *  1    mpeg      1.0         9/30/02 12:15:52 PM    Billy Jackman   
 * $
 * 
 ****************************************************************************/

#endif /* __LNB */

