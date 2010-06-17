/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           fpleds.h                                             */
/*                                                                          */
/* Description:        Front Panel LEDS driver                              */
/*                                                                          */
/* Author:             Steve Glennon                                        */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header $
$Log: 
 9    mpeg      1.8         3/20/03 4:12:24 PM     Matt Korte      SCR(s) 5837 
       :
       Add display routine prototype
       
 8    mpeg      1.7         11/26/02 3:59:28 PM    Dave Wilson     SCR(s) 4902 
       :
       Changed IIC bus parameter on all iic calls to use label from config 
       file.
       
 7    mpeg      1.6         7/11/00 8:12:32 PM     Dave Wilson     Added macros
        to allow code images that include FPLEDS calls to compile
       when the driver is not include in the build
       
 6    mpeg      1.5         3/28/00 12:26:20 PM    Dave Wilson     Added new 
       APIs to support 20x4 matrix display
       
 5    mpeg      1.4         3/3/00 1:34:44 PM      Tim Ross        Eliminated 
       code that would be inefficient for inclusion in the boot
       block using the RTOS == NOOS switch.
       
 4    mpeg      1.3         4/28/99 12:25:54 PM    Dave Wilson     Added 
       #ifndef _FPLEDS_H_ protection.
       
 3    mpeg      1.2         11/6/98 9:44:32 AM     Dave Wilson     Added 
       conditional definition of BYTE.
       
 2    mpeg      1.1         10/20/98 5:34:48 PM    Steve Glennon   Fixed up and
        made it work.
       
 1    mpeg      1.0         10/19/98 4:59:22 PM    Steve Glennon   
$
*/

#ifndef _FPLEDS_H_
#define _FPLEDS_H_

/************/
/* Segments */
/************/
#define TP     0x01
#define TR     0x02
#define BR     0x04
#define BT     0x08
#define BL     0x10
#define TL     0x20
#define MD     0x40
#define DP     0x80
#define LED_BLANK 0x00

#ifndef BYTE
#define BYTE u_int8
#endif

#define LED_CONFIG_REG_VAL 0x37    /* 9mA drive, alternating mode, all digits enabled */
#define NUM_LEDS           4

#ifdef DRIVER_INCL_FPLEDS

typedef enum _CNXT_LED_STATUS
{
    CNXT_LED_OK = 0,
    CNXT_LED_ERROR
} CNXT_LED_STATUS;

/****************************/
/* Real function prototypes */
/****************************/
bool LEDInit(void);
bool LEDIsMatrixDisplay(void);
bool LEDGetDisplaySize(int *width, int *height);
bool LEDLongString(char *string);
bool LEDStringAt(char *string, int x, int y);
void LEDString(char *pszString);  
void LEDClearDisplay(void);
CNXT_LED_STATUS LEDDisplayApplication(char *pszAppName);

#if RTOS != NOOS
bool LEDLongIntAt(int number, bool hex, int x, int y);
void LEDVal(unsigned int i, int num_digits, int bHex);
#define LEDInt(i, d) LEDVal(i,d,FALSE)
#define LEDHex(i, d) LEDVal(i,d,TRUE)
#endif
void LEDSegs(BYTE a, BYTE b, BYTE c, BYTE d);

#else

/*********************************************************************/
/* Dummy functions to allow code to build when the FPLEDS library is */
/* not included in the image                                         */
/*********************************************************************/
#define  LEDInit()              TRUE
#define  LEDIsMatrixDisplay()   FALSE
#define  LEDGetDisplaySize(w,h) FALSE  
#define  LEDLongString(s)       TRUE
#define  LEDStringAt(s,x,y)     TRUE
#define  LEDString              if (0) ((int (*)(char *)) 0)
#define  LEDClearDisplay        if (0) ((int (*)(void)) 0)

#define LEDLongIntAt(n,h,x,y)   TRUE
#define LEDVal                  if (0) ((int (*)(unsigned int, int, int)) 0)
#define LEDInt                  if (0) ((int (*)(unsigned int, int)) 0)
#define LEDHex                  if (0) ((int (*)(unsigned int, int)) 0)
#define LEDSegs                 if (0) ((int (*)(BYTE, BYTE, BYTE, BYTE)) 0)

#endif
#endif

