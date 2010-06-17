/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           SABOCODE.H                                           */
/*                                                                          */
/* Description:        OpenTV Library Extensions header file                */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header:   K:/sabine/pvcs/sabocode/sabocode.h_v
$Log:   K:/sabine/pvcs/sabocode/sabocode.h_v
*/

#include "oconfig.h"

#ifndef _SABOCODE_H_
#define _SABOCODE_H_

#ifdef __ocod__
extern unsigned long O_rockwell_test_api(int parm1, char *parm2);
extern unsigned long O_rockwell_get_config(sabine_ocode_config *confdata);
extern unsigned long O_rockwell_set_config(sabine_ocode_config *confdata);
extern unsigned long O_rockwell_call_native(int call_id, void *parms);
#else
extern unsigned long o_rockwell_test_api(int parm1, char *parm2);
extern unsigned long o_rockwell_get_config(sabine_ocode_config *confdata);
extern unsigned long o_rockwell_set_config(sabine_ocode_config *confdata);
extern unsigned long o_rockwell_call_native(int call_id, void *parms);
#endif

#endif
