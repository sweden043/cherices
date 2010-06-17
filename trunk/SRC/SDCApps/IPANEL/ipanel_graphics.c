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
#include "ipanel_config.h"
#include "ipanel_os.h"
#include "ipanel_base.h"
#include "ipanel_graphics.h"

static unsigned char    *m_src_buffer = NULL;
static GFX_BITMAP       m_SrcBitmap, m_DstBitmap;
static GFX_RECT         m_SrcRect;
static GFX_XY           m_DstPt;
static GFX_OP           m_Opcode;
static OSDHANDLE        m_osd_handle;

/******************************************************************
    ����˵����
    ��װ��ɫ�壬iPanel MiddleWare Ҫ���ɫ���е�0 ��ɫΪ͸��ɫ��
    �������ʽС�ڵ���8bppʱ����Ҫʵ�ָú�����������ʽʱ�ú���Ӧ�ÿա�

    ����˵����
        ���������
        npals����ɫ���С��8 λ��ɫʱ256��
        pal[]������ɫ�����ݣ����е���ɫ�������32bit ��
        ���ͣ���ʾ��ɫ��ʽ�ǣ�0x00RRGGBB���ú�������
        IPanel Middleware ϵͳÿ�ο�ʼ����ʱ����һ�Σ�
        ���������й��̵��в��ٵ��á�����8λ����ɫ
        ģʽ����ʹ�õ�ɫ�塣

    ��    �أ�
    IPANEL_OK:�ɹ�;
    IPANEL_ERR:ʧ�ܡ�
******************************************************************/
INT32_T ipanel_porting_graphics_install_palette(UINT32_T *pal,INT32_T npals)
{
    int ret = IPANEL_OK;

    ipanel_porting_dprintf("[ipanel_porting_graphics_install_palette] is called ! npals=%d\n",npals);

    return ret;
}

/******************************************************************************
	������: ipanel_graphics_avail_win_notify
	����:
	    IPANEL_GRAPHICS_WIN_RECT  
		�����ʵ������Ĵ��ڵĴ�С��λ�á�	
	����ֵ:
	    �ɹ�: IPANEL_OK ��ʧ��: IPANEL_ERR 
******************************************************************************/
INT32_T ipanel_porting_graphics_get_info(INT32_T *width,INT32_T *height,
        void **pbuf, INT32_T *buf_width,INT32_T *buf_height)
{
    INT32_T ret = IPANEL_OK ;

#ifdef TEST_OSD_AUTOSCALE
    *width  = *buf_width    = IPANEL_DEST_PAL_WIDTH ;
    *height = *buf_height   = IPANEL_DEST_PAL_HEIGHT;
#else
    *width  = *buf_width    = IPANEL_TV_SCREEN_WIDTH ;
    *height = *buf_height   = IPANEL_TV_SCREEN_HEIGHT;
#endif

#ifdef OSD_MALLOC_SELF
    *pbuf   = m_src_buffer;
#endif

    ipanel_porting_dprintf("[ipanel_porting_graphics_getInfo] W=%d,H=%d,bufW=%d,bufH=%d \n",\
         				   *width,*height,*buf_width,*buf_height);

    return ret;
}

/*******************************************************************************
����˵����
	��������graphics���͸���ȡ�
����˵����
	���������
		alpha��0��100��0Ϊȫ͸��100Ϊ��ȫ��͸����
	���������	�ޡ�

��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�

*******************************************************************************/
INT32_T ipanel_porting_graphics_set_alpha(INT32_T alpha)
{
    static bool  bInit = FALSE;
    INT32_T ret = IPANEL_ERR;
    OSDALPHAFLAG AlphaFlag;

	ipanel_porting_dprintf("[ipanel_porting_graphics_set_alpha] alpha = %d.\n",alpha);
	
    if (alpha >= 0 && alpha <= 100)
    {
        /*set physical OSD alpha here*/
        AlphaFlag = REGION_ALPHA;
        alpha = (alpha*255)/100;
        if(!bInit)
        {
            alpha = 255;
            bInit = TRUE;
        }

        SetOSDRgnOptions((OSDHANDLE)m_osd_handle, OSD_RO_FORCEREGIONALPHA, 1);
        SetOSDRgnAlpha((OSDHANDLE)m_osd_handle, AlphaFlag, alpha, (u_int32)0);

        ret = IPANEL_OK;
    }

    return ret;
}

