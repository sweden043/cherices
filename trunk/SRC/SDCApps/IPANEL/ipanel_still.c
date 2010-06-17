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
#include "ipanel_os.h"
#include "ipanel_vdec.h"
#include "ipanel_still.h"

extern u_int32 gDemuxInstance;

extern UINT32_T  g_ipanel_video_channel, g_ipanel_audio_channel;

extern int FCopy( const void *dst, const void *src, int count );

//--------------------------------------------------------------------------------------------------
// Internal Prototypes
//--------------------------------------------------------------------------------------------------
//

HVIDRGN gbl_StillPlane    = (HVIDRGN)NULL;

void ipanel_decode_still(voidF pImage,u_int32 uSize)
{
	gen_farcir_q VideoQueue;

	gen_video_play(PLAY_STILL_STATE, NULL);

	VideoQueue.begin = (voidF)pImage;
	VideoQueue.size  = uSize+1;
	VideoQueue.in = VideoQueue.size;
	VideoQueue.out = 0;

	while (VideoQueue.in - VideoQueue.out >= 4)
	{
	  VideoQueue.out += gen_send_still_data((gen_farcir_q *) &VideoQueue);
	}

	gen_flush_data(NULL);

	gen_video_stop(NULL);	
}

bool ipanel_copy_video_to_region(HVIDRGN hTarget)
{
	HVIDRGN hRgnSrc;
	u_int32 dwSrcW, dwSrcH;
	u_int32 dwDstStride;
	u_int8 *pSrc, *pDst;
	u_int32 dwWidth, dwHeight;
	u_int8* pNext, *pCur;
	int32 line;

	hRgnSrc = vidGet420Hardware();

	/* Get the source and destination image sizes */
	dwSrcW = vidGetOptions(hRgnSrc, VO_IMAGE_WIDTH);
	dwSrcH = vidGetOptions(hRgnSrc, VO_IMAGE_HEIGHT);

	// dwDstStride = vidGetOptions(hTarget, VO_BUFFER_STRIDE);
	dwDstStride = CNXT_GET_VAL(DRM_MPEG_OFFSET_WIDTH_REG, DRM_MPEG_OFFSET_WIDTH_FETCH_WIDTH_MASK);

	/* Note - this assumes we are copying the currently displayed image into the */
	/* target video region. If you want to copy something decoded into a back    */
	/* buffer, you must ensure that you set up the pointer correctly here.       */
	gen_video_get_current_display_buffer(&pSrc);
	
	// gen_video_get_safe_still_buffer(&pSrc);
	pDst   = (u_int8 *)vidGetOptions(hTarget, VO_BUFFER_PTR);

	/* Work out the width to blit */
	dwWidth  = dwSrcW;
	dwHeight = dwSrcH;

	/* Copy the luma plane first */
	GfxCopyRectMem((u_int32*)pDst, (u_int32*)pSrc, dwDstStride,
	               dwSrcW, dwWidth, dwHeight, 0, 0, 0, 0, 8);

	pCur = pDst;
	pNext = pDst + dwDstStride;
	
	for(line = 0; line<dwHeight; line+=2)  // copy line to (line+1)
	{
		FCopy(pNext, pCur, dwDstStride);
		pCur = pNext+ dwDstStride;
		pNext = pCur + dwDstStride;
	}

	/* Then copy the chroma. Move the pointers to the chroma buffers. */
	pSrc += dwSrcW * dwSrcH;
	pDst += dwDstStride * dwSrcH;

	// Adjust the area for the 16 bpp chroma.
	dwHeight ++;
	dwHeight >>= 1;
	dwWidth >>= 1;

	GfxCopyRectMem((u_int32*)pDst, (u_int32*)pSrc, dwDstStride,
		  		    dwSrcW, dwWidth, dwHeight, 0, 0, 0, 0, 16);

	vidSetOptions(hTarget, VO_IMAGE_WIDTH, dwSrcW, 0);
	vidSetOptions(hTarget, VO_IMAGE_HEIGHT, dwSrcH, 1);

	return(TRUE);
}

