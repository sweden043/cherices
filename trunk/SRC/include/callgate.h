/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           CALLGATE.H                                           */
/*                                                                          */
/* Description:        Public header file describing call IDs and parameter */
/*                     blocks for o-code to native function calls.          */
/*                                                                          */
/* Author:             Christopher Chapman & Dave Wilson                    */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: callgate.h, 17, 12/9/99 12:25:50 PM, Dave Wilson$
$Log: 
 17   mpeg      1.16        12/9/99 12:25:50 PM    Dave Wilson     Added some 
       OpenTV EN2 specific functions
       
 16   mpeg      1.15        6/25/99 12:04:30 PM    Dave Wilson     Added API to
        get processor in use.
       
 15   mpeg      1.14        6/15/99 4:28:36 PM     Dave Wilson     Added calls 
       mapping to Set/GetMpgRectClipped.
       
 14   mpeg      1.13        6/2/99 12:46:52 PM     Dave Wilson     Added 
       SetCursorInvertColor call.
       
 13   mpeg      1.12        5/31/99 3:58:08 PM     Dave Wilson     Added a few 
       more calls to enable the video/graphics compositing demo.
       
 12   mpeg      1.11        3/18/99 1:49:18 PM     Dave Wilson     Added hooks 
       for TunerAddChannel and TunerRemoveChannel.
       
 11   mpeg      1.10        3/16/99 12:40:08 PM    Dave Wilson     Added 
       software version number query.
       
 10   mpeg      1.9         2/24/99 3:20:00 PM     Ismail Mustafa  Added 
       O_rockwell_flash_image & CALLID_FLASHIMAGE.
       
 9    mpeg      1.8         1/29/99 9:33:52 AM     Dave Wilson     Added GCP 
       demo calls.
       
 8    mpeg      1.7         1/21/99 12:36:34 PM    Dave Wilson     Added a 
       bunch more functions for tuner control.
       
 7    mpeg      1.6         1/10/99 9:54:36 AM     Dave Wilson     Analog TV 
       setup and ploarization signal fields added.
       
 6    mpeg      1.5         11/21/98 4:40:24 PM    Dave Wilson     Added glue 
       for live video functions
       
 5    mpeg      1.4         11/6/98 9:45:20 AM     Dave Wilson     Added new 
       calls for Front Panel LED setting.
       
 4    mpeg      1.3         10/15/98 3:19:56 PM    Dave Wilson     Added hook 
       for GetMpgRect.
       
 3    mpeg      1.2         10/12/98 3:48:10 PM    Dave Wilson     Latest 
       version
       
 2    mpeg      1.1         9/30/98 11:29:44 AM    Dave Wilson     Added bit 
       blit glue.
       
 1    mpeg      1.0         9/29/98 10:22:14 AM    Dave Wilson     
$
 * 
 *    Rev 1.16   09 Dec 1999 12:25:50   dawilson
 * Added some OpenTV EN2 specific functions
 * 
 *    Rev 1.15   25 Jun 1999 11:04:30   dawilson
 * Added API to get processor in use.
 * 
 *    Rev 1.14   15 Jun 1999 15:28:36   dawilson
 * Added calls mapping to Set/GetMpgRectClipped.
 * 
 *    Rev 1.13   02 Jun 1999 11:46:52   dawilson
 * Added SetCursorInvertColor call.
 * 
 *    Rev 1.12   31 May 1999 14:58:08   dawilson
 * Added a few more calls to enable the video/graphics compositing demo.
 * 
 *    Rev 1.11   18 Mar 1999 13:49:18   dawilson
 * Added hooks for TunerAddChannel and TunerRemoveChannel.
 *
 *    Rev 1.10   16 Mar 1999 12:40:08   dawilson
 * Added software version number query.
 *
 *    Rev 1.9   24 Feb 1999 15:20:00   mustafa
 * Added O_rockwell_flash_image & CALLID_FLASHIMAGE.
 *
 *    Rev 1.8   29 Jan 1999 09:33:52   dawilson
 * Added GCP demo calls.
 *
 *    Rev 1.7   21 Jan 1999 12:36:34   dawilson
 * Added a bunch more functions for tuner control.
 *
 *    Rev 1.6   10 Jan 1999 09:54:36   dawilson
 * Analog TV setup and ploarization signal fields added.
 *
 *    Rev 1.5   21 Nov 1998 16:40:24   dawilson
 * Added glue for live video functions
 *
 *    Rev 1.4   06 Nov 1998 09:45:20   dawilson
 * Added new calls for Front Panel LED setting.
 *
 *    Rev 1.3   15 Oct 1998 14:19:56   dawilson
 * Added hook for GetMpgRect.
 *
 *    Rev 1.2   12 Oct 1998 14:48:10   dawilson
 * Latest version
 *
 *    Rev 1.1   30 Sep 1998 10:29:44   dawilson
 * Added bit blit glue.
 *
 *    Rev 1.0   29 Sep 1998 09:22:14   dawilson
 * Initial revision.