/*******************************************************************************
����˵����
		��(x,y)���꿪ʼ������Ļ�ϻ�һ�����εĵ���ͼ����Ļ�ϡ�

����˵����
	���������
		x, y, w, h: ��ʾ��������λ�úʹ�С��
		bits: �������������ʼ��ַ��ע��:bits�������������ģ����ӣ�x��y����ʼ��
		      һ���н�����һ���У�w_src���������ο�����Ҫ�����ݲ�һ���������ģ�
		      ���ӣ�x��y����ʼ��ȡ����w�����أ��ٴ���һ�п�ʼ��
		w_src: iPanel Middleware �ڲ�������ÿ�еĿ�ȣ�������Ϊ��λ����
	�����������
��    �أ�
	IPANEL_OK:�ɹ�;
	IPANEL_ERR:ʧ�ܡ�
����˵����
	�ú�����iPanel Middleware �������й��̵���Ƶ�����ã����Ը���OSD ����ʾ����
	������Ҫ�����е�Ч�ʺܸߡ����⣬��ע�⣬x��y �����꣬��ʱ��δ֪����£�
	�ᳬ������ʾ��Χ����ʵ�ִ�����������Խ����жϡ�	
*******************************************************************************/
INT32_T ipanel_porting_graphics_draw_image(INT32_T x, INT32_T y, INT32_T w, INT32_T h,
                                  BYTE_T *bits, INT32_T w_src)
{
    int ret = IPANEL_OK ;
    
#ifndef OSD_MALLOC_SELF
    u_int32 i=0, j=0;
    unsigned char *pSrc = m_src_buffer;
    unsigned char *currPtr = NULL;
    static unsigned char *eisdata = NULL;
    static int  time;
#endif

    //ipanel_porting_dprintf("[ipanel_porting_graphics_draw_image]  x = %d y = %d w =%d h =%d,w_src = %d \n",
    //                         x, y, w, h, w_src);
    
    //time = ipanel_porting_time_ms();

#ifndef OSD_MALLOC_SELF
    if (!eisdata)
    {
        eisdata = bits;
    }

#if (PORTING_COLORFMT == PORTING_ARGB1555)
    currPtr = eisdata;
    currPtr += (y*w_src+x)*2;
    pSrc    += (y*w_src+x)*2;

    /* convert ARGB to ABGR */
    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j++)
        {
            pSrc[0] = (currPtr[0]&0xe0)|((currPtr[1]&0x7f)>>2);
            pSrc[1] = (currPtr[1]&0x83)|((currPtr[0]&0x1f)<<2);

            currPtr += 2;
            pSrc += 2;
        }
        currPtr += ((w_src - w)*2);
        pSrc += ((w_src - w)*2);
    }

#elif (PORTING_COLORFMT == ARGB8888_to_ARGB1555)
    currPtr = eisdata;
    currPtr += (y*w_src+x)*4;
    pSrc += (y*w_src+x)*2;

    /* convert ARGB to ABGR */
    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j++)
        {
            pSrc[0] = (currPtr[0]>>3)&0x1F | (currPtr[1]<<2)&0xE0;
            pSrc[1] = ((currPtr[1]>>6)&0x03 | (currPtr[2]>>1)&0x7C | (currPtr[3]>>7)&0x80);
            currPtr += 4;
            pSrc += 2;
        }
        currPtr += ((w_src - w)*4);
        pSrc += ((w_src - w)*2);
    }

#elif (PORTING_COLORFMT == PORTING_ARGB8888)
    currPtr = eisdata;
    currPtr += (y*w_src+x)*4;
    pSrc += (y*w_src+x)*4 ;

    for (i=0; i<h; i++)
    {
        for (j=0; j<w; j++)
        {
            pSrc[0] = currPtr[0]; //read
            pSrc[1] = currPtr[1]; //green
            pSrc[2] = currPtr[2]; //blue
            pSrc[3] = currPtr[3]; //alpha
            currPtr += 4;
            pSrc += 4;
        }
        currPtr += ((w_src - w)*4);
        pSrc += ((w_src - w)*4);
    }
#else
    ipanel_porting_dprintf("[ipanel_porting_graphics_draw_image] ERROR : unknown color format ! \n");
#endif

#endif

#ifdef USE_PORTING_TRANSPARENT
    {
        unsigned int *ptr =  (unsigned int*)m_SrcBitmap.pBits;

        for(i = 0; i<336640; i++)
        {
            if ( (*ptr) != 0x0 && ((*ptr) & 0x00FFFFFF) == 0x00 ) 
            {
                *ptr = ((*ptr)&0xff000000)|0x00010101 ;
            }
            ptr ++ ;
        }
    }
