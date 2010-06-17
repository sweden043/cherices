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
#include "ipanel_config.h"
#include "ipanel_vout.h"

extern VIDEO_STATUS ipanel_get_video_status(void);

extern void gen_video_unblank_internal(void);

static INT32_T  ipanel_set_win_location(IPANEL_RECT *rect);

static CNXT_TVENC_HANDLE m_TvencHandle = NULL;

//static IPANEL_RECT s_last_rect;
/**********************************************************************************/
/*Description: Set TV decode mode. 1 - NTSC, 2 - PAL, 3 - SECAM, 4 - AUTO         */
/*Input      : mode value, if value is not in 1 to 4, the return fail             */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_video_mode(INT32_T mode)
{
	INT32_T ret = IPANEL_OK;

	if (mode >= 0 && mode < 4) 
	{
        ipanel_porting_dprintf("[ipanel_set_video_mode] mode = %d.\n",mode);
        
		/*set physical TV decode mode here*/
		switch(mode)
		{
			case IPANEL_DIS_TVENC_NTSC:
				//SetOutputType(NTSC);
				break;
			
			case IPANEL_DIS_TVENC_PAL:
				SetOutputType(PAL);
				break;
			
			case IPANEL_DIS_TVENC_SECAM:
				SetOutputType(SECAM);
				break;
			
			case IPANEL_DIS_TVENC_AUTO:
			default :
				SetOutputType(PAL);
				
				break;
		}
	}
	//neet to reset the location of little video
	//ipanel_set_win_location((IPANEL_RECT *)&s_last_rect);
	return ret;
}

/**********************************************************************************/
/*Description: Set TV screen display. 1 - width:height=4:3, 2 - width:height=16:9 */
/*Input      : scale value, if value is not in 1 to 2, the return fail            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_video_visable(INT32_T bVisable)
{
	INT32_T ret = IPANEL_OK;

	// set TV video visable or disable 

	return ret ;
}

/**********************************************************************************/
/*Description: Set TV screen scale. 1 - width:height=4:3, 2 - width:height=16:9   */
/*Input      : scale value, if value is not in 1 to 2, the return fail            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_display_scale(INT32_T scale)
{
	INT32_T ret = IPANEL_OK ;
	bool  bRet ; 
	OSD_DISPLAY_AR display_ar ;

    ipanel_porting_dprintf("[ipanel_set_display_scale] scale = %d.\n",scale);

	/*set physical TV SCREEN scale here*/
	if( IPANEL_DIS_AR_43 == scale )
	{
		display_ar = OSD_DISPLAY_AR_43 ;		
	}
	else if(IPANEL_DIS_AR_169 == scale) 
	{
		display_ar = OSD_DISPLAY_AR_169 ;		
	}

	bRet = SetDisplayAR(display_ar, TRUE);
	if( FALSE == bRet )
	{
		ret = IPANEL_ERR ;
		ipanel_porting_dprintf("[ipanel_set_display_scale]SetDisplayAR failed!\n");
	}

	return ret;
}

/**********************************************************************************/
/*Description: Set TV screen scale. 1 - width:height=4:3, 2 - width:height=16:9   */
/*Input      : scale value, if value is not in 1 to 2, the return fail            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_aspect_ratio(INT32_T aRatio)
{
	INT32_T ret = IPANEL_ERR ; 
	OSD_AR_MODE AspectConversionMode ; 
    OSD_DISPLAY_AR aspect_ratio;
    bool  bRet ; 

    ipanel_porting_dprintf("[ipanel_set_aspect_ratio] aRatio = %d.\n",aRatio);

	if (IPANEL_DIS_AR_PILLARBOX == aRatio) /* 4 : 3 */
	{
		AspectConversionMode = OSD_ARM_LETTERBOX;
		aspect_ratio = OSD_DISPLAY_AR_43;
	}
	else if(IPANEL_DIS_AR_PAN_SCAN == aRatio) /* 16 : 9 */
	{
		AspectConversionMode = OSD_ARM_LETTERBOX;
		aspect_ratio = OSD_DISPLAY_AR_43;	
	}
	else if(IPANEL_DIS_AR_FULL_SCREEN == aRatio) /* �Զ� full screen */
	{
		AspectConversionMode = OSD_ARM_LETTERBOX;
		aspect_ratio = OSD_DISPLAY_AR_43;
	}
    else if(IPANEL_DIS_AR_LETTERBOX == aRatio)
    {
		AspectConversionMode = OSD_ARM_PANSCAN;
		aspect_ratio = OSD_DISPLAY_AR_169;
    }
	
	bRet = SetMpgAR(AspectConversionMode,TRUE);
	if( FALSE == bRet )
	{
		ret = IPANEL_ERR ;
		ipanel_porting_dprintf("[ipanel_set_aspect_ratio]SetMpgAR failed!\n");
	}

    bRet = SetDisplayAR(aspect_ratio, TRUE);
	if( FALSE == bRet )
	{
		ret = IPANEL_ERR ;
		ipanel_porting_dprintf("[ipanel_set_aspect_ratio]SetDisplayAR failed!\n");
	}

	return ret;
}


