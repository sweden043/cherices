/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           WARNFIX.H                                            */
/*                                                                          */
/* Description:        Public header file containing various items to       */
/*                     fix up otherwise bothersome warnings from the        */
/*                     compiler. It is preferable to have this in a         */
/*                     separate header to avoid making changes to           */
/*                     'delivered' source code, such as pSOS header files   */
/*                     or OpenTV delivered files which generate benign      */
/*                     warnings.                                            */
/*                                                                          */
/* Author:             Steve Glennon                                        */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* NOTE:                                                                    */
/*                                                                          */
/* Please DO NOT use double-slash comments in this file. It is used by some */
/* OpenTV o-code applications and the o-code compiler falls over whenever   */
/* it hits a non-standard comment.                                          */
/*                                                                          */
/****************************************************************************/

#ifndef _WARNFIX_H_
#define _WARNFIX_H_

#ifndef __GNUC__
#define __GNUC__ 0
#endif

/* to fix undefined __CADUL__ in stddef.h */
#ifndef __CADUL__
#define __CADUL__ 0
#endif

/* to fix problems in PSOS 2.51 headers */
#ifndef __DIAB
 #define __DIAB 0
#endif
#ifndef GHS
 #define GHS 0
#endif

#endif