#endif

    ret = GfxCopyBlt(&m_SrcBitmap, &m_SrcRect, &m_DstBitmap, &m_DstPt, &m_Opcode);
    if ( 0 != ret)
    {
        ipanel_porting_dprintf("[ipanel_porting_graphics_draw_image] GfxCopyBlt failed \n");
        ret = IPANEL_ERR ;
    }
	
    //ipanel_porting_dprintf("[ipanel_porting_graphics_draw_image] tm=%d\n", 
    //                        ipanel_porting_time_ms() - time);

	return ret;
}

/******************************************************************************
    ������: ipanel_graphics_avail_win_notify
    ����:
        IPANEL_GRAPHICS_WIN_RECT
        �����ʵ������Ĵ��ڵĴ�С��λ�á�
    ����ֵ:
        �ɹ�: IPANEL_OK ��ʧ��: IPANEL_ERR
******************************************************************************/
static INT32_T ipanel_graphics_avail_win_notify(IPANEL_GRAPHICS_WIN_RECT *rect)
{
    INT32_T ret = IPANEL_ERR ;
    OSDRECT Rect;
    bool    bOK ;

    ipanel_porting_dprintf("[ipanel_graphics_avail_win_notify] (x,y,w,h).\n",
                                      rect->x,rect->y,rect->w,rect->h);

    Rect.left   = rect->x;
    Rect.top    = rect->y;
    Rect.right  = rect->x + rect->w;
    Rect.bottom = rect->y + rect->h;
    
 /*   bOK = SetOSDRgnRect(m_osd_handle, &Rect);
    if (!bOK)
    {
        ipanel_porting_dprintf("[ipanel_graphics_avail_win_notify] change display rect error!\n");
        return ret ;
    }
*/
    return (ret = IPANEL_OK);
}

/******************************************************************
    ����˵����
        ��Graphics����һ�������������������ú�
        ��ȡGraphics�豸�Ĳ��������ԡ�

    ����˵����
        ���������
            arg - �������������Ĳ�����������ö���ͻ�32λ����ֵʱ��
            arg��ǿ��ת���ɶ�Ӧ�������͡�

    ��    �أ�
    IPANEL_OK:�ɹ�;
    IPANEL_ERR:ʧ�ܡ�
******************************************************************/
INT32_T ipanel_porting_graphics_ioctl(IPANEL_GRAPHICS_IOCTL_e op, VOID *arg)
{
    INT32_T      ret = IPANEL_OK ;
    UINT32_T oparg = (UINT32_T)arg;

    ipanel_porting_dprintf("[ipanel_porting_graphics_ioctl] op = %d , value = %d \n", op, oparg);

    return IPANEL_OK;

    switch(op)
    {
        case IPANEL_GRAPHICS_AVAIL_WIN_NOTIFY:  //֪ͨ�ⲿģ�飬�����ʵ������Ĵ��ڵĴ�С��λ�á�
            ret = ipanel_graphics_avail_win_notify((IPANEL_GRAPHICS_WIN_RECT*)arg);
            break;

        default:
            break;
    }

    return ret ;
}

INT32_T ipanel_osd_show_region()
{
    SetOSDRgnOptions(m_osd_handle,OSD_RO_ENABLE,TRUE);

    return IPANEL_OK;
}

INT32_T ipanel_osd_hide_region()
{
    SetOSDRgnOptions(m_osd_handle,OSD_RO_ENABLE,FALSE);

    return IPANEL_OK;
}

