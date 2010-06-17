/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       cnxt_types.h
 *
 *
 * Description:    Common type definitions used by Conexant driver software
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: cnxt_types.h, 1, 3/1/04 3:01:56 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _CNXT_TYPES_H_
#define _CNXT_TYPES_H_

/************************************/
/* General time and date structures */
/************************************/
typedef enum
{
  JANUARY = 1,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVEMBER,
  DECEMBER,
  NUMBER_OF_MONTHS
} CNXT_MONTH;

typedef enum
{
  SUNDAY = 1,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  NUMBER_OF_DAYS
} CNXT_DAY;
  
typedef struct
{
  CNXT_MONTH Month;
  u_int8     uDay;
  CNXT_DAY   DayOfWeek;
  u_int8     uHour;
  u_int8     uMinute;
  u_int8     uSecond;
} CNXT_UTC_TIME;

typedef struct
{
  int16 nX;
  int16 nY;
} CNXT_XY;

typedef struct
{
  u_int16 uWidth;
  u_int16 uHeight;
} CNXT_SIZE;

typedef struct
{
  u_int16 uLeft;
  u_int16 uTop;
  u_int16 uRight;
  u_int16 uBottom;
} CNXT_RECT;

typedef struct
{
  bool bRGB;
  /* Add appropriate union for RGB/YCrCb color */
} CNXT_COLOR;
#endif /* _CNXT_TYPES_H_ */


/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         3/1/04 3:01:56 PM      Dave Wilson     CR(s) 
 *        8484 : Standard Conexant data types (Edwards header file)
 * $
 *
 ****************************************************************************/