/**********************************************************************************/
/*Description: Set TV screen area to play. (0, 0, 0, 0) is full srceen            */
/*Input      : TV screen area x pixls, y pixls, w width, and h height.            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T  ipanel_set_win_location(IPANEL_RECT *rect)
{
	INT32_T  ret = IPANEL_ERR;
	OSDRECT src, dest;
	int x,y,w,h ; 

	x = rect->x; 
	y = rect->y; 
	h = rect->h; 
    w = rect->w; 
#if 0
	s_last_rect.x = rect->x; 
	s_last_rect.y = rect->y; 
	s_last_rect.h = rect->h; 
    s_last_rect.w = rect->w; 
#endif

	ipanel_porting_dprintf("[ipanel_porting_video_location] x=%d, y=%d, w=%d, h=%d\n", x, y, w, h);		

	if ( (x>=0 && x<=IPANEL_DEST_PAL_WIDTH) && (y>=0 && y<=IPANEL_DEST_PAL_HEIGHT)
		&& (w>=0 && w<=IPANEL_DEST_PAL_WIDTH) && (h>=0 && h<=IPANEL_DEST_PAL_HEIGHT) ) 
	{
		if (x==0 && y==0 && w==0 && h==0)
		{
            dest.left   = 15;
            dest.top    = 0;
			dest.right  = IPANEL_DEST_PAL_WIDTH ;
			dest.bottom = IPANEL_DEST_PAL_HEIGHT ;
		}
		else 
		{
			dest.left = x+40;
			dest.right = (dest.left + w + 15);
			dest.top = y+30;
			dest.bottom = (dest.top+h);	
		}

        /* ��������ʱ�������������� */
    	if(VIDEO_START_STATUS == ipanel_get_video_status())
    	{
    		gen_video_unblank_internal();
    	}
    	else
    	{
    		gen_video_blank( NULL );
    	}

		src.top     = 0;
		src.left    = 0;
		src.right   = IPANEL_DEST_PAL_WIDTH;
		src.bottom  = IPANEL_DEST_PAL_HEIGHT;

		ret = vidSetPos(vidGetVideoRegion(), &dest, &src);
		if (!ret)
		{
            ipanel_porting_dprintf("[ipanel_video_location] vidSetPos failed\n");
            return ret;
       	}

       	ret = IPANEL_OK;
	}

	return ret;
}

