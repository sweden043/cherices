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
#include "ipanel_base.h"
#include "ipanel_media.h"

/**********************************************************************************/
/*Description: play the opened media                                              */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
/**********************************************************************************/
INT32_T ipanel_porting_program_play(VOID)
{
	ipanel_porting_dprintf("[ipanel_porting_program_play]\n");
	return IPANEL_ERR;
}

/**********************************************************************************/
/*Description: Open a media URL                                                   */
/*Input      : media url string, media type such as "MPEG" "mp3" NULL etc         */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_media_open(CONST CHAR_T *url, CONST CHAR_T *mediatype)
{
    return IPANEL_OK;
}

/**********************************************************************************/
/*Description: Close the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_media_close(VOID)
{
    printf("[ipanel_porting_media_close]\n");
    return IPANEL_OK;
}

/**********************************************************************************/
/*Description: get media track length for time                                    */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : length, uints is seconds                                           */
/**********************************************************************************/
INT32_T ipanel_porting_media_duration(VOID)
{
    ipanel_porting_dprintf("[ipanel_porting_media_duration] \n");
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: get media status                                                   */
/*Input      : save speed buffer                                                  */
/*Output     : speed (optioned)                                                   */
/*Return     : media status                                                       */
/**********************************************************************************/
CONST CHAR_T *ipanel_porting_media_status(INT32_T *speed)
{
	ipanel_porting_dprintf("[ipanel_porting_media_status]\n");
	return IPANEL_NULL;
}


/**********************************************************************************/
/*Description: Reset the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_reset_media_setting(void)
{
	/*reset physical media here*/
    ipanel_porting_dprintf("[ipanel_porting_reset_media_setting] \n");
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: pause the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_pause(VOID)
{
	ipanel_porting_dprintf("[ipanel_porting_program_pause]\n");
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: stop the opened media                                              */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_stop(VOID)
{
	ipanel_porting_dprintf("[ipanel_porting_program_stop]\n");
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: resume the opened media                                            */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_resume(VOID)
{
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: play the opened media fast                                         */
/*Input      : multi speed                                                        */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_forward(INT32_T number)
{
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: play the opened media slow                                         */
/*Input      : division speed                                                     */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_slow(INT32_T number)
{
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: repeat the opened media times                                      */
/*Input      : repeat times, -1 - no limits                                       */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_repeat(INT32_T number)
{
	ipanel_porting_dprintf("[ipanel_porting_program_repeat]\n");
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: rewind the opened media                                            */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_rewind(VOID)
{
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: seek a position of the opened media                                */
/*Input      : mode (ipanel_media_seek_mod_t)                                     */
/*             value is (0 ~86399) in time mode, and >0 in chapter mode.          */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_seek(INT32_T mode, INT32_T value)
{
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: break the opened media                                             */
/*Input      : No                                                                 */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_break(VOID)
{
	ipanel_porting_dprintf("[ipanel_porting_program_break]\n");
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: reverse the opened media                                           */
/*Input      : reverse speed                                                      */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_porting_program_reverse(INT32_T number)
{
	return IPANEL_OK;
}

/**********************************************************************************/
/*Description: set the media mode, audio or video mode                            */
/*Input      : "av" or "tv"                                                       */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_video_tvav(const char *mode)
{
	int ret = IPANEL_OK;

	//if ( strcmp(mode, "av")==0 || strcmp(mode, "tv")==0 ) {
	//	ret = 0;
	//}
	return ret;
}


/**********************************************************************************/
/*Description: set parameter value,                                               */
/*Input      : parameter name and parameter value.                                */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
int ipanel_porting_vod_set_param(const char *name, const char *value)
{
	/*set parameter value here*/
	return IPANEL_OK;
}


/**********************************************************************************/
/*Description: get parameter value,                                               */
/*Input      : parameter name                                                     */
/*Output     : No                                                                 */
/*Return     : parameter value                                                    */
/**********************************************************************************/
const char *ipanel_porting_vod_get_param(const char*name)
{
	return IPANEL_NULL;
}
