/*********************************************************************
    Copyright (c) 2008 - 2010 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the base Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    History:
		version		date		name		desc
         0.01     2009/8/1     Vicegod     create
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_MEDIA_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_MEDIA_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* open a media url */
INT32_T ipanel_porting_media_open(CONST CHAR_T *url, CONST CHAR_T *mediatype);

/* close the opened media */
INT32_T ipanel_porting_media_close(VOID);

/* get media track length for time */
INT32_T ipanel_porting_media_duration(VOID);

/* get media status, special need speed */
CONST CHAR_T *ipanel_porting_media_status(INT32_T *speed);

/* play the opened media */
INT32_T ipanel_porting_program_play(VOID);

/* pause the opened media */
INT32_T ipanel_porting_program_pause(VOID);

/* stop the opened media */
INT32_T ipanel_porting_program_stop(VOID);

/* resume the opened media */
INT32_T ipanel_porting_program_resume(VOID);

/* play the opened media fast */
INT32_T ipanel_porting_program_forward(INT32_T number);

/* play the opened media slow */
INT32_T ipanel_porting_program_slow(INT32_T number);

/* repeat the opened media times */
INT32_T ipanel_porting_program_repeat(INT32_T number);

/* rewind the opened media */
INT32_T ipanel_porting_program_rewind(VOID);

/* seek a position of the opened media */
INT32_T ipanel_porting_program_seek(INT32_T mode, INT32_T value);

/* break the opened media */
INT32_T ipanel_porting_program_break(VOID);

/* reverse the opened media */
INT32_T ipanel_porting_program_reverse(INT32_T number);

#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_MEDIA_API_FUNCTOTYPE_H_