/**********************************************************************************/
/*Description: Set still picture area to display. (0, 0, 0, 0) is full srceen            */
/*Input      : TV screen area x pixls, y pixls, w width, and h height.            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
INT32_T ipanel_set_still_win_location(IPANEL_RECT *rect)
{
	INT32_T  ret = IPANEL_ERR;
	OSDRECT src, dest;
	int x,y,w,h ; 

	x = rect->x ; 
	y = rect->y ; 
	h = rect->h ; 
    w = rect->w ; 
	
	ipanel_porting_dprintf("[ipanel_set_still_win_location] x=%d, y=%d, w=%d, h=%d\n", x, y, w, h);		

	if ( (x>=0 && x<=IPANEL_DEST_PAL_WIDTH) && (y>=0 && y<=IPANEL_DEST_PAL_HEIGHT)
		&& (w>=0 && w<=IPANEL_DEST_PAL_WIDTH) && (h>=0 && h<=IPANEL_DEST_PAL_HEIGHT) ) 
	{
		if (x==0 && y==0 && w==0 && h==0)
		{
    		dest.left = 15;
    		dest.top = 0;
			dest.right      = IPANEL_DEST_PAL_WIDTH ;
			dest.bottom = IPANEL_DEST_PAL_HEIGHT ;
		}
		else 
		{
			dest.left = x+40;
			dest.right = dest.left+w + 15;
			dest.top = y+30;
			dest.bottom = dest.top+h;	
		}

		src.top = 0;
		src.left = 0;
		src.right  = IPANEL_DEST_PAL_WIDTH;
		src.bottom = IPANEL_DEST_PAL_HEIGHT;

		ret = vidSetPos(vidGetVideoRegion(), &dest, &src);
		if (!ret)
		{
			ipanel_porting_dprintf("[ipanel_set_still_win_location] vidSetPos failed\n");
			return ret;
     	}

    	ret = IPANEL_OK;
	}

	return ret;
}

/*****************************************************************************************/
/*Description: Set TV picture transparent . 0 -no color, 100 - colorest, between 0 to 100*/
/*Input      : picture color value, if value is not in 0 to 100, the return fail         */
/*Output     : No                                                                        */
/*Return     : 0 -- success, -1 -- fail                                                  */
/*****************************************************************************************/
static INT32_T ipanel_set_win_transparent(INT32_T transparent)
{
	INT32_T ret = IPANEL_ERR;
	
	if (transparent >= 0 && transparent <= 100) 
	{
		ret = IPANEL_OK;
	}

	return ret;
}

/**************************************************************************************/
/*Description: Set TV picture contrast . 0 -no color, 100 - colorest, between 0 to 100*/
/*Input      : picture color value, if value is not in 0 to 100, the return fail      */
/*Output     : No                                                                     */
/*Return     : 0 -- success, -1 -- fail                                               */
/**************************************************************************************/
static INT32_T ipanel_set_picture_contrast(INT32_T contrast)
{
	INT32_T ret = IPANEL_ERR;
	CNXT_TVENC_STATUS retcode ; 
	long tvContrast ; 
	
	if (contrast >= 0 && contrast <= 100) 
	{
		/*set physical TV picture contrast here*/
		// The range of it is [-128, 127] <== (0-100)
		if( contrast < 50)
		{
			tvContrast = (128*contrast)/50 - 128 ; 
		}
		else
		{
			tvContrast = (127*contrast)/50 - 127 ; 			
		}			
	
		retcode = cnxt_tvenc_set_picture_control(m_TvencHandle,
                                                 CNXT_TVENC_CONTRAST,
                                                 tvContrast);
		if(retcode != CNXT_TVENC_OK)
		{
			ipanel_porting_dprintf("[ipanel_set_picture_contrast]cnxt_tvenc_set_picture_control set failed!\n");
			return ret ; 
		}
	
		ret = IPANEL_OK;
	}
	
	return ret;
}

/**********************************************************************************/
/*Description: Set TV picture hue . 0 -no color, 100 - colorest, between 0 to 100 */
/*Input      : picture color value, if value is not in 0 to 100, the return fail  */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_picture_hue(INT32_T hue)
{
	INT32_T ret = IPANEL_ERR;
	CNXT_TVENC_STATUS retcode ; 
	long tvHue ; 
	
	if (hue >= 0 && hue <= 100) 
	{
		/*set physical TV picture hue here*/
		// The range of it is [-128, 127] <== (0-100)
		if( hue < 50)
		{
			tvHue = (128*hue)/50 - 128 ; 
		}
		else
		{
			tvHue = (127*hue)/50 - 127 ; 			
		}			
	
		retcode = cnxt_tvenc_set_picture_control(m_TvencHandle,
                                                 CNXT_TVENC_HUE,
                                                 tvHue);
		if(retcode != CNXT_TVENC_OK)
		{
			ipanel_porting_dprintf("[ipanel_set_picture_hue]cnxt_tvenc_set_picture_control set failed!\n");
			return ret ; 
		}
		
		ret = IPANEL_OK;
	}
	
	return ret;
}