int ipanel_decode_still_to_region(voidF pImage, u_int32 uSize)
{
	bool bRetcode;
	gen_farcir_q VideoQueue ;
	CNXT_VID_STATUS eRetcode;
	static HVIDRGN StillPlane = (HVIDRGN)NULL;
	OSDRECT rectDest ;

	if (StillPlane == (HVIDRGN)NULL)
	{
		rectDest.bottom = STILL_BOTTOM;
		rectDest.top    = STILL_TOP ;
		rectDest.left   = STILL_LEFT ;
		rectDest.right  = STILL_RIGHT ;

		StillPlane = vidCreateVideoRgn(&rectDest, TYPE_420);
		if (!StillPlane)
		{
			ipanel_porting_dprintf("can't create offscreen video region!\n");
			return (FALSE);
		}

		gbl_StillPlane = StillPlane;
		
		/* Now set various properties for the region */
		vidSetOptions(StillPlane, VO_CONNECTION, SOFTWARE, FALSE);
		vidSetOptions(StillPlane, VO_IMAGE_ON_TOP, FALSE, FALSE);
		vidSetOptions(StillPlane, VO_AUTOMODE, VID_AUTO_ALL, FALSE);
		
		/* Associate the new region with the hardware image plane but don't */
		/* display it yet since it has nothing in it.                       */
		vidSetRegionInput(StillPlane, 1);
		vidDisplayRegion(StillPlane, FALSE);
		
		/* Full source image scaled to full screen */
		vidSetPos(StillPlane, NULL, NULL);
	}

	cnxt_dmx_channel_control( gDemuxInstance, g_ipanel_video_channel,
							  (gencontrol_channel_t) GEN_DEMUX_DISABLE );

	eRetcode = gen_video_play(PLAY_STILL_STATE, NULL);
	if (eRetcode == CNXT_VID_STATUS_OK)
	{
		VideoQueue.begin = pImage;
		VideoQueue.size  = uSize;
		VideoQueue.in = VideoQueue.size;
		VideoQueue.out = 0;
		
		while (VideoQueue.in - VideoQueue.out >= 4)
		{
			VideoQueue.out += gen_send_still_data((gen_farcir_q *) &VideoQueue);
		}
		
		task_time_sleep(400);
		
		bRetcode = gen_flush_data(NULL);
	
		/* Copy it into the target video region */
		if(bRetcode)
		{
			bRetcode = ipanel_copy_video_to_region( StillPlane );
		}
		
		cnxt_dmx_channel_control( gDemuxInstance, g_ipanel_video_channel, 
			                     (gencontrol_channel_t)GEN_DEMUX_ENABLE );

		gen_video_play( PLAY_LIVE_STATE_NO_SYNC, NULL );
	}
	else
	{
		/* Bad return code from gen_video_play */
		bRetcode = FALSE;
	}
	
	/* Display the background region if the decode completed successfully */
    if(bRetcode)
		bRetcode = vidDisplayRegion(StillPlane, TRUE);
	
	return(bRetcode);	
}

void ipanel_clear_still_plane()
{
    u_int8 *pbuff;
    
    pbuff=(u_int8 *)vidGetOptions(gbl_StillPlane, VO_BUFFER_PTR);
    
    memset(pbuff, 0x10, STILL_RIGHT*STILL_BOTTOM);
    memset(pbuff+STILL_RIGHT*STILL_BOTTOM, 0x80, STILL_RIGHT*STILL_BOTTOM/2);
}

bool ipanel_WaitForAudioValid(u_int32 Timeoutms, u_int32 WaitAfterValidms)
{
    bool        bValid = FALSE;
    u_int32     StartTime;
    u_int32     frametime = 24;   /* frame time at 48KHz MPEG */

    /* Wait for audio data to be valid with timeout */
    StartTime = ipanel_porting_time_ms();
    while (!bValid && ((ipanel_porting_time_ms() - StartTime) < Timeoutms))
    {
        /* Sleep a couple of frame times then check for valid again */
        task_time_sleep(2 * frametime);
        bValid = cnxt_audio_is_valid();
    }

    /* now wait the extra time after valid audio to avoid repeats) */
    if (bValid)
    {
        task_time_sleep(WaitAfterValidms);
    }

    return bValid;
}