*
*/

/****************************************************************************/
/* NB: This header file is used by both native and o-code applications. Do  */
/*     not use double slash comments (since these are illegal as far as the */
/*     o-code compiler is concerned) and stick to basic C data types in     */
/*     structures.                                                          */
/****************************************************************************/

/*********************/
/* Function Call IDs */
/*********************/
#define CALLID_SETMPGRECT            0x01
#define CALLID_CREATEOSDREGION       0x02
#define CALLID_DESTROYOSDREGION      0x03
#define CALLID_SETOSDRGNOPTIONS      0x04
#define CALLID_GETOSDRGNOPTIONS      0x05
#define CALLID_SETOSDRGNRECT         0x06
#define CALLID_GETOSDRGNRECT         0x07
#define CALLID_GETOSDRGNBPP          0x08
#define CALLID_SETOSDRGNPALETTE      0x09
#define CALLID_SETOSDRGNALPHA        0x0A
#define CALLID_SETOSDCOLORKEY        0x0B
#define CALLID_GETOSDCOLORKEY        0x0C
#define CALLID_SETOSDBACKGROUND      0x0D
#define CALLID_GETOSDBACKGROUND      0x0E
#define CALLID_SETDEFAULTOSDPALETTE  0x0F
#define CALLID_GETOSDRGNPALENTRY     0x10
#define CALLID_OSDBITBLT             0x11
#define CALLID_GETMPGRECT            0x12
#define CALLID_OSDRLBITBLT           0x13
#define CALLID_LEDSTRING             0x14
#define CALLID_LEDINT                0x15
#define CALLID_SETTOPVIDEO           0x16
#define CALLID_GETTOPVIDEO           0x17
#define CALLID_CREATELIVEVIDEORGN    0x18
#define CALLID_DESTROYLIVEVIDEORGN   0x19
#define CALLID_LIVEVIDEOENABLE       0x1A
#define CALLID_SETLIVEVIDEOOPTION    0x1B
#define CALLID_GETLIVEVIDEOOPTION    0x1C
#define CALLID_SETLIVEVIDEOPOS       0x1D
#define CALLID_GETLIVEVIDEOPOS       0x1E
#define CALLID_SETLIVEVIDEOCON       0x1F
#define CALLID_GETLIVEVIDEOCON       0x20
#define CALLID_ISLIVEVIDEORGN        0x21
#define CALLID_TUNERTESTCOUNTRY      0x22
#define CALLID_TUNERGETCHANNELLIMITS 0x23
#define CALLID_TUNERSETCOUNTRY       0x24
#define CALLID_TUNERGETCOUNTRY       0x25
#define CALLID_TUNERSETMEDIUM        0x26
#define CALLID_TUNERGETMEDIUM        0x27
#define CALLID_TUNERGETSTANDARD      0x28
#define CALLID_TUNERSETCHANNEL       0x29
#define CALLID_TUNERGETCHANNEL       0x2A
#define CALLID_TUNERCHANNELUP        0x2B
#define CALLID_TUNERCHANNELDOWN      0x2C
#define CALLID_TUNERAUTOSCAN         0x2D
#define CALLID_TUNERADDCHANNEL       0x2E
#define CALLID_TUNERREMOVECHANNEL    0x2F
#define CALLID_AUDIOROUTING          0x30
#define CALLID_GRAPHICSCOPROCDEMO    0x31
#define CALLID_FLASHIMAGE            0x32
#define CALLID_GETSOFTWAREVERSION    0x33
#define CALLID_MEMBLIT16             0x34
#define CALLID_SETCURSORINVERT       0x35
#define CALLID_SETMPEGCROPSCALE      0x36
#define CALLID_GETMPEGCROPSCALE      0x37
#define CALLID_GETSILICONREVISION    0x38
#define CALLID_SVLSETSERVICE         0x39
#define CALLID_SVLGETNEXTSERVICE     0x3A
#define CALLID_SVLMAPSERVICEINDEX    0x3B
#define CALLID_SVLMAPINDEXSERVICE    0x3C

#define OPENTV_OSD_HANDLE            0xFFFFFFFF

/*******************************/
/* Parameter block definitions */
/*******************************/
typedef struct _callgate_parms
{
        unsigned int    parms[8];
} callgate_parms;

/****************************************************/
/* O-code macros allowing easier calls to functions */
/****************************************************/
#ifdef __ocod__

