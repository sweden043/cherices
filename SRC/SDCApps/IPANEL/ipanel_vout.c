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
	else if(IPANEL_DIS_AR_FULL_SCREEN == aRatio) /* 自动 full screen */
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

        /* 用于启动时清除最下面的绿条 */
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
功能：打开一个显示单元
原型：UINT32_T ipanel_porting_display_open(VOID)
参数说明：

  输入参数：无

  输出参数：无

返    回：

  != IPANEL_NULL：成功，视频解码器句柄；

  == IPANEL_NULL：失败

***************************************************************************************************/
UINT32_T ipanel_porting_display_open(VOID)
{
    UINT32_T display = 0xD0D0;

    ipanel_porting_dprintf("[ipanel_porting_display_open] is called!\n");

    return display;
}

/***************************************************************************************************
功能：关闭一个显示管理单元
原型：INT32_T ipanel_porting_display_close(UINT32_T display)
参数说明：

  输入参数：显示管理单元句柄

  输出参数：无

返    回：

  IPANEL_OK：成功

  IPANEL_ERR：失败

***************************************************************************************************/
INT32_T ipanel_porting_display_close(UINT32_T display)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_display_close] display=0x%x\n", display);

    return ret;
}

/***************************************************************************************************
功能：在指定的显示管理单元上的创建一个窗口
原型：UINT32_T ipanel_porting_display_open_window(UINT32_T display, INT32_T type)
参数说明：

  输入参数：
    display： 一个显示单元管理句柄

    type：  窗口的类型，只支持0，表示视频窗口。

  输出参数：无

返    回：

  != IPANEL_NULL：成功，视频解码器句柄；

  == IPANEL_NULL：失败

***************************************************************************************************/
UINT32_T ipanel_porting_display_open_window(UINT32_T display, INT32_T type)
{
    UINT32_T window = 0xE0E0;

    ipanel_porting_dprintf("[ipanel_porting_display_open_window] display=0x%x, \
                            type=%d\n", display, type);

    return window;
}

/***************************************************************************************************
功能：关闭指定的窗口
原型：INT32_T ipanel_porting_display_close_window(UINT32_T display, UINT32_T window)
参数说明：

  输入参数：
    display: 显示单元管理句柄。

    window： 要关闭的窗口句柄。

  输出参数：无

返    回：

  IPANEL_OK：成功

  IPANEL_ERR：失败

***************************************************************************************************/
INT32_T ipanel_porting_display_close_window(UINT32_T display, UINT32_T window)
{
    INT32_T ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_display_close_window] display=0x%x, \
                            window=%d\n", display, window);

    return ret;
}


