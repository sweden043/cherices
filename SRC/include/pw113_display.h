#ifndef INCLUDE_PW113_DISPLAY_H
#define INCLUDE_PW113_DISPLAY_H
/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                        Conexant Systems Inc. (c)                         */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        PW113_DISPLAY.H
 *
 *
 * Description:     PixelWorks PW113 device driver header file
 *
 *
 * Author:          Ian Mitchell
 *
 ****************************************************************************/
/* $Id: pw113_display.h,v 1.2, 2003-12-23 15:54:40Z, Ian Mitchell$
 ****************************************************************************/

typedef enum
{
   PW113_SUCCESS,
   PW113_INVALID,
   PW113_NOT_INIT,
   PW113_ERROR
} PW113_STATUS;

typedef enum
{
   PW113_SCREEN_SIZE_16_9,
   PW113_SCREEN_SIZE_4_3
} PW113_SCREEN_SIZE;

typedef struct
{
   u_int8   ui8Contrast;
   u_int8   ui8Saturation;
   u_int8   ui8Hue;
   u_int8   ui8Sharpness;
   u_int8   ui8Brightness;
} PW113_DISPLAY_SETTINGS;

typedef enum
{
   PW113_SCREEN_STANDARD_PAL,
   PW113_SCREEN_STANDARD_NTSC,
   PW113_SCREEN_STANDARD_SECAM
} PW113_SCREEN_STANDARD;

typedef enum
{
   PW113_SCREEN_OSD_COMPOSITE,
   PW113_SCREEN_OSD_S_VIDEO,
   PW113_SCREEN_OSD_COMPONENT,
   PW113_SCREEN_OSD_DTV,
   PW113_SCREEN_OSD_ATV
} PW113_SCREEN_OSD;

#define PW113_DEFAULT_BRIGHTNESS       (128) /* range of 0-255 */
#define PW113_DEFAULT_CONTRAST         (32)  /* range of 0-63 */
#define PW113_DEFAULT_SATURATION       (128) /* range of 0-255 */
#define PW113_DEFAULT_HUE              (128) /* range of 0-255 */
#define PW113_DEFAULT_SHARPNESS        (128) /* range of 0-255 */
#define PW113_DEFAULT_SCREEN_STANDARD  PW113_SCREEN_STANDARD_PAL
#define PW113_DEFAULT_SCREEN_SIZE      PW113_SCREEN_SIZE_16_9

PW113_STATUS   cnxt_pw113_display_init(void);

PW113_STATUS   cnxt_pw113_display_set_screen_size(PW113_SCREEN_SIZE ssSize);
PW113_STATUS   cnxt_pw113_display_get_screen_size(PW113_SCREEN_SIZE *pssSize);

PW113_STATUS   cnxt_pw113_display_set_screen_settings(PW113_DISPLAY_SETTINGS  dsSettings);
PW113_STATUS   cnxt_pw113_display_get_screen_settings(PW113_DISPLAY_SETTINGS  *pdsSettings);

PW113_STATUS   cnxt_pw113_display_set_screen_standard(PW113_SCREEN_STANDARD   ssStandard);
PW113_STATUS   cnxt_pw113_display_get_screen_standard(PW113_SCREEN_STANDARD   *pssStandard);

PW113_STATUS   cnxt_pw113_display_set_de_interlacer_power(bool bOn);
PW113_STATUS   cnxt_pw113_display_get_de_interlacer_power(bool *bOn);

PW113_STATUS   cnxt_pw113_display_power(bool bPowerOn);

PW113_STATUS   cnxt_pw113_display_show_osd(PW113_SCREEN_OSD  psoOsd,
                                                          const char        *lpString,
                                                          u_int8            ui8Seconds);
PW113_STATUS   cnxt_pw113_display_clear_osds(void);

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         12/23/03 9:54:40 AM    Ian Mitchell    CR(s) 
 *        7739 : New API's and modifications from testing on the Crosby 
 *        platform.
 *        
 *  2    mpeg      1.1         12/2/03 4:36:51 AM     Ian Mitchell    CR(s): 
 *        7739 Add functions to display osd's and power on and off the display.
 *        
 *  1    mpeg      1.0         11/25/03 7:46:08 AM    Ian Mitchell    CR(s): 
 *        7739 Header file for the PW113 driver.
 * $
 * 
 ****************************************************************************/
#endif /* #ifndef INCLUDE_PW113_DISPLAY_H */