/**********************************************************************************/
/*Description: Set TV picture brightness. 0-black, 100-brightest, between 0 to 100*/
/*Input      : picture brightness, if value is not in 0 to 100, the return fail   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_picture_brightness(INT32_T bright)
{
	INT32_T ret = IPANEL_ERR;
	CNXT_TVENC_STATUS retcode ; 
	long tvBright ; 
		
	if (bright >= 0 && bright <= 100) 
	{	
		/*set physical TV picture brightness here*/	
		// The range of it is [-128, 127] <== (0-100)
		if( bright < 50)
		{
			tvBright = (128*bright)/50 - 128 ; 
		}
		else
		{
			tvBright = (127*bright)/50 - 127 ; 			
		}			
	
		retcode = cnxt_tvenc_set_picture_control(m_TvencHandle,
                                                 CNXT_TVENC_BRIGHTNESS,
                                                 tvBright);
		if(retcode != CNXT_TVENC_OK)
		{
			ipanel_porting_dprintf("[ipanel_set_picture_brightness]cnxt_tvenc_set_picture_control set failed!\n");
			return ret ; 
		}
			
		ret = IPANEL_OK;
	}
	
	return ret;
}
/**********************************************************************************/
/*Description: Set TV picture shturation. 0-black, 100-brightest, between 0 to 100*/
/*Input      : picture brightness, if value is not in 0 to 100, the return fail   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_picture_saturation(INT32_T saturation)
{
	INT32_T ret = IPANEL_ERR;
	CNXT_TVENC_STATUS retcode ; 
	long tvSaturation ; 
	
	if (saturation >= 0 && saturation <= 100) 
	{
		/*set physical TV picture sharpness here*/				
		// The range of it is [-128, 127] <== (0-100)
		if( saturation < 50)
		{
			tvSaturation = (128*saturation)/50 - 128 ; 
		}
		else
		{
			tvSaturation = (127*saturation)/50 - 127 ; 			
		}			
	
		retcode = cnxt_tvenc_set_picture_control(m_TvencHandle,
                                                 CNXT_TVENC_SATURATION,
                                                 tvSaturation);
		if(retcode != CNXT_TVENC_OK)
		{
			ipanel_porting_dprintf("[ipanel_set_picture_saturation]cnxt_tvenc_set_picture_control set failed!\n");
			return ret ; 
		}
		
		ret = IPANEL_OK;
	}
	
	return ret;
}

/**********************************************************************************/
/*Description: Set TV picture sharpness. 0-black, 100-brightest, between 0 to 100 */
/*Input      : picture brightness, if value is not in 0 to 100, the return fail   */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail                                           */
/**********************************************************************************/
static INT32_T ipanel_set_picture_sharpness(INT32_T sharpness)
{
	INT32_T ret = IPANEL_ERR;
	CNXT_TVENC_STATUS retcode ; 
	long tvSharpness ; 
	
	if (sharpness >= 0 && sharpness <= 100) 
	{
		/*set physical TV picture sharpness here*/				
		// The range of it is [-128, 127] <== (0-100)
		if( sharpness < 50)
		{
			tvSharpness = (128*sharpness)/50 - 128 ; 
		}
		else
		{
			tvSharpness = (127*sharpness)/50 - 127 ; 			
		}			
	
		retcode = cnxt_tvenc_set_picture_control(m_TvencHandle,
                                                 CNXT_TVENC_SHARPNESS,
                                                 tvSharpness);
		if(retcode != CNXT_TVENC_OK)
		{
			ipanel_porting_dprintf("[ipanel_set_picture_sharpness]cnxt_tvenc_set_picture_control set failed!\n");
			return ret ; 
		}
		
		ret = IPANEL_OK;
	}
	
	return ret;
}

INT32_T ipanel_set_encoder_output()
{
    static u_int8 oConnection;

    //disable encoder
    cnxt_tvenc_get_output_connection(m_TvencHandle, &oConnection);

    /* disconnect current output */
    cnxt_tvenc_set_output_connection(m_TvencHandle, 0);

    return IPANEL_OK;
}

/***************************************************************************************************
���ܣ���һ����ʾ��Ԫ
ԭ�ͣ�UINT32_T ipanel_porting_display_open(VOID)
����˵����

  �����������

  �����������

��    �أ�

  != IPANEL_NULL���ɹ�����Ƶ�����������

  == IPANEL_NULL��ʧ��

***************************************************************************************************/
UINT32_T ipanel_porting_display_open(VOID)
{
    UINT32_T display = 0xD0D0;

    ipanel_porting_dprintf("[ipanel_porting_display_open] is called!\n");

    return display;
}