INT32_T ipanel_graphics_init(VOID)
{
    INT32_T ret = IPANEL_ERR ;
    OSDRECT rectOsd;
    u_int32 OsdOption;
    u_int32 dwStride = 0;

#if (PORTING_COLORFMT == PORTING_ARGB1555) || (PORTING_COLORFMT == ARGB8888_to_ARGB1555)

    m_src_buffer = (unsigned char*)ipanel_porting_malloc(IPANEL_DEST_PAL_WIDTH 
                                                       * IPANEL_DEST_PAL_HEIGHT* 2);

    if ( !m_src_buffer )
    {
        ipanel_porting_dprintf("[ipanel_osd_nxp_init] malloc src_buffer failed\n");
        goto OSD_INIT_FAILED;
    }
    memset(m_src_buffer, 0x0, IPANEL_DEST_PAL_WIDTH*IPANEL_DEST_PAL_HEIGHT*2);

    OSDInit();

    rectOsd.top     = 0;
    rectOsd.left    = 0;
    rectOsd.bottom  = IPANEL_DEST_PAL_HEIGHT;
    rectOsd.right   = IPANEL_DEST_PAL_WIDTH;
    dwStride        = 0;
    OsdOption       = OSD_RO_LOADPALETTE|OSD_RO_ALPHABOTHVIDEO|OSD_RO_ALPHAENABLE|
                             OSD_RO_FFENABLE|OSD_RO_ENABLE;

    m_osd_handle = CreateOSDRgn(&rectOsd, OSD_MODE_16ARGB1555, OsdOption, NULL, dwStride);
    if (!m_osd_handle)
    {
        ipanel_porting_dprintf("[ipanel_osd_nxp_init] CreateOSDRgn failed !!!\n");
        goto OSD_INIT_FAILED;
    }

    SetOSDColorKey(0) ;
    SetOSDRgnOptions(m_osd_handle, OSD_RO_COLORKEYENABLE,  TRUE);

    m_SrcBitmap.Version   = 0;
    m_SrcBitmap.VerSize   = 0;
    m_SrcBitmap.Type      = GFX_ARGB16_1555;
    m_SrcBitmap.Bpp       = 16;
#ifdef TEST_OSD_AUTOSCALE
    m_SrcBitmap.Height    = IPANEL_DEST_PAL_HEIGHT;
    m_SrcBitmap.Width     = IPANEL_DEST_PAL_WIDTH;
    m_SrcBitmap.Stride    = IPANEL_DEST_PAL_WIDTH * 2;
#else
    m_SrcBitmap.Height    = IPANEL_TV_SCREEN_HEIGHT;
    m_SrcBitmap.Width     = IPANEL_TV_SCREEN_WIDTH;
    m_SrcBitmap.Stride    = IPANEL_TV_SCREEN_WIDTH * 2;
#endif
    m_SrcBitmap.dwRef     = 0;
    m_SrcBitmap.pPalette  = NULL;
    m_SrcBitmap.pBits     = (void*)m_src_buffer;


    m_DstBitmap.Version   = 0x0001;
    m_DstBitmap.VerSize   = sizeof(GFX_BITMAP);
    m_DstBitmap.Type      = GFX_ARGB16_1555;
    m_DstBitmap.Bpp       = 16;
    m_DstBitmap.Height    = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_BUFFERHEIGHT);
    m_DstBitmap.Width     = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_BUFFERWIDTH);
    m_DstBitmap.Stride    = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_FRAMESTRIDE);
    m_DstBitmap.dwRef     = (u_int32) m_osd_handle;
    m_DstBitmap.pPalette  = (void*) GetOSDRgnOptions(m_osd_handle, OSD_RO_PALETTEADDRESS);
    m_DstBitmap.pBits     = (void*) GetOSDRgnOptions(m_osd_handle, OSD_RO_FRAMEBUFFER);

    m_SrcRect.Left        = 0;
    m_SrcRect.Top         = 0;
    m_SrcRect.Right       = m_SrcBitmap.Width;
    m_SrcRect.Bottom      = m_SrcBitmap.Height;

    m_DstPt.X = IPANEL_OSD_XOFFSET;
    m_DstPt.Y = IPANEL_OSD_YOFFSET;

    m_Opcode.AlphaUse     = 1;
    m_Opcode.Alpha        = 0x80;
    m_Opcode.BltControl   = GFX_OP_COPY;
    m_Opcode.ROP          = GFX_OP_COPY;

    memset(m_SrcBitmap.pBits, 0x0, m_SrcBitmap.Stride*m_SrcBitmap.Height);
    memset(m_DstBitmap.pBits, 0x0, m_DstBitmap.Stride*m_DstBitmap.Height);

