/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           OCODEEXT.H                                           */
/*                                                                          */
/* Description:        OpenTV Library Extensions header file (o-code side)  */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header:   K:/sabine/pvcs/include/ocodeext.h_v
$Log:   K:/sabine/pvcs/include/ocodeext.h_v
*/

/* This header is to be included in the o-code application */

#ifndef _SABOCODE_H_
#define _SABOCODE_H_

extern unsigned long O_rockwell_test_api(int parm1, zero char *parm2);
extern unsigned long O_rockwell_get_config(zero char *confdata);
extern unsigned long O_rockwell_set_config(zero char *confdata);

#endif

