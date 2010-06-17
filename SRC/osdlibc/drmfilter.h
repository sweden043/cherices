/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       drmfilter.h
 *
 *
 * Description:    Header for DRM scaler filter coefficient setup code
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: drmfilter.h, 2, 1/22/03 9:36:36 AM, Matt Korte$
 ****************************************************************************/
#ifndef _DRMFILTER_H_
#define _DRMFILTER_H_

/********************************************************************/
/*  FUNCTION:    cnxt_drm_init_filter_coefficients                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Dummy function for ICs earlier than Brazos (where  */
/*               the filter coefficients were in internal ROM)      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void cnxt_drm_init_filter_coefficients(void);

#endif  /* _DRMFILTER_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         1/22/03 9:36:36 AM     Matt Korte      SCR(s) 
 *        5258 :
 *        Fix EOF.
 *        
 *        
 *  1    mpeg      1.0         12/17/02 3:37:46 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.1   22 Jan 2003 09:36:36   kortemw
 * SCR(s) 5258 :
 * Fix EOF.
 * 
 * 
 *    Rev 1.0   17 Dec 2002 15:37:46   dawilson
 * SCR(s) 5073 :
 * Header for DRM filter initialisation functions
 *
 ****************************************************************************/