/***************************************************************************************************
���ܣ��ر�һ����ʾ����Ԫ
ԭ�ͣ�INT32_T ipanel_porting_display_close(UINT32_T display)
����˵����

  �����������ʾ����Ԫ���

  �����������

��    �أ�

  IPANEL_OK���ɹ�

  IPANEL_ERR��ʧ��

***************************************************************************************************/
INT32_T ipanel_porting_display_close(UINT32_T display)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_display_close] display=0x%x\n", display);

    return ret;
}

/***************************************************************************************************
���ܣ���ָ������ʾ����Ԫ�ϵĴ���һ������
ԭ�ͣ�UINT32_T ipanel_porting_display_open_window(UINT32_T display, INT32_T type)
����˵����

  ���������
    display�� һ����ʾ��Ԫ������

    type��  ���ڵ����ͣ�ֻ֧��0����ʾ��Ƶ���ڡ�

  �����������

��    �أ�

  != IPANEL_NULL���ɹ�����Ƶ�����������

  == IPANEL_NULL��ʧ��

***************************************************************************************************/
UINT32_T ipanel_porting_display_open_window(UINT32_T display, INT32_T type)
{
    UINT32_T window = 0xE0E0;

    ipanel_porting_dprintf("[ipanel_porting_display_open_window] display=0x%x, \
                            type=%d\n", display, type);

    return window;
}

/***************************************************************************************************
���ܣ��ر�ָ���Ĵ���
ԭ�ͣ�INT32_T ipanel_porting_display_close_window(UINT32_T display, UINT32_T window)
����˵����

  ���������
    display: ��ʾ��Ԫ��������

    window�� Ҫ�رյĴ��ھ����

  �����������

��    �أ�

  IPANEL_OK���ɹ�

  IPANEL_ERR��ʧ��

***************************************************************************************************/
INT32_T ipanel_porting_display_close_window(UINT32_T display, UINT32_T window)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_display_close_window] display=0x%x, \
                            window=%d\n", display, window);

    return ret;
}