extern void O_rockwell_set_mpeg_rect(int left, int top, int right, int bottom);
extern int  O_rockwell_get_mpeg_rect(short *left, short *top, short *right, short *bottom);
extern int  O_rockwell_create_osd_region(int left, int top, int right, int bottom, int mode, int options);
extern int  O_rockwell_destroy_osd_region(int handle);
extern int  O_rockwell_set_osd_region_options(int handle, int flags, int value);
extern int  O_rockwell_get_osd_region_options(int handle, int flags);
extern int  O_rockwell_set_osd_region_rect(int handle, int left, int top, int right, int bottom);
extern int  O_rockwell_get_osd_region_rect(int handle, int *pleft, int *ptop, int *pright, int *pbottom);
extern int  O_rockwell_get_osd_region_bpp(int handle);
extern int  O_rockwell_set_osd_region_palette(int handle, unsigned int *ppalette, int bLoadAlpha);
extern int  O_rockwell_set_osd_region_alpha(int handle, int flags, int alpha, int index);
extern void O_rockwell_osd_color_key(unsigned int colorkey);
extern unsigned int O_rockwell_get_osd_color_key(void);
extern int  O_rockwell_set_osd_background(unsigned int ycc_color);
extern unsigned int O_rockwell_get_osd_background(void);
extern int  O_rockwell_set_default_osd_palette(int handle);
extern unsigned int  O_rockwell_get_osd_region_pal_entry(int handle, int index);
extern void O_rockwell_bitblt(int handle, void *data, int bytes_per_line, int left, int top, int right, int bottom);
extern void O_rockwell_rl_bitblt(int handle, o_pixmap_rl *data, int left, int top, int right, int bottom);
extern void O_rockwell_led_string(char *pszString);
extern void O_rockwell_led_int(int iInt, int iDigits);
extern unsigned int O_rockwell_create_live_video_region(short left, short top, short right, short bottom, int enable);
extern unsigned int O_rockwell_destroy_live_video_region(void);
extern unsigned int O_rockwell_set_top_video(int plane);
extern unsigned int O_rockwell_get_top_video(void);
extern unsigned int O_rockwell_enable_live_video(int enable);
extern unsigned int O_rockwell_set_live_video_option(unsigned int option, unsigned int value);
extern unsigned int O_rockwell_get_live_video_option(unsigned int option);
extern int  O_rockwell_set_live_video_pos(short left, short top, short right, short bottom);
extern int  O_rockwell_get_live_video_pos(short *left, short *top, short *right, short *bottom);
extern int  O_rockwell_set_live_video_connector(int connector);
extern int  O_rockwell_get_live_video_connector(void);
extern unsigned int O_rockwell_is_live_video(void);
extern int  O_rockwell_tuner_test_country(unsigned int country);
extern int  O_rockwell_tuner_get_channel_limits(unsigned int standard, unsigned int medium, unsigned int *pMin, unsigned int *pMax);
extern int  O_rockwell_tuner_set_country(unsigned int country);
extern unsigned int O_rockwell_tuner_get_country(void);
extern unsigned int O_rockwell_tuner_get_standard(void);
extern int  O_rockwell_tuner_set_medium(unsigned int medium);
extern unsigned int O_rockwell_tuner_get_medium(void);
extern int  O_rockwell_tuner_set_channel(unsigned int channel);
extern unsigned int O_rockwell_tuner_get_channel(void);
extern int  O_rockwell_tuner_channel_up(void);
extern int  O_rockwell_tuner_channel_down(void);
extern int  O_rockwell_tuner_auto_scan_channels(void);
extern int  O_rockwell_tuner_add_channel(unsigned int iChannel);
extern int  O_rockwell_tuner_remove_channel(unsigned int iChannel);
extern int  O_rockwell_set_audio_routing(unsigned int input, unsigned int outputs, unsigned int gain);
extern void O_rockwell_show_gcp_demo(unsigned int handle, unsigned int demo_num);
extern void O_rockwell_flash_image(void);
extern void O_rockwell_get_image_version(char *szLabel, int iLabelMax,
                                         char *szDate,  int iDateMax,
                                         char *szTime,  int iTimeMax);
extern void O_rockwell_memory_blit_16(char *pDest, char *pSrc, int DstPitch, int SrcPitch, int ExtX, int ExtY, int SrcX, int SrcY);
extern int  O_rockwell_set_cursor_invert_color(int Red, int Green, int  Blue, int bInvert);
extern int O_rockwell_set_mpeg_crop_scale(int sleft, int stop, int sright, int sbottom,
                                          int dleft, int dtop, int dright, int dbottom);
extern int O_rockwell_get_mpeg_crop_scale(int *sleft, int *stop, int *sright, int *sbottom,
                                          int *dleft, int *dtop, int *dright, int *dbottom);
extern int O_rockwell_get_silicon_revision(int *rev);
extern int O_rockwell_svl_set_service(int iService);
extern int O_rockwell_svl_get_next_service(int iService, bool bUpwards);
extern int O_rockwell_svl_map_service_index(int iService);
extern int O_rockwell_svl_map_index_service(int iIndex);

#endif