/***************************************************************************************************
功能：对显示设备进行一个操作，或者用于设置和获取显示设备的参数和属性
原型：INT32_T ipanel_porting_display_ioctl(UINT32_T display, IPANEL_DIS_IOCTL_e op, VOID *arg)
参数说明：

  输入参数：

    display：显示管理单元句柄

    op :  操作命令

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

    arg -- 操作命令所带的参数，当传递枚举型或32位整数值时，arg可强制转换成对应数据类型。

    op, arg取值见下表：

    +---------------------+---------------------------------+-----------------------------+
    |  op                 |   arg                           |  说明                       |
    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum  {                  |设置视频输出设备.操作对象:显 |
    |   SELECT_DEV        | IPANEL_VIDEO_OUTPUT_CVBS= 1<<0, |  示管理单元                 |
    |                     | IPANEL_VIDEO_OUTPUT_SVIDEO=1<<1,|                             |
    |                     | IPANEL_VIDEO_OUTPUT_RGB= 1<<2,  |                             |
    |                     | IPANEL_VIDEO_OUTPUT_YPBPR=1<<3, |                             |
    |                     | IPANEL_VIDEO_OUTPUT_HDMI=1<<4   |                             |
    |                     |} IPANEL_VDIS_VIDEO_OUTPUT_e;    |                             |

    |                     |arg的值可以是以上值的或.         |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |见IPANEL_VDIS_VIDEO_OUTPUT_e定义,|控制是否在相应的接口设备上输 |
    |   ENABLE_DEV        |相应位为0表示停止输出,为1表示允许|出信号。操作对象：显示管理单 |
    |                     |输出.                            |元.                          |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_SET_MODE |typedef enum {                   |设置视频输出制式(display属性)|
    |                     | IPANEL_DIS_TVENC_NTSC,          |                             |
    |                     | IPANEL_DIS_TVENC_PAL,           |操作对象：显示管理单元.      |
    |                     | IPANEL_DIS_TVENC_SECAM,         |                             |
    |                     | IPANEL_DIS_TVENC_AUTO           |                             |
    |                     |} IPANEL_DIS_TVENC_MODE_e;       |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum {                   |设置视频窗口的可见性.        |
    |   SET_VISABL        | IPANEL_DISABLE,                 |                             |
    |                     | IPANEL_ENABLE                   |操作对象：显示窗口.          |
    |                     |} IPANEL_SWITCH_e;               |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |typedef enum {                   |设置视频的纵横比和显示设备纵 |
    |   SET_ASPECT_RATIO  | IPANEL_DIS_AR_FULL_SCREEN,      |横比的对应方式.              |
    |                     | IPANEL_DIS_AR_PILLARBOX,        |操作对象：显示窗口.          |
    |                     | IPANEL_DIS_AR_LETTERBOX,        |                             |
    |                     | IPANEL_DIS_AR_PAN_SCAN          |                             |
    |                     |} IPANEL_DIS_AR_MODE_e;          |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |指向一个结构: typedef struct {   |设置视频窗口的位置。         |
    |   SET_WIN_LOCATION  | UINT32_T x; //起始点的横坐标    |                             |
    |                     | UINT32_T y; //起始点的纵坐标    |操作对象：显示窗口.          |
    |                     | UINT32_T w; //视频区域的宽      |                             |
    |                     | UINT32_T h; //视频区域的高      |                             |
    |                     |} IPANEL_RECT;                   |                             |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_SET_     |0 ~ 100之间的整数值.             |设置透明度,透明程度.0-最小,完|
    |   WIN_TRANSPARENT   |                                 |全可见;100-最大,完全不可见.  |
    |                     |                                 |操作对象：显示窗口.          |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100之间的整数值.             |设置图像对比度,以百分比表示. |
    |   SET_CONTRAST      |                                 |                             |
    |                     |                                 |操作对象：显示管理单元.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100之间的整数值.             |设置图像色调,以百分比表示.   |
    |   SET_HUE           |                                 |                             |
    |                     |                                 |操作对象：显示管理单元.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100之间的整数值.             |设置图像亮度,以百分比表示.   |
    |   SET_BRIGHTNESS    |                                 |                             |
    |                     |                                 |操作对象：显示管理单元.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100之间的整数值.             |设置图像饱和度,以百分比表示. |
    |   SET_SATURATION    |                                 |                             |
    |                     |                                 |操作对象：显示管理单元.      |

    +---------------------+---------------------------------+-----------------------------+
    | IPANEL_DIS_         |0 ~ 100之间的整数值.             |设置图像锐度度,以百分比表示. |
    |   SET_SHARPNESS     |                                 |                             |
    |                     |                                 |操作对象：显示管理单元.      |

    +---------------------+---------------------------------+-----------------------------+
  输出参数：无

返    回：

  IPANEL_OK：成功

  IPANEL_ERR：失败
***************************************************************************************************/
INT32_T ipanel_porting_display_ioctl(UINT32_T display, IPANEL_DIS_IOCTL_e op, VOID *arg)
{
    INT32_T ret = IPANEL_OK;
    INT32_T oparg = (INT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_display_ioctl] display=0x%x, \
                            op=%d, arg=%p\n", display, op, arg);

    switch (op)
    {
        case IPANEL_DIS_SELECT_DEV :                    /* 设置视频输出设备*/
            break;

        case IPANEL_DIS_ENABLE_DEV :                    /* 控制是否在相应的接口设备上输出信号*/
            break;

        case IPANEL_DIS_SET_MODE :                      /* 设置视频输出制式*/
	     	ret = ipanel_set_video_mode(oparg);
            break;

        case IPANEL_DIS_SET_VISABLE :                   /* 设置视频窗口的可见性*/
	     	ret = ipanel_set_video_visable(oparg);
            break;

        case IPANEL_DIS_SET_ASPECT_RATIO :
	     	ret = ipanel_set_aspect_ratio(oparg);          /* 设置视频的纵横比和显示设备纵横比 */
            break;

        case IPANEL_DIS_SET_WIN_LOCATION :              /* 设置视频窗口的 */
	     	ret = ipanel_set_win_location((IPANEL_RECT*)arg);			
            break;

        case IPANEL_DIS_SET_WIN_TRANSPARENT :           /* 设置透明度*/
	     	ret = ipanel_set_win_transparent(oparg);
            break;

        case IPANEL_DIS_SET_CONTRAST :                  /* 设置图像对比度*/
	     	//ret = ipanel_set_picture_contrast(oparg);
            break;

        case IPANEL_DIS_SET_HUE :                       /* 设置图像色调*/
	     	//ret = ipanel_set_picture_hue(oparg);
            break;

        case IPANEL_DIS_SET_BRIGHTNESS :                /* 设置图像亮度 */
	     	//ret = ipanel_set_picture_brightness(oparg);
            break;

        case IPANEL_DIS_SET_SATURATION :                /* 设置图像饱和度*/
	     	//ret = ipanel_set_picture_saturation(oparg);
            break;

        case IPANEL_DIS_SET_SHARPNESS :                 /* 设置图像锐度 */
	     	//ret = ipanel_set_picture_sharpness(oparg);
            break;

        case IPANEL_DIS_SET_HD_RES :                    /* 设置高清输出接口上的显示模式（分辨率）*/
            break;

        case IPANEL_DIS_SET_IFRAME_LOCATION :           /* 指定IFRAEM窗口的位置和大小。 */
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