/***************************************************************************************************
���ܣ�����ʾ�豸����һ�������������������úͻ�ȡ��ʾ�豸�Ĳ���������
ԭ�ͣ�INT32_T ipanel_porting_display_ioctl(UINT32_T display, IPANEL_DIS_IOCTL_e op, VOID *arg)
����˵����

  ���������

    display����ʾ����Ԫ���

    op :  ��������

      typedef enum

      {

        IPANEL_DIS_SELECT_DEV =1,

        IPANEL_DIS_ENABLE_DEV,

        IPANEL_DIS_SET_MODE,

        IPANEL_DIS_SET_VISABLE,

        IPANEL_DIS_SET_ASPECT_RATIO,

        IPANEL_DIS_SET_WIN_LOCATION,

        IPANEL_DIS_SET_WIN_TRANSPARENT,

        IPANEL_DIS_SET_CONTRAST,

        IPANEL_DIS_SET_HUE,

        IPANEL_DIS_SET_BRIGHTNESS,

        IPANEL_DIS_SET_SATURATION,

        IPANEL_DIS_SET_SHARPNESS,

        } IPANEL_DIS_IOCTL_e;

    arg -- �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��arg��ǿ��ת���ɶ�Ӧ�������͡�

    op, argȡֵ���±�

    +---------------------+---------------------------------+-----------------------------+
    |  op                 |   arg                           |  ˵��                       |
    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum  {                  |������Ƶ����豸.��������:�� |
    |   SELECT_DEV        | IPANEL_VIDEO_OUTPUT_CVBS= 1<<0, |  ʾ����Ԫ                 |
    |                     | IPANEL_VIDEO_OUTPUT_SVIDEO=1<<1,|                             |
    |                     | IPANEL_VIDEO_OUTPUT_RGB= 1<<2,  |                             |
    |                     | IPANEL_VIDEO_OUTPUT_YPBPR=1<<3, |                             |
    |                     | IPANEL_VIDEO_OUTPUT_HDMI=1<<4   |                             |
    |                     |} IPANEL_VDIS_VIDEO_OUTPUT_e;    |                             |

    |                     |arg��ֵ����������ֵ�Ļ�.         |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |��IPANEL_VDIS_VIDEO_OUTPUT_e����,|�����Ƿ�����Ӧ�Ľӿ��豸���� |
    |   ENABLE_DEV        |��ӦλΪ0��ʾֹͣ���,Ϊ1��ʾ����|���źš�����������ʾ���� |
    |                     |���.                            |Ԫ.                          |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_SET_MODE |typedef enum {                   |������Ƶ�����ʽ(display����)|
    |                     | IPANEL_DIS_TVENC_NTSC,          |                             |
    |                     | IPANEL_DIS_TVENC_PAL,           |����������ʾ����Ԫ.      |
    |                     | IPANEL_DIS_TVENC_SECAM,         |                             |
    |                     | IPANEL_DIS_TVENC_AUTO           |                             |
    |                     |} IPANEL_DIS_TVENC_MODE_e;       |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum {                   |������Ƶ���ڵĿɼ���.        |
    |   SET_VISABL        | IPANEL_DISABLE,                 |                             |
    |                     | IPANEL_ENABLE                   |����������ʾ����.          |
    |                     |} IPANEL_SWITCH_e;               |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum {                   |������Ƶ���ݺ�Ⱥ���ʾ�豸�� |
    |   SET_ASPECT_RATIO  | IPANEL_DIS_AR_FULL_SCREEN,      |��ȵĶ�Ӧ��ʽ.              |
    |                     | IPANEL_DIS_AR_PILLARBOX,        |����������ʾ����.          |
    |                     | IPANEL_DIS_AR_LETTERBOX,        |                             |
    |                     | IPANEL_DIS_AR_PAN_SCAN          |                             |
    |                     |} IPANEL_DIS_AR_MODE_e;          |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |ָ��һ���ṹ: typedef struct {   |������Ƶ���ڵ�λ�á�         |
    |   SET_WIN_LOCATION  | UINT32_T x; //��ʼ��ĺ�����    |                             |
    |                     | UINT32_T y; //��ʼ���������    |����������ʾ����.          |
    |                     | UINT32_T w; //��Ƶ����Ŀ�      |                             |
    |                     | UINT32_T h; //��Ƶ����ĸ�      |                             |
    |                     |} IPANEL_RECT;                   |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_SET_     |0 ~ 100֮�������ֵ.             |����͸����,͸���̶�.0-��С,��|
    |   WIN_TRANSPARENT   |                                 |ȫ�ɼ�;100-���,��ȫ���ɼ�.  |
    |                     |                                 |����������ʾ����.          |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100֮�������ֵ.             |����ͼ��Աȶ�,�԰ٷֱȱ�ʾ. |
    |   SET_CONTRAST      |                                 |                             |
    |                     |                                 |����������ʾ����Ԫ.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100֮�������ֵ.             |����ͼ��ɫ��,�԰ٷֱȱ�ʾ.   |
    |   SET_HUE           |                                 |                             |
    |                     |                                 |����������ʾ����Ԫ.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100֮�������ֵ.             |����ͼ������,�԰ٷֱȱ�ʾ.   |
    |   SET_BRIGHTNESS    |                                 |                             |
    |                     |                                 |����������ʾ����Ԫ.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100֮�������ֵ.             |����ͼ�񱥺Ͷ�,�԰ٷֱȱ�ʾ. |
    |   SET_SATURATION    |                                 |                             |
    |                     |                                 |����������ʾ����Ԫ.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100֮�������ֵ.             |����ͼ����ȶ�,�԰ٷֱȱ�ʾ. |
    |   SET_SHARPNESS     |                                 |                             |
    |                     |                                 |����������ʾ����Ԫ.      |

    +---------------------+---------------------------------+-----------------------------+
  �����������

��    �أ�

  IPANEL_OK���ɹ�

  IPANEL_ERR��ʧ��
***************************************************************************************************/
INT32_T ipanel_porting_display_ioctl(UINT32_T display, IPANEL_DIS_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    INT32_T oparg = (INT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_display_ioctl] display=0x%x, \
                            op=%d, arg=%p\n", display, op, arg);

    switch (op)
    {
        case IPANEL_DIS_SELECT_DEV :                    /* ������Ƶ����豸*/
            break;

        case IPANEL_DIS_ENABLE_DEV :                    /* �����Ƿ�����Ӧ�Ľӿ��豸������ź�*/
            break;

        case IPANEL_DIS_SET_MODE :                      /* ������Ƶ�����ʽ*/
	     	ret = ipanel_set_video_mode(oparg);
            break;

        case IPANEL_DIS_SET_VISABLE :                   /* ������Ƶ���ڵĿɼ���*/
	     	ret = ipanel_set_video_visable(oparg);
            break;

        case IPANEL_DIS_SET_ASPECT_RATIO :
	     	ret = ipanel_set_aspect_ratio(oparg);          /* ������Ƶ���ݺ�Ⱥ���ʾ�豸�ݺ�� */
            break;

        case IPANEL_DIS_SET_WIN_LOCATION :              /* ������Ƶ���ڵ� */
	     	ret = ipanel_set_win_location((IPANEL_RECT*)arg);			
            break;

        case IPANEL_DIS_SET_WIN_TRANSPARENT :           /* ����͸����*/
	     	ret = ipanel_set_win_transparent(oparg);
            break;

        case IPANEL_DIS_SET_CONTRAST :                  /* ����ͼ��Աȶ�*/
	     	//ret = ipanel_set_picture_contrast(oparg);
            break;

        case IPANEL_DIS_SET_HUE :                       /* ����ͼ��ɫ��*/
	     	//ret = ipanel_set_picture_hue(oparg);
            break;

        case IPANEL_DIS_SET_BRIGHTNESS :                /* ����ͼ������ */
	     	//ret = ipanel_set_picture_brightness(oparg);
            break;

        case IPANEL_DIS_SET_SATURATION :                /* ����ͼ�񱥺Ͷ�*/
	     	//ret = ipanel_set_picture_saturation(oparg);
            break;

        case IPANEL_DIS_SET_SHARPNESS :                 /* ����ͼ����� */
	     	//ret = ipanel_set_picture_sharpness(oparg);
            break;

        case IPANEL_DIS_SET_HD_RES :                    /* ���ø�������ӿ��ϵ���ʾģʽ���ֱ��ʣ�*/
            break;

        case IPANEL_DIS_SET_IFRAME_LOCATION :           /* ָ��IFRAEM���ڵ�λ�úʹ�С�� */
			ret = ipanel_set_still_win_location((IPANEL_RECT*)arg);			
            break;

        default :
            ipanel_porting_dprintf("[ipanel_porting_display_ioctl] ERROR parameter!\n");
            ret = IPANEL_ERR;
    }

    return ret;
}