#elif (PORTING_COLORFMT == PORTING_ARGB8888)
    m_src_buffer = (unsigned char*)ipanel_porting_malloc(IPANEL_DEST_PAL_WIDTH 
                                                       * IPANEL_DEST_PAL_HEIGHT * 4);

    if ( !m_src_buffer)
    {
        ipanel_porting_dprintf("[ipanel_osd_nxp_init] malloc src_buffer failed \n");
        goto OSD_INIT_FAILED;
    }

    memset(m_src_buffer, 0x00, IPANEL_DEST_PAL_WIDTH*IPANEL_DEST_PAL_HEIGHT*4);

    OSDInit();

    rectOsd.top     = 0;
    rectOsd.left    = 0;
    rectOsd.bottom  = IPANEL_DEST_PAL_HEIGHT;
    rectOsd.right   = IPANEL_DEST_PAL_WIDTH;
    dwStride        = 0;
    OsdOption       =  OSD_RO_LOADPALETTE|OSD_RO_ALPHABOTHVIDEO|OSD_RO_ALPHAENABLE|
                       OSD_RO_FFENABLE|OSD_RO_ENABLE;

    m_osd_handle = CreateOSDRgn(&rectOsd, OSD_MODE_32ARGB, OsdOption, NULL, dwStride);
    if (!m_osd_handle)
    {
        ipanel_porting_dprintf("[ipanel_osd_nxp_init] CreateOSDRgn failed !!!\n");
        goto OSD_INIT_FAILED;
    }
    SetOSDColorKey(0) ;
    SetOSDRgnOptions(m_osd_handle, OSD_RO_COLORKEYENABLE,  TRUE);

    m_SrcBitmap.Version   = 0;
    m_SrcBitmap.VerSize   = 0;
    m_SrcBitmap.Type      = GFX_ARGB32;
    m_SrcBitmap.Bpp       = 32;
#ifdef TEST_OSD_AUTOSCALE
    m_SrcBitmap.Height    = IPANEL_DEST_PAL_HEIGHT;
    m_SrcBitmap.Width     = IPANEL_DEST_PAL_WIDTH;
    m_SrcBitmap.Stride    = IPANEL_DEST_PAL_WIDTH * 4;
#else
    m_SrcBitmap.Height    = IPANEL_TV_SCREEN_HEIGHT;
    m_SrcBitmap.Width     = IPANEL_TV_SCREEN_WIDTH;
    m_SrcBitmap.Stride    = IPANEL_TV_SCREEN_WIDTH * 4;
#endif
    m_SrcBitmap.dwRef     = 0;
    m_SrcBitmap.pPalette  = NULL;
    m_SrcBitmap.pBits     = (void*)m_src_buffer;

    m_DstBitmap.Version   = 0x0001;
    m_DstBitmap.VerSize   = sizeof(GFX_BITMAP);
    m_DstBitmap.Type      = GFX_ARGB32;
    m_DstBitmap.Bpp       = 32;
    m_DstBitmap.Height    = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_BUFFERHEIGHT);
    m_DstBitmap.Width     = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_BUFFERWIDTH);
    m_DstBitmap.Stride    = (u_int16) GetOSDRgnOptions(m_osd_handle, OSD_RO_FRAMESTRIDE);
    m_DstBitmap.dwRef     = (u_int32) m_osd_handle;
    m_DstBitmap.pPalette  = (void*) GetOSDRgnOptions(m_osd_handle, OSD_RO_PALETTEADDRESS);
    m_DstBitmap.pBits     = (void*) GetOSDRgnOptions(m_osd_handle, OSD_RO_FRAMEBUFFER);

    m_SrcRect.Left        = 0;
    m_SrcRect.Top         = 0;
    m_SrcRect.Right       = m_SrcBitmap.Width;
    m_SrcRect.Bottom      = m_SrcBitmap.Height;

    m_DstPt.X = IPANEL_OSD_XOFFSET;
    m_DstPt.Y = IPANEL_OSD_YOFFSET;

#ifdef USE_SEMITRANSPARENT_COLOR
    m_Opcode.AlphaUse     = GFX_BLEND_USE_FIXED;
    m_Opcode.Alpha        = 0xff;
    m_Opcode.BltControl   = GFX_OP_ALPHA;
    m_Opcode.ROP          = GFX_ROP_COPY;
#else
    m_Opcode.AlphaUse     = 1;
    m_Opcode.Alpha        = 0xff;
    m_Opcode.BltControl   = GFX_OP_COPY;
    m_Opcode.ROP          = GFX_ROP_COPY;
#endif

    memset(m_SrcBitmap.pBits, 0x0, m_SrcBitmap.Stride*m_SrcBitmap.Height);
    memset(m_DstBitmap.pBits, 0x0, m_DstBitmap.Stride*m_DstBitmap.Height);
#else
    ipanel_porting_dprintf("[ipanel_osd_nxp_init] ERROR : unknown color format !\n");
#endif

    return (ret=IPANEL_OK);

OSD_INIT_FAILED:
    return ret;
}

VOID ipanel_graphics_exit(VOID)
{
    ipanel_porting_dprintf("[ipanel_graphics_exit] is called\n");

    DestroyOSDRegion(m_osd_handle);
    m_osd_handle = 0 ;

    if (m_src_buffer)
    {
        ipanel_porting_free(m_src_buffer);
        m_src_buffer = NULL;
    }
}