INT32_T ipanel_video_tvenc_init()
{
	INT32_T ret = IPANEL_ERR ; 
	CNXT_TVENC_CAPS tvencCaps;
	CNXT_TVENC_STATUS retcode;

	 /* Initialize the video encoder */
	/* Initialize the TV encoder driver*/
	tvencCaps.uLength = sizeof(CNXT_TVENC_CAPS);

	/* abtain a handle to the TV encoder driver */
	retcode = cnxt_tvenc_get_unit_caps( 0, &tvencCaps );
	if( retcode != CNXT_TVENC_OK )
	{
	   	ipanel_porting_dprintf("[ipanel_video_tvenc_init]cnxt_tvenc_get_unit_caps failed. \n");
    	return ret;
	}
	
	retcode = cnxt_tvenc_open( &m_TvencHandle, &tvencCaps, 0, 0 );
	if( retcode != CNXT_TVENC_OK )
	{
    	ipanel_porting_dprintf("[ipanel_video_tvenc_init]cnxt_tvenc_open failed. \n");
    	return ret;
	}

	return IPANEL_OK;
}

VOID ipanel_video_tvenc_exit()
{
	CNXT_TVENC_STATUS retcode;

	retcode = cnxt_tvenc_close(m_TvencHandle);
	if( retcode != CNXT_TVENC_OK)
	{
    	ipanel_porting_dprintf("[ipanel_video_tvenc_init]cnxt_tvenc_close failed. \n");
	}
}

