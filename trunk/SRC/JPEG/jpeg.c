/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       JPEG.C                                                   */
/*                                                                          */
/* Description:    OpenTV 1.2 JPEG image decompression driver               */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/********************************************************************************/
/* Note: This driver uses a semaphore to serialise access to the APIs. For this */
/* to work, we have to make the assumption that it will not be called from      */
/* interrupt context. Testing will show if this is a valid assumption or not    */
/* but it is fairly likely since the OpenTV interpreter is the likely client.   */
/********************************************************************************/

/*#define STUB_OUT_ARM_LIBRARY*/
/* #define USE_INTERNAL_READ_HEADER */

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#ifdef OPENTV_12
#include "opentv_12.h"
#else
#include "basetype.h"
#endif
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "armjapi.h"
#include "osdlib.h"
#include "vidlib.h"
#include "gfxlib.h"
#include "gfxtypes.h"

#ifndef OPENTV_12

typedef int *intF;

typedef struct {
	char		name[12];
	char		version[8];
	unsigned char	type;
} o_jpeg_sys_desc;

#define UNKNOWN_JPEG_DECODER	0
#define SOFTWARE_JPEG_DECODER	1 
#define HARDWARE_JPEG_DECODER	2
#define HYBRID_JPEG_DECODER	3

typedef struct { 
	/* system query registration data */ 
	o_jpeg_sys_desc	jpeg_sys_info;
	/* decode parameters */ 
	int	(* jpeg_decode_set_horizontal_scaling)(intF scale_factor);
	int	(* jpeg_decode_set_vertical_scaling)(intF scale_factor);
	int	(* jpeg_decode_get_horizontal_scaling)(void);
	int	(* jpeg_decode_get_vertical_scaling)(void);
	/* decode control */ 
	int	(* jpeg_decode_start)(voidF jpeg_data, int size);
	int	(* jpeg_decode_resume)(void);
	void	(* jpeg_decode_abort)(void);
	/* decoded image retrieval */ 
	int	(* jpeg_image_get_width)(void);
	int	(* jpeg_image_get_height)(void);
	voidF	(* jpeg_image_get_buffer)(void);
	int	(* jpeg_image_get_current_segment)(intF start, intF height);
	void	(* jpeg_image_free_buffer)(void);
} jpeg_driver_ftable;

#define JPEG_COMMAND_NOT_ACCEPTED	0
#define JPEG_COMMAND_ACCEPTED		  1
#define JPEG_SCALE_CONSTANT		    (1024L)

extern void o_jpeg_driver_register(jpeg_driver_ftable *table);
extern void o_jpeg_driver_failure(void);
extern void o_jpeg_command_done(void);
extern void o_jpeg_command_failed(void);

#endif

/********************************/       
/* Internal Function Prototypes */
/********************************/
bool         jpeg_decode_init(void);
static int   jpeg_decode_set_horizontal_scaling(intF scale_factor);
static int   jpeg_decode_set_vertical_scaling(intF scale_factor);
static int   jpeg_decode_get_horizontal_scaling(void);
static int   jpeg_decode_get_vertical_scaling(void);
static int   jpeg_decode_start(voidF jpeg_data, int size);
static int   jpeg_decode_resume(void);
static void  jpeg_decode_abort(void);
static int   jpeg_image_get_width(void);
static int   jpeg_image_get_height(void);
static int   jpeg_image_get_current_segment(intF start, intF height);
static voidF jpeg_image_get_buffer(void);
static void  jpeg_image_free_buffer(void);

#ifdef USE_INTERNAL_READ_HEADER
static armjpeg_error jpeg_read_header(jpeg_buffer *pJpegBuffer, image_buffer *pImageBuffer);
static armjpeg_error ProcessMarker(u_int8 *pMarker, jpeg_buffer *pJpeg, image_buffer *pImage, int *pSkip);
static u_int8 *AdvanceToNextMarker(u_int8 *pCurrent, u_int8 *pEnd);
#endif

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/

/* Trace message types and levels */
#define JPG_ERROR (TRACE_GEN | TRACE_LEVEL_4)
#define JPG_INFO  (TRACE_GEN | TRACE_LEVEL_3)
#define JPG_FUNC  (TRACE_GEN | TRACE_LEVEL_2)

/* Supported Scale Factors */
#define SCALE_JPG_05  (JPEG_SCALE_CONSTANT/2)
#define SCALE_JPG_10  JPEG_SCALE_CONSTANT
#define SCALE_JPG_20  (JPEG_SCALE_CONSTANT*2)

/* Buffer size constraints */
#define MIN_BUFFER_LINES        16
#define NUM_LINE_GRANULARITY    16
#define WIDTH_PIXEL_GRANULARITY 16
#define MAX_BUFFER_LINES        128

/* Images whose height is not a multiple of the value below will be */
/* truncated to the next lower multiple of this value. OpenTV 1.2   */
/* seems to need 16 pixel granularity in the height of the image    */
#define HEIGHT_LINE_GRANULARITY 16

/* How many mS do we wait for a blit to complete? */
#define JPEG_BLIT_TIMEOUT 1000

/* OpenTV driver function table */
jpeg_driver_ftable jpeg_jump_table =
{
  {
    {'C','N','X','T','_','J','P','E','G',' ',' ','\0'},
    {'1','.','0','0',' ',' ',' ','\0'},
    SOFTWARE_JPEG_DECODER
  },
  jpeg_decode_set_horizontal_scaling,
  jpeg_decode_set_vertical_scaling,
  jpeg_decode_get_horizontal_scaling,
  jpeg_decode_get_vertical_scaling,
  jpeg_decode_start,
  jpeg_decode_resume,
  jpeg_decode_abort,
  jpeg_image_get_width,
  jpeg_image_get_height,
  jpeg_image_get_buffer,
  jpeg_image_get_current_segment,
  jpeg_image_free_buffer
};

typedef enum _jpeg_state
{
  JPG_IDLE,             /* Waiting for new image */
  JPG_STARTED,          /* Initialised for new decode */
  JPG_SECTION_STARTED,  /* Image received, decode in progress */
  JPG_SECTION_COMPLETE, /* Section decode finished, waiting for read */
  JPG_SECTION_READ      /* Section read, waiting for resume */
} jpeg_state;

/* Variables used in communication between API and decoder task  */
typedef struct _jpeg_decoder
{
  jpeg_state     sState;
  int            iScaleX;
  int            iScaleY;                
  int            iMacroSizeX;
  int            iMacroSizeY;
  int            iStartLine;
  int            iBufferLines;
  int            iHeight;  /* Scaled image height (rather than buffer size) */
  int            iWidth;   /* Scaled image width */
  int            iPaddedHeight;
  HVIDRGN        hBuffer;
  armjpeg_byte  *pInstance;
  armjpeg_handle hDecoder;
  jpeg_buffer    sJpgBuffer;
  image_buffer   sImgBuffer;
} jpeg_decoder;

typedef struct _rgb_pixel
{
  u_int8 R;
  u_int8 G;
  u_int8 B;
  u_int8 Reserved;
} rgb_pixel;

static sem_id_t      semJpeg;
static bool          bJpegInitialised = FALSE;
static jpeg_decoder  sJpeg = 
{
  JPG_IDLE,
  SCALE_JPG_10,
  SCALE_JPG_10,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  NULL,
  0,
  { NULL, NULL, 0, JPEGS_2H2V, JPEGF_none, 0, 0, 0 },
  { NULL, NULL, PIX_YCBCR, 8, 8, 0, 0, 0, 0, 0, 0, 0, NULL }
};                                                               

/*************************************/       
/* More Internal Function Prototypes */
/*************************************/

static int   jpeg_get_closest_scale(int iRequested);
static bool  jpeg_reset_decoder(voidF jpeg_data, int size);
static bool  jpeg_free_resources(void);
static bool  jpeg_get_image_buffer(image_buffer *pImage, int iGranX, int iGranY);
static void blank_overrun_pixels(jpeg_decoder *pInstance);
static void fixup_chroma_position(jpeg_decoder *pInstance);
int calc_macroblocks_from_lines(jpeg_decoder *pInstance, int iNumLines);
static void fill_420_buffer_rect_with_black(u_int8 *pDest, int iPitch, int iHeight,
                                            int iExtX, int iExtY,
                                            int iX,    int iY);
/*********************************/
/*********************************/
/**                             **/
/** Driver Functions Start Here **/
/**                             **/
/*********************************/
/*********************************/

/********************************************************************/
/*  FUNCTION:    jpeg_decode_init                                   */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the JPEG driver and register it with    */
/*               OpenTV                                             */
/*                                                                  */
/*  RETURNS:     TRUE for success, FALSE otherwise                  */
/********************************************************************/
bool jpeg_decode_init(void)
{
  if (bJpegInitialised)
  {
    trace_new(JPG_ERROR, "JPG: Driver init called more than once\n");
    return(TRUE);
  }
    
  /* Create the API serialisation semaphore */
  semJpeg = sem_create(1, "JPGS");
  if (!semJpeg)
  {
    trace_new(JPG_ERROR, "JPG: Can't create serialisation sem\n");
    error_log(ERROR_WARNING | MOD_JPG);
    return(FALSE);
  }
  
  /* Register the driver with OpenTV */
  o_jpeg_driver_register(&jpeg_jump_table);
  
  bJpegInitialised = TRUE;
  
  trace_new(JPG_ERROR, "JPG: Driver inititialised\n");
  
  return(TRUE);
}


/********************************************************************/
/*  FUNCTION:    jpeg_set_horizontal_scaling                        */
/*                                                                  */
/*  PARAMETERS:  pScale - ptr to requested scaling factor           */
/*                              multiplied by JPEG_SCALE_CONSTANT   */
/*                                                                  */
/*  DESCRIPTION: Set the horizontal scaling factor to be used for   */
/*               decode of the next image.                          */
/*                                                                  */
/*  RETURNS:     JPEG_COMMAND_NOT_ACCEPTED if a decode is already   */
/*               in progress.                                       */
/*               JPEG_COMMAND_ACCEPTED otherwise. In this case, the */
/*               pScale value will be updated to hold the scale     */
/*               factor that has been set (closest to requested)    */
/********************************************************************/
static int jpeg_decode_set_horizontal_scaling(intF pScale)
{
  int iKalRet;
  int iRetcode = JPEG_COMMAND_NOT_ACCEPTED;
  
  trace_new(JPG_FUNC, "->jpeg_decode_set_horizontal_scaling 0x%x ->\n", *pScale);
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    if (sJpeg.sState == JPG_IDLE)
    {
      sJpeg.iScaleX = jpeg_get_closest_scale(*pScale);
      *pScale = sJpeg.iScaleX;
      iRetcode = JPEG_COMMAND_ACCEPTED;
    }  
    sem_put(semJpeg);
  }
  else
  {
    error_log(ERROR_WARNING | MOD_JPG);
    trace_new(JPG_ERROR, "JPG: Can't get API sem\n");
  }
    
  /* Tell OpenTV we finished. Note that this call MUST be after the */
  /* sem_put call or else the callback will not be able to call the */
  /* JPEG driver without a dealock.                                 */
  if(iRetcode == JPEG_COMMAND_ACCEPTED)
    o_jpeg_command_done();
    
  trace_new(JPG_FUNC, "<-jpeg_decode_set_horizontal_scaling\n");
  
  return(iRetcode);
}

/********************************************************************/
/*  FUNCTION:    jpeg_set_vertical_scaling                          */
/*                                                                  */
/*  PARAMETERS:  pScale - ptr to requested scaling factor           */
/*                              multiplied by JPEG_SCALE_CONSTANT   */
/*                                                                  */
/*  DESCRIPTION: Set the vertical scaling factor to be used for     */
/*               decode of the next image.                          */
/*                                                                  */
/*  RETURNS:     JPEG_COMMAND_NOT_ACCEPTED if a decode is already   */
/*               in progress.                                       */
/*               JPEG_COMMAND_ACCEPTED otherwise. In this case, the */
/*               pScale value will be updated to hold the scale     */
/*               factor that has been set (closest to requested)    */
/********************************************************************/
static int jpeg_decode_set_vertical_scaling(intF pScale)
{
  int iKalRet;
  int iRetcode = JPEG_COMMAND_NOT_ACCEPTED;
  
  trace_new(JPG_FUNC, "->jpeg_decode_set_vertical_scaling 0x%x\n", *pScale);
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    if (sJpeg.sState == JPG_IDLE)
    {
      sJpeg.iScaleY = jpeg_get_closest_scale(*pScale);
      *pScale = sJpeg.iScaleY;
      iRetcode = JPEG_COMMAND_ACCEPTED;
    }  
    sem_put(semJpeg);
  }
  else
  {
    error_log(ERROR_WARNING | MOD_JPG);
    trace_new(JPG_ERROR, "JPG: Can't get API sem\n");
  }
    
  /* Tell OpenTV we finished. Note that this call MUST be after the */
  /* sem_put call or else the callback will not be able to call the */
  /* JPEG driver without a dealock.                                 */
  if(iRetcode == JPEG_COMMAND_ACCEPTED)
    o_jpeg_command_done();
      
  trace_new(JPG_FUNC, "<-jpeg_decode_set_vertical_scaling\n");
  return(iRetcode);
}

/********************************************************************/
/*  FUNCTION:    jpeg_get_horizontal_scaling                        */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Get the horizontal scaling factor to be used for   */
/*               decode of the current or next image.               */
/*                                                                  */
/*  RETURNS:     Scale factor * JPEG_SCALE_CONSTANT or 0 if error   */
/********************************************************************/
static int jpeg_decode_get_horizontal_scaling(void)
{
  int iKalRet;
  int iScale = 0;
  
  trace_new(JPG_FUNC, "->jpeg_decode_get_horizontal_scaling\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    iScale = sJpeg.iScaleX;
    sem_put(semJpeg);
  }
  else
  {
    error_log(ERROR_WARNING | MOD_JPG);
    trace_new(JPG_ERROR, "JPG: Can't get API sem\n");
  }
  
  trace_new(JPG_FUNC, "<-jpeg_decode_get_horizontal_scaling\n");
    
  return(iScale);
}

/********************************************************************/
/*  FUNCTION:    jpeg_get_vertical_scaling                          */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Get the vertical scaling factor to be used for     */
/*               decode of the current or next image.               */
/*                                                                  */
/*  RETURNS:     Scale factor * JPEG_SCALE_CONSTANT or 0 if error   */
/********************************************************************/
static int jpeg_decode_get_vertical_scaling(void)
{
  int iKalRet;
  int iScale = 0;
  
  trace_new(JPG_FUNC, "->jpeg_decode_get_vertical_scaling\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    iScale = sJpeg.iScaleY;
    sem_put(semJpeg);
  }
  else
  {
    error_log(ERROR_WARNING | MOD_JPG);
    trace_new(JPG_ERROR, "JPG: Can't get API sem\n");
  }
    
  trace_new(JPG_FUNC, "<-jpeg_decode_get_vertical_scaling\n");
  
  return(iScale);
}


/********************************************************************/
/*  FUNCTION:    jpeg_decode_start                                  */
/*                                                                  */
/*  PARAMETERS:  jpeg_data - new compressed image data pointer      */
/*               size      - size of data pointed to by jpeg_data   */
/*                                                                  */
/*  DESCRIPTION: Start decoding of a new image.                     */
/*                                                                  */
/*  RETURNS:     JPEG_COMMAND_ACCEPTED if successful.               */
/*               JPEG_COMMAND_NOT_ACCEPTED if error.                */
/********************************************************************/
static int jpeg_decode_start(voidF jpeg_data, int size)
{
  int iKalRet;
  int iRetcode = JPEG_COMMAND_NOT_ACCEPTED;
  int iNumLines;
  int iNumMacroblocks;
  bool bReportDone = FALSE;
  armjpeg_error armRetcode;
  
  trace_new(JPG_FUNC, "->jpeg_decode_start 0x%x, 0x%x\n", jpeg_data, size);
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    /* Check to see if we are in the correct state */
    if (sJpeg.sState != JPG_IDLE)
    {
      trace_new(JPG_ERROR, "Decoder in wrong state %d\n", sJpeg.sState);
      error_log(MOD_JPG | ERROR_WARNING);
    }
    
    /* Clean up any resources currently held and set up for a new decode */
    if (jpeg_reset_decoder(jpeg_data, size))
    {
      /* We managed to allocate all required resources for the decode */
      /* so start it going.                                           */
      armRetcode = armjpeg_decompress_start(sJpeg.hDecoder,
                                            &sJpeg.sJpgBuffer,
                                            &sJpeg.sImgBuffer);
      if (armRetcode != ERR_ok)
      {
        error_log(MOD_JPG | ERROR_WARNING);
        trace_new(JPG_ERROR, "Error starting decode 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      }  
      else
      {
        sJpeg.sState = JPG_SECTION_STARTED;
        
        /* Reset the output pointer */
        sJpeg.sImgBuffer.ptr  = sJpeg.sImgBuffer.base;
        //sJpeg.sImgBuffer.xptr = sJpeg.sImgBuffer.base;
    
        /* Figure out how many blocks to decompress */
        iNumLines = min(sJpeg.sImgBuffer.pheight, sJpeg.iHeight);
        iNumMacroblocks = calc_macroblocks_from_lines(&sJpeg, iNumLines);
        
        trace_new(JPG_INFO, "Decompressing %d lines (%d macroblocks)\n", iNumLines, iNumMacroblocks);
        
        armRetcode = armjpeg_decompress_some(sJpeg.hDecoder, iNumMacroblocks);
        
        if (armRetcode != ERR_ok)
        {
          error_log(MOD_JPG | ERROR_WARNING);
          trace_new(JPG_ERROR, "Error starting decode 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
        }  
        else
        {
          sJpeg.sState = JPG_SECTION_COMPLETE;
          sJpeg.iBufferLines = iNumLines;

          /* Write-back the D-cache here to make sure that a following blit will */
          /* get all the data.                                              */
          CleanDCache();
          DrainWriteBuffer();
    
          /* Fill any extra pixels on the right edge with black and        */
          /* reposition the chroma plane if the buffer is only partly full */
          blank_overrun_pixels(&sJpeg);
          
          /* If the buffer is not completely full, we move the chroma    */
          /* information up to immediately below the valid luma to allow */
          /* the code using the JPEG driver to correctly copy the image  */
          /* segment without having to know both the number of lines of  */
          /* image available and the height of the intermediate buffer.  */
          fixup_chroma_position(&sJpeg);
          
          /* Wait for the blitter to finish */
          GfxIsGXACmdDone (JPEG_BLIT_TIMEOUT);
          
          iRetcode = JPEG_COMMAND_ACCEPTED;
          bReportDone = TRUE;
        }  
      }
    }
    /* If something went wrong during the function, tidy up */
    if ((iRetcode == JPEG_COMMAND_NOT_ACCEPTED) && (sJpeg.sState != JPG_IDLE))
    {
      trace_new(JPG_INFO, "Freeing resources after error in jpeg_decode_start\n");
      jpeg_free_resources();
    }
    
    sem_put(semJpeg);
  }

  if(bReportDone)
    o_jpeg_command_done();

  trace_new(JPG_FUNC, "<-jpeg_decode_start\n");
      
  return(iRetcode);
}


/********************************************************************/
/*  FUNCTION:    calc_macroblocks_from_lines                        */
/*                                                                  */
/*  PARAMETERS:  pInstance                                          */
/*               iNumLines                                          */
/*                                                                  */
/*  DESCRIPTION: Given the number of lines to decompress and info   */
/*               on the image, calculate the number of macroblocks  */
/*               to pass to armjpeg_decompress_some.                */
/*                                                                  */
/*  RETURNS:     Number of macroblocks included in the group of     */
/*               lines passed                                       */
/********************************************************************/
int calc_macroblocks_from_lines(jpeg_decoder *pInstance, int iNumLines)
{
  int iBlocksPerLine;
  int iNumBlocks;
 
  iBlocksPerLine = pInstance->sImgBuffer.pwidth / pInstance->iMacroSizeX;
  iNumBlocks     = (iNumLines + (pInstance->iMacroSizeY - 1)) / pInstance->iMacroSizeY;
  iNumBlocks    *= iBlocksPerLine;
  
  return(iNumBlocks);
}

/********************************************************************/
/*  FUNCTION:    jpeg_decode_resume                                 */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Continue decoding an image.                        */
/*                                                                  */
/*  RETURNS:     JPEG_COMMAND_ACCEPTED if successful.               */
/*               JPEG_COMMAND_NOT_ACCEPTED if error.                */
/********************************************************************/
static int jpeg_decode_resume(void)
{
  int iKalRet;
  int iNumLines;
  int iNumMacroblocks;
  armjpeg_error armRetcode;
  bool bReportDone = FALSE;
  int iRetcode = JPEG_COMMAND_NOT_ACCEPTED;
  
  trace_new(JPG_FUNC, "->jpeg_decode_resume\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    /* Check state */
    if (sJpeg.sState != JPG_SECTION_READ)
    {
      trace_new(JPG_ERROR, "Decoder in wrong state %d\n", sJpeg.sState);
      error_log(MOD_JPG | ERROR_WARNING);
    }
    else
    {
      sJpeg.iStartLine += sJpeg.iBufferLines;
      sJpeg.iBufferLines = 0;
    }
  
    /* Regardless of whether last section was read, we can continue */
    /* as long as the state is not JPG_IDLE.                        */
    if (sJpeg.sState != JPG_IDLE)
    {
      /* Make sure that the blitter is idle before we start. It is possible that the   */
      /* last section copy has not completed when we are called to start decompressing */
      /* the next block.                                                                */
      GfxIsGXACmdDone (JPEG_BLIT_TIMEOUT);
      
      /* Reset the output pointers */
      sJpeg.sImgBuffer.ptr  = sJpeg.sImgBuffer.base;
      //sJpeg.sImgBuffer.xptr = sJpeg.sImgBuffer.base;
      
      /* Figure out how many blocks to decompress */
      iNumLines = min(sJpeg.sImgBuffer.pheight, (sJpeg.iHeight - sJpeg.iStartLine));
      iNumMacroblocks = calc_macroblocks_from_lines(&sJpeg, iNumLines);
        
      trace_new(JPG_INFO, "Decompressing %d lines (%d macroblocks) from line %d\n", iNumLines, iNumMacroblocks, sJpeg.iStartLine);
                                                                   
      armRetcode = armjpeg_decompress_some(sJpeg.hDecoder, iNumMacroblocks);

      if (armRetcode != ERR_ok)
      {
        error_log(MOD_JPG | ERROR_WARNING);
        trace_new(JPG_ERROR, "Error from decompress_some 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      }  
      else
      {
        sJpeg.sState = JPG_SECTION_COMPLETE;
        sJpeg.iBufferLines = iNumLines;

        /* Write-back the D-cache here to make sure that a following blit will */
        /* get all the data.                                              */
        CleanDCache();
        DrainWriteBuffer();
    
        blank_overrun_pixels(&sJpeg);
        
        /* If the buffer is not completely full, we move the chroma    */
        /* information up to immediately below the valid luma to allow */
        /* the code using the JPEG driver to correctly copy the image  */
        /* segment without having to know both the number of lines of  */
        /* image available and the height of the intermediate buffer.  */
        fixup_chroma_position(&sJpeg);
          
        /* Wait for the blitter to finish */
        GfxIsGXACmdDone (JPEG_BLIT_TIMEOUT);

        iRetcode = JPEG_COMMAND_ACCEPTED;
        bReportDone = TRUE;
      }
    }

    sem_put(semJpeg);
  }
  
  if(bReportDone)
    o_jpeg_command_done();

  trace_new(JPG_FUNC, "<-jpeg_decode_resume\n");
  
  return(iRetcode);
}


/********************************************************************/
/*  FUNCTION:    jpeg_decode_abort                                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Abort any decode operation that is currently       */
/*               underway.                                          */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static void jpeg_decode_abort(void)
{
  int iKalRet;
  
  trace_new(JPG_FUNC, "->jpeg_decode_abort\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    jpeg_free_resources();
    sem_put(semJpeg);
  }  
  
  trace_new(JPG_FUNC, "<-jpeg_decode_abort\n");
  return;
}

/********************************************************************/
/*  FUNCTION:    jpeg_image_get_width                               */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the width of the image currently being      */
/*               decoded.                                           */
/*                                                                  */
/*  RETURNS:     Width of current image in pixels                   */
/********************************************************************/
static int jpeg_image_get_width(void)
{
  int iKalRet;
  int iWidth = 0;

  trace_new(JPG_FUNC, "->jpeg_image_get_width\n");
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    iWidth = sJpeg.sImgBuffer.pwidth;
    sem_put(semJpeg);
  }
  
  trace_new(JPG_FUNC, "<-jpeg_image_get_width\n");
  
  return(iWidth);
}

/********************************************************************/
/*  FUNCTION:    jpeg_image_get_height                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the height of the image currently being     */
/*               decoded.                                           */
/*                                                                  */
/*  RETURNS:     Height of current image in pixels                  */
/********************************************************************/
static int jpeg_image_get_height(void)
{
  int iKalRet;
  int iHeight = 0;

  trace_new(JPG_FUNC, "->jpeg_image_get_height\n");
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    iHeight = sJpeg.iHeight; 
    sem_put(semJpeg);
  }
  
  trace_new(JPG_FUNC, "<-jpeg_image_get_height\n");
  
  return(iHeight);
}

/********************************************************************/
/*  FUNCTION:    jpeg_image_get_current_segment                     */
/*                                                                  */
/*  PARAMETERS:  start  - pointer to storage for decoded segment    */
/*                        start line number                         */
/*               height - pointer to storage for number of lines in */
/*                        decoded segment                           */
/*                                                                  */
/*  DESCRIPTION: Return a the start line and height of the last     */
/*               decoded image segment.                             */
/*                                                                  */
/*  RETURNS:     JPEG_COMMAND_ACCEPTED if successful.               */
/*               JPEG_COMMAND_NOT_ACCEPTED if error.                */
/********************************************************************/
static int jpeg_image_get_current_segment(intF start, intF height)
{
  int iKalRet;
  int iRetcode = JPEG_COMMAND_NOT_ACCEPTED;
  
  trace_new(JPG_FUNC, "->jpeg_image_get_current_segment\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    if((sJpeg.sState != JPG_SECTION_COMPLETE) && (sJpeg.sState != JPG_SECTION_READ))
    {
      trace_new(JPG_ERROR, "Decoder in wrong state %d\n", sJpeg.sState);
      error_log(MOD_JPG | ERROR_WARNING);
    }
    else
    {
      sJpeg.sState = JPG_SECTION_READ;
      *start = sJpeg.iStartLine;
      *height = sJpeg.iBufferLines;
      iRetcode = JPEG_COMMAND_ACCEPTED;
    }
    
    sem_put(semJpeg);
  }
  
  trace_new(JPG_FUNC, "<-jpeg_image_get_current_segment\n");
  return(iRetcode);
}

/********************************************************************/
/*  FUNCTION:    jpeg_image_get_buffer                              */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return a pointer to the partial image decode       */
/*               buffer.                                            */
/*                                                                  */
/*  RETURNS:     Pointer if buffer exists, NULL otherwise           */
/********************************************************************/
static voidF jpeg_image_get_buffer(void)
{
  voidF pBuffer;
  int iKalRet;
  
  trace_new(JPG_FUNC, "->jpeg_image_get_buffer\n");

  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    if((sJpeg.sState != JPG_SECTION_COMPLETE) && (sJpeg.sState != JPG_SECTION_READ))
    {
      trace_new(JPG_ERROR, "Incorrect decoder state %d\n", sJpeg.sState);
      error_log(ERROR_WARNING | MOD_JPG);
    }  

    pBuffer = (voidF)sJpeg.hBuffer;
    sJpeg.sState = JPG_SECTION_READ;
    
    sem_put(semJpeg);
  }  
  trace_new(JPG_FUNC, "<-jpeg_image_get_buffer\n");
  
  return(pBuffer);
}

/********************************************************************/
/*  FUNCTION:    jpeg_image_free_buffer                             */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Free resources after completion of decoding        */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static void jpeg_image_free_buffer(void)
{
  int iKalRet;
  armjpeg_error armRetcode;
  
  trace_new(JPG_FUNC, "->jpeg_image_free_buffer\n");
  
  iKalRet = sem_get(semJpeg, KAL_WAIT_FOREVER);
  if (iKalRet == RC_OK)
  {
    if (sJpeg.sState != JPG_SECTION_READ)
    {
      trace_new(JPG_ERROR, "Incorrect decoder state %d\n", sJpeg.sState);
      error_log(ERROR_WARNING | MOD_JPG);
    }
    
    if (sJpeg.hDecoder)
    {
      armRetcode = armjpeg_decompress_stop(sJpeg.hDecoder);
      if (armRetcode != ERR_ok)
      {
        trace_new(JPG_ERROR, "Error from armpeg_decompress_stop 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
        error_log(ERROR_WARNING | MOD_JPG);
      }  
    }
      
    jpeg_free_resources();
    sem_put(semJpeg);
  }
  
  trace_new(JPG_FUNC, "<-jpeg_image_free_buffer\n");
  
  return;
}

/************************/
/************************/
/**                    **/
/** Internal Functions **/
/**                    **/
/************************/
/************************/

/********************************************************************/
/*  FUNCTION:    jpeg_get_closest_scale                             */
/*                                                                  */
/*  PARAMETERS:  iRequested - requested scaling factor              */
/*                                                                  */
/*  DESCRIPTION: Return the supported scaling factor which is       */
/*               closest to the requested factor.                   */
/*                                                                  */
/*  RETURNS:     SCALE_JPG_nn representing the closest supported    */
/*               scale factor                                       */
/********************************************************************/
static int jpeg_get_closest_scale(int iRequested)
{
  if (iRequested < (3*JPEG_SCALE_CONSTANT)/4)
    return SCALE_JPG_05;
  else if (iRequested < (5*JPEG_SCALE_CONSTANT)/4)
    return SCALE_JPG_10;
  else
    return SCALE_JPG_20;
}

#ifdef OLD_VERSION
/********************************************************************/
/*  FUNCTION:    jpeg_reset_decoder                                 */
/*                                                                  */
/*  PARAMETERS:  jpeg_data - new compressed image data pointer      */
/*               size      - size of data pointed to by jpeg_data   */
/*                                                                  */
/*  DESCRIPTION: Fix up the decoder to start a new image decode.    */
/*               Free any existing resources and set up an          */
/*               appropriately sized decode buffer for the new image*/
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE otherwise                */
/********************************************************************/
static bool jpeg_reset_decoder(voidF jpeg_data, int size)
{
  armjpeg_error  armRetcode;
  armjpeg_config sConfig;
  jpeg_buffer    sJpegBuff;
  image_buffer   sImageBuff;
  int            iBuffSize;
  bool           bRetcode;
  
  /* Get the size of the new image */
  sJpegBuff.base = jpeg_data;
  sJpegBuff.size = size;
  
  armRetcode = armjpeg_read_header(&sJpegBuff, &sImageBuff);
  if (armRetcode != ERR_ok)
  {
    trace_new(JPG_ERROR, "Error reading JPEG image header 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
    error_log(ERROR_WARNING | MOD_JPG);
    return(FALSE);
  }
  
  #ifdef USE_INTERNAL_READ_HEADER
  /* Call our own version to fill in fields that armjpeg_read_header misses */
  armRetcode = jpeg_read_header(&sJpegBuff, &sImageBuff);
  if (armRetcode != ERR_ok)
  {
    trace_new(JPG_ERROR, "Error reading JPEG image header 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
    error_log(ERROR_WARNING | MOD_JPG);
    return(FALSE);
  }
  #endif
  
  trace_new(JPG_INFO, 
            "JPEG: Resetting for image of size (%d, %d)\n",
            sImageBuff.width,
            sImageBuff.height);
          
  /* Fill in other image buffer fields as appropriate */
  sImageBuff.format = PIX_YCBCR;
  
  /* ARM scale factors are (factor * 8) so rescale from OpenTV definitions */
  sImageBuff.scalex = (sJpeg.iScaleX * 8)/JPEG_SCALE_CONSTANT;
  sImageBuff.scaley = (sJpeg.iScaleY * 8)/JPEG_SCALE_CONSTANT;
  
  /* Calculate the macroblock size taking into account source image and scaling */
  #ifndef OLD_CALCULATION
  if (sJpegBuff.components == 1)
  {
      sJpeg.iMacroSizeX = sImageBuff.scalex;      
      sJpeg.iMacroSizeY = sImageBuff.scaley;
  }
  else
  {
    switch (sJpegBuff.samp)
    {
       case JPEGS_2H2V:
        sJpeg.iMacroSizeX = 2 * sImageBuff.scalex;
        sJpeg.iMacroSizeY = 2 * sImageBuff.scaley;
        break;
      
       case JPEGS_2H1V:  
        sJpeg.iMacroSizeX = sImageBuff.scalex;
        sJpeg.iMacroSizeY = 2 * sImageBuff.scaley;
        break;
      
       case JPEGS_1H2V:  
        sJpeg.iMacroSizeX = 2 * sImageBuff.scalex;
        sJpeg.iMacroSizeY = sImageBuff.scaley;
        break;
      
       case JPEGS_1H1V:  
        sJpeg.iMacroSizeX = sImageBuff.scalex;
        sJpeg.iMacroSizeY = sImageBuff.scaley;
        break;
    }
  }  
  #else
  if( (sJpegBuff.components == 1) || 
     ((sJpegBuff.components == 3) && (sJpegBuff.samp == 0x011)))
    sJpeg.iMacroSizeY = sImageBuff.scaley;
  else
    sJpeg.iMacroSizeY = 2 * sImageBuff.scaley;  
    
  sJpeg.iMacroSizeX = 2 * sImageBuff.scalex;
  #endif
  
  /* New decoder provides us with the macroblock dimensions, however it     */
  /* doesn't make the information available until armjpeg_decompress_start  */
  /* returns. This API, however, needs the image buffer to be allocated but */   
  /* the image buffer needs to be sized in multiples of macroblocks so....  */
  
  /* Fix up the output size according to the scale factors */
  sImageBuff.height = (sImageBuff.height * sImageBuff.scaley + 7)/8;
  sImageBuff.width  = (sImageBuff.width * sImageBuff.scalex + 7)/8;
 
  /* Truncate the image height as appropriate (for OpenTV 1.2) */
  sImageBuff.height = (sImageBuff.height + (HEIGHT_LINE_GRANULARITY-1)) & ~(HEIGHT_LINE_GRANULARITY-1);
  
  /* Save the scaled size for later */
  sJpeg.iHeight = sImageBuff.height;
  sJpeg.iWidth  = sImageBuff.width;
  
  /* Free up any buffers that have not been tidied up already */
  bRetcode = jpeg_free_resources();
  if (bRetcode)
  {
    /****************************************/
    /* Get things set up for the new decode */
    /****************************************/
    
    /* How much workspace memory is required? */
    sConfig.max_components = (int)sJpegBuff.components;
    sConfig.features       = JPEGF_decompress;
    iBuffSize = armjpeg_create( &sJpeg.hDecoder,
                                &sConfig,
                                NULL);
    if (iBuffSize < 0)
    {
      armRetcode = (armjpeg_error)iBuffSize;
      trace_new(JPG_ERROR, "Error querying JPEG decoder workspace size 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
      return(FALSE);
    }                            
    else
    {
      trace_new(JPG_INFO, "JPEG decoder requests %d bytes of working memory\n", iBuffSize);                            
    }  
    
    /* Allocate decoder workspace */
    sJpeg.pInstance = mem_malloc(iBuffSize);

    if (sJpeg.pInstance == (armjpeg_byte *)NULL)
    {
      trace_new(JPG_ERROR, "No memory avaiable for JPEG decode workspace\n");
      error_log(ERROR_WARNING | MOD_JPG);
      return(FALSE);
    }

    /* Make sure the instance data is cleared out. The ARM JPEG  */
    /* library seems to make this assumption since it doesn't do */
    /* it itself and this results in it freeing the instance     */
    /* data with a call to free() on armjpeg_destroy when we     */
    /* don't want it to do this.                                 */
    memset(sJpeg.pInstance, 0, iBuffSize);


    /* Create a new decoder instance */
    iBuffSize = armjpeg_create( &sJpeg.hDecoder,
                                &sConfig,
                                sJpeg.pInstance);
    if (iBuffSize < 0)
    {
      armRetcode = (armjpeg_error)iBuffSize;
      trace_new(JPG_ERROR, "Error creating JPEG decoder instance 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
      mem_free(sJpeg.pInstance);
      sJpeg.pInstance = NULL;

      return(FALSE);
    }                            
    
    /* Copy our local jpeg and image buffer info into the global struct */
    sJpeg.sImgBuffer = sImageBuff;
    sJpeg.sJpgBuffer = sJpegBuff;
    
    /* Get image decompression workspace */
    bRetcode = jpeg_get_image_buffer(&sJpeg.sImgBuffer, sJpeg.iMacroSizeX, sJpeg.iMacroSizeY);
    
    if (!bRetcode)
    {
      trace_new(JPG_INFO, "Cleaning up JPEG decoder on malloc failure\n");
      armjpeg_destroy(sJpeg.hDecoder);
      mem_free(sJpeg.pInstance);
      sJpeg.pInstance = NULL;
    }  
    else
    {
      sJpeg.sState       = JPG_STARTED;
      sJpeg.iStartLine   = 0;
      sJpeg.iBufferLines = 0;
      
      trace_new(JPG_INFO, "JPEG decoder instance created successfully\n");
    }
  }
  
  return(bRetcode);
}
#else
/********************************************************************/
/*  FUNCTION:    jpeg_reset_decoder                                 */
/*                                                                  */
/*  PARAMETERS:  jpeg_data - new compressed image data pointer      */
/*               size      - size of data pointed to by jpeg_data   */
/*                                                                  */
/*  DESCRIPTION: Fix up the decoder to start a new image decode.    */
/*               Free any existing resources and set up an          */
/*               appropriately sized decode buffer for the new image*/
/*                                                                  */
/*  RETURNS:     TRUE if successful, FALSE otherwise                */
/********************************************************************/
static bool jpeg_reset_decoder(voidF jpeg_data, int size)
{
  armjpeg_error  armRetcode;
  armjpeg_config sConfig;
  int            iBuffSize;
  bool           bRetcode;
  
  /* Free up any buffers that have not been tidied up already */
  bRetcode = jpeg_free_resources();
  if (bRetcode)
  {
    /****************************************/
    /* Create a new instance of the decoder */
    /****************************************/
    
    /* Fill in the size of the new image */
    sJpeg.sJpgBuffer.base = jpeg_data;
    sJpeg.sJpgBuffer.size = size;
  
    /* Fill in other image buffer fields as appropriate */
    sJpeg.sImgBuffer.format = PIX_YCBCR;
  
    /* ARM scale factors are (factor * 8) so rescale from OpenTV definitions */
    sJpeg.sImgBuffer.scalex = (sJpeg.iScaleX * 8)/JPEG_SCALE_CONSTANT;
    sJpeg.sImgBuffer.scaley = (sJpeg.iScaleY * 8)/JPEG_SCALE_CONSTANT;
    
    armRetcode = armjpeg_read_header(&sJpeg.sJpgBuffer, &sJpeg.sImgBuffer);
    if (armRetcode != ERR_ok)
    {
      trace_new(JPG_ERROR, "Unable to read header on image. 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
      return(FALSE);
    }
    
    /* How much workspace memory is required? */
    sConfig.max_components = (int)sJpeg.sJpgBuffer.components;
    sConfig.features       = JPEGF_decompress;
    iBuffSize = armjpeg_create( &sJpeg.hDecoder,
                                &sConfig,
                                NULL);
    if (iBuffSize < 0)
    {
      armRetcode = (armjpeg_error)iBuffSize;
      trace_new(JPG_ERROR, "Error querying JPEG decoder workspace size 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
      return(FALSE);
    }                            
    else
    {
      trace_new(JPG_INFO, "JPEG decoder requests %d bytes of working memory\n", iBuffSize);                            
    }  
    
    /* Allocate decoder workspace */
    sJpeg.pInstance = mem_malloc(iBuffSize);

    if (sJpeg.pInstance == (armjpeg_byte *)NULL)
    {
      trace_new(JPG_ERROR, "No memory avaiable for JPEG decode workspace\n");
      error_log(ERROR_WARNING | MOD_JPG);
      return(FALSE);
    }

    /* Make sure the instance data is cleared out. The ARM JPEG  */
    /* library seems to make this assumption since it doesn't do */
    /* it itself and this results in it freeing the instance     */
    /* data with a call to free() on armjpeg_destroy when we     */
    /* don't want it to do this.                                 */
    memset(sJpeg.pInstance, 0, iBuffSize);

    /* Create a new decoder instance */
    iBuffSize = armjpeg_create( &sJpeg.hDecoder,
                                &sConfig,
                                sJpeg.pInstance);
    if (iBuffSize < 0)
    {
      armRetcode = (armjpeg_error)iBuffSize;
      trace_new(JPG_ERROR, "Error creating JPEG decoder instance 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
      mem_free(sJpeg.pInstance);
      sJpeg.pInstance = NULL;

      return(FALSE);
    }                            
    
    /***********************************************************************/
    /* Determine the size of the image and the macroblock dimensions       */
    /* Note that armjpeg_read_header does not return enough information to */
    /* allow this so we must call armjpeg_decompress_start as well.        */
    /***********************************************************************/
    
    armRetcode = armjpeg_decompress_start(sJpeg.hDecoder,
                                          &sJpeg.sJpgBuffer,
                                          &sJpeg.sImgBuffer);                                           
    if (armRetcode != ERR_ok)
    {
      error_log(MOD_JPG | ERROR_WARNING);
      trace_new(JPG_ERROR, "Error reading image header 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      armjpeg_destroy(sJpeg.hDecoder);
      mem_free(sJpeg.pInstance);
      sJpeg.pInstance = NULL;
      return(FALSE);
    }  

    /* Scale the macroblock dimensions correctly given the scaling in use */
    /* for the decoded image.                                             */
    sJpeg.iMacroSizeX = (sJpeg.sJpgBuffer.blk_width  * sJpeg.sImgBuffer.scalex)/8;     
    sJpeg.iMacroSizeY = (sJpeg.sJpgBuffer.blk_height * sJpeg.sImgBuffer.scaley)/8;     
    
    /* Fix up the output size according to the scale factors */
    sJpeg.sImgBuffer.height = (sJpeg.sImgBuffer.height * sJpeg.sImgBuffer.scaley + 7)/8;
    sJpeg.sImgBuffer.width  = (sJpeg.sImgBuffer.width  * sJpeg.sImgBuffer.scalex + 7)/8;
  
    /* Keep a copy of the scaled image dimensions in our top level state data */
    sJpeg.iHeight = sJpeg.sImgBuffer.height;
    sJpeg.iWidth  = sJpeg.sImgBuffer.width;

    /* Truncate the image height as appropriate (for OpenTV 1.2) */
    sJpeg.sImgBuffer.height = (sJpeg.sImgBuffer.height + (HEIGHT_LINE_GRANULARITY-1)) & ~(HEIGHT_LINE_GRANULARITY-1);
    
    /*************************************/
    /* Get image decompression workspace */
    /*************************************/
    
    bRetcode = jpeg_get_image_buffer(&sJpeg.sImgBuffer, sJpeg.iMacroSizeX, sJpeg.iMacroSizeY);
    
    if (!bRetcode)
    {
      trace_new(JPG_INFO, "Cleaning up JPEG decoder on malloc failure\n");
      armjpeg_destroy(sJpeg.hDecoder);
      mem_free(sJpeg.pInstance);
      sJpeg.pInstance = NULL;
    }  
    else
    {
      sJpeg.sState       = JPG_STARTED;
      sJpeg.iStartLine   = 0;
      sJpeg.iBufferLines = 0;
      
      trace_new(JPG_INFO, "JPEG decoder instance created successfully\n");
    }
  }
  
  return(bRetcode);
}
#endif

/********************************************************************/
/*  FUNCTION:     jpeg_free_resources                               */
/*                                                                  */
/*  PARAMETERS:   None                                              */
/*                                                                  */
/*  DESCRIPTION:  Free any buffers or other resources associated    */
/*                with a JPEG decode instance.                      */
/*                                                                  */
/*  RETURNS:      TRUE on success, FALSE on error                   */
/********************************************************************/
static bool jpeg_free_resources(void)
{
  armjpeg_error armRetcode;
  bool bRetcode;
  
  /* If the decoder is in idle state, it has claimed no resources */
  if (sJpeg.sState != JPG_IDLE)
  {
    /* End the current decoder instantiation */
    armRetcode = armjpeg_destroy(sJpeg.hDecoder);
    if (armRetcode != ERR_ok)
    {
      trace_new(JPG_ERROR, "Error from armpeg_destroy 0x%x ""%s""\n", armRetcode, armjpeg_error_message(armRetcode));
      error_log(ERROR_WARNING | MOD_JPG);
    }
    mem_free(sJpeg.pInstance);
    sJpeg.pInstance = NULL;
    
    /* Free up our output image buffer */
    bRetcode = vidDestroyVideoRgn(sJpeg.hBuffer);
    if (!bRetcode)
    {
      trace_new(JPG_ERROR, "JPEG: Unable to destroy buffer region\n");
      error_log(ERROR_WARNING | MOD_JPG);
    }
    
    /* Clear pointers and values for safety */
    sJpeg.pInstance          = NULL;
    sJpeg.hBuffer            = NULL;
    sJpeg.sImgBuffer.base    = NULL;
    sJpeg.sImgBuffer.ptr     = NULL;
    sJpeg.sImgBuffer.height  = 0;
    sJpeg.sImgBuffer.width   = 0;
    sJpeg.sImgBuffer.pheight = 0;
    sJpeg.sImgBuffer.pwidth  = 0;
    sJpeg.sImgBuffer.bpl     = 0;
    
    sJpeg.sJpgBuffer.base    = NULL;
    sJpeg.sJpgBuffer.ptr     = NULL;
    sJpeg.sJpgBuffer.size    = 0;
    
    sJpeg.iStartLine         = 0;
    sJpeg.iBufferLines       = 0;
    
    /* Set state to idle */
    sJpeg.sState = JPG_IDLE;
    
    if((armRetcode == ERR_ok) && bRetcode)
      return(TRUE);
    else
    {
      trace_new(JPG_ERROR, "JPEG: Error freeing resources\n");
      return(FALSE);  
    }  
  }
  
  return(TRUE);
}


/********************************************************************/
/*  FUNCTION:    jpeg_get_image_buffer                              */
/*                                                                  */
/*  PARAMETERS:  pImage - pointer to structure containing image     */
/*                        buffer description.                       */
/*               iGranX - buffer size X granularity                 */
/*               iGranY - buffer size Y granularity                 */
/*                                                                  */
/*  DESCRIPTION: Given the supplied parameters, allocate the        */
/*               largest possible buffer to hold all or part of the */
/*               image that is about to be decoded.                 */
/*                                                                  */
/*  RETURNS:     TRUE if a buffer is allocated successfully         */
/*               FALSE otherwise                                    */
/*                                                                  */
/*               If the return code is true, the image_buffer       */
/*               structure passed to the function will be updated   */
/*               to include the dimensions of and pointer to the    */
/*               allocated buffer.                                  */
/*                                                                  */
/********************************************************************/
bool jpeg_get_image_buffer(image_buffer *pImage, int iGranX, int iGranY)
{
  int iStride;
  int iPackedStride;
  int iNumLines;
  int iExtraLines;
  int iMaxLines;
  int iTemp;
  void *pBuffer;
  HVIDRGN hRgn;
  OSDRECT rectBuffer;
 
  /* Work out how wide and high our buffer must be for the supplied image */
  iTemp   = pImage->width % iGranX;
  iStride = iTemp ? pImage->width + (iGranX - iTemp) : pImage->width;

  iExtraLines = pImage->height % iGranY;
  iMaxLines   = iExtraLines ? pImage->height + (iGranY - iExtraLines) : pImage->height;
  
  /* Save the apparent image size including the extra added for granularity */
  sJpeg.iPaddedHeight = iMaxLines;
  
  /* Reduce to the maximum decode buffer size we want to use */
  iMaxLines   = min(MAX_BUFFER_LINES, iMaxLines);
  
  /* The output image is actually YUV 4:2:0 in planar format. To simplify  */
  /* our allocation loop, adjust the stride to pretend that we are dealing */
  /* with a packed line with enough space for the Cr and Cb components too */
  iPackedStride = (iStride * 3) / 2;

  /* Set initial rectangle values */
  rectBuffer.left  = 0;
  rectBuffer.top   = 0;
  rectBuffer.right = iStride;
  
  /* Try to allocate increasingly smaller buffers until a */
  /* mem_malloc call succeeds                             */
  for(iNumLines = iMaxLines; iNumLines >= MIN_BUFFER_LINES; iNumLines -= NUM_LINE_GRANULARITY)
  {
     /* Set the height of the buffer video region */
     rectBuffer.bottom = iNumLines;
     
     /* Try to create a region of this size */
     hRgn = vidCreateVideoRgn(&rectBuffer, TYPE_420);
     if (hRgn)
     {
       /* We succeeded! Fill in the image buffer info and return. This assumes that format */
       /* information is in buffer info structure that was passed to us originally.        */
       pBuffer = (void *)vidGetOptions(hRgn, VO_BUFFER_PTR);
       iStride = (int)vidGetOptions(hRgn, VO_BUFFER_STRIDE);
       
       pImage->bpl     = iStride;
       pImage->pheight = iNumLines;
       pImage->pwidth  = iStride;
       pImage->base    = (armjpeg_byte *)pBuffer;
       pImage->ptr     = (armjpeg_byte *)pBuffer;
       //pImage->xptr    = (armjpeg_byte *)pBuffer;
       sJpeg.hBuffer   = hRgn;
       trace_new(JPG_INFO, "Got %d line decode buffer, %d pixels wide at 0x%08x\n", iNumLines, iStride, pBuffer);
       return(TRUE);
     }
  }
  trace_new(JPG_INFO, "Can't allocate a JPEG image decode buffer\n");
  error_log(MOD_JPG | ERROR_WARNING);
  return(FALSE);
}


/********************************************************************/
/*  FUNCTION:    blank_overrun_pixels                               */
/*                                                                  */
/*  PARAMETERS:  pInstance - pointer to instance data               */
/*                                                                  */
/*  DESCRIPTION: If the decompressed image is narrower than the     */
/*               buffer it was decompressed into, this function     */
/*               fills the edge rectangles with black as specified  */
/*               by OpenTV. The ARM JPEG decoder fills these        */
/*               pixels by copying the pixel to the left instead of */
/*               filling with black.                                */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static void blank_overrun_pixels(jpeg_decoder *pInstance)
{
  /* Is there a stip to fill down the right hand side? */
  if (pInstance->iWidth != pInstance->sImgBuffer.pwidth)
  {
    fill_420_buffer_rect_with_black(pInstance->sImgBuffer.base, 
                                    pInstance->sImgBuffer.bpl, 
                                    pInstance->sImgBuffer.pheight,
                                    pInstance->sImgBuffer.pwidth - pInstance->iWidth,
                                    pInstance->sImgBuffer.pheight,
                                    pInstance->iWidth,
                                    0);
  }
}

/********************************************************************/
/*  FUNCTION:    fixup_chroma_position                              */
/*                                                                  */
/*  PARAMETERS:  pInstance - pointer to instance data               */
/*                                                                  */
/*  DESCRIPTION: If the decompressed image is shorter than the      */
/*               buffer it was decompressed into, this function     */
/*               moves the chroma information upwards to fall       */
/*               immediately below the valid luma data. This allows */
/*               the driver client to correctly copy the image      */
/*               segment when it only knows the number of valid     */
/*               lines. OpenTV does not provide a method of passing */
/*               the buffer height and number of valid lines        */
/*               independently as would be needed here where the    */
/*               original position of the chroma plane is defined   */
/*               by the buffer size rather than the number of valid */
/*               image lines it contains.                           */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static void fixup_chroma_position(jpeg_decoder *pInstance)
{
  u_int8 *pChromaSrc;
  u_int8 *pChromaDst;
  int     iWidth;
  
  if (pInstance->iBufferLines != pInstance->sImgBuffer.pheight)
  {
    pChromaSrc = pInstance->sImgBuffer.base +
                 pInstance->sImgBuffer.pheight * pInstance->sImgBuffer.bpl;
    pChromaDst = pInstance->sImgBuffer.base +
                 pInstance->iBufferLines * pInstance->sImgBuffer.bpl;
    
    /* Move the chroma plane up under the valid luma data */             
    GfxCopyRectMem((u_int32 *)pChromaDst,
                   (u_int32 *)pChromaSrc,
                   pInstance->sImgBuffer.bpl,
                   pInstance->sImgBuffer.bpl,
                   pInstance->sImgBuffer.pwidth,
                   pInstance->iBufferLines/2,
                   0,0,0,0, 8);
                   
    /* Fix up the size of the region so other code can correctly calculate */
    /* where the chroma plane is now.                                      */
    iWidth = vidGetOptions(pInstance->hBuffer, VO_BUFFER_WIDTH);               
    vidSetVideoRgnDimensions(pInstance->hBuffer, 
                             iWidth,
                             pInstance->iBufferLines);
    
  }
}

/********************************************************************/
/*  FUNCTION:    fill_420_buffer_rect_with_black                    */
/*                                                                  */
/*  PARAMETERS:  pDest   - Pointer to top left of dest image        */
/*               iPitch  - Pitch of dest image in bytes             */
/*               iHeight - Height of dest image (y plane) in pixels */
/*               iExtX   - Width of rectangle to fill               */
/*               iExtY   - Height of rectangle to fill              */
/*               iX      - Left coordinate of rectangle to fill     */
/*               iY      - Top line of rectangle to fill            */
/*                                                                  */
/*  DESCRIPTION: Fill a subrectangle of a 420 image buffer with     */
/*               blackness. This function deals with filling the    */
/*               correct subrectangles in the luma and chroma       */
/*               planes.                                            */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
static void fill_420_buffer_rect_with_black(u_int8 *pDest, int iPitch, int iHeight,
                                            int iExtX, int iExtY,
                                            int iX,    int iY)
{                                     
   GfxFillMem((u_int32 *)pDest,
              iPitch,
              iExtX,
              iExtY,
              0,
              iX,
              iY,
              8,
              GFX_ROP_COPY);
   
   /* Adjust pointer to start of chroma plane */
   pDest += iPitch * iHeight;
   
   GfxFillMem((u_int32 *)pDest,
              iPitch,
              iExtX,
              iExtY/2,
              0x80808080,
              iX,
              iY/2,
              8,
              GFX_ROP_COPY);

}

#ifdef USE_INTERNAL_READ_HEADER

#define BIG_ENDIAN_WORD(x) ((int)(*(u_int8 *)(x)) * 256 + (int)(*((u_int8 *)x + 1)))

/********************************************************************/
/*  FUNCTION:    jpeg_read_header                                   */
/*                                                                  */
/*  PARAMETERS:  pJpegBuffer  -                                     */
/*               pImageBuffer -                                     */
/*                                                                  */
/*  DESCRIPTION: A plug-in replacemend for armjpeg_read_header      */
/*               which, in the latest incarnation of the library,   */
/*               no longer returns vital information on the image   */
/*               subsampling.                                       */
/*                                                                  */
/*  RETURNS:     As armjpeg_read_header                             */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context.       */
/*                                                                  */
/********************************************************************/
static armjpeg_error jpeg_read_header(jpeg_buffer *pJpegBuffer, image_buffer *pImageBuffer) 
{
  u_int8 *ptrCurrent;
  int     iSkip;
  armjpeg_error eRetcode = ERR_ok;
  
  ptrCurrent = (u_int8 *)pJpegBuffer->base;
  
  /* Move to the first JFIF marker in the data */
  ptrCurrent = AdvanceToNextMarker(ptrCurrent, pJpegBuffer->base+pJpegBuffer->size);
  
  /* Scan through the markers looking for the ones we are interested in */
  while((eRetcode == ERR_ok) && ptrCurrent)
  {
      eRetcode = ProcessMarker(ptrCurrent, pJpegBuffer, pImageBuffer, &iSkip);  
      ptrCurrent+=iSkip;
      ptrCurrent = AdvanceToNextMarker(ptrCurrent, pJpegBuffer->base+pJpegBuffer->size);
  }
  return(eRetcode);
}

/**********************************************************/
/* Move forward to the next valid 0xFF marker in the file */
/**********************************************************/
static u_int8 *AdvanceToNextMarker(u_int8 *pCurrent, u_int8 *pEnd)
{
    u_int8 *pMoving = pCurrent;

    /* Find the next 0xFF from the current position */
    while(1)
    {
      while((pMoving < (pEnd-1)) && (*pMoving != 0xFF))
          pMoving++;

      /* If we fell off the end of the buffer, we're done */
      if(pMoving == pEnd)
          return(NULL);

      /* If we found a valid marker, return the pointer */
      if((*(pMoving+1) != 0xFF) && (*(pMoving+1) != 0x00))
          return(pMoving+1);
      else
          pMoving++;
    }
}

/**********************************************************/
/* Do whatever is necessary to process individual markers */
/**********************************************************/
static armjpeg_error ProcessMarker(u_int8 *pMarker, jpeg_buffer *pJpeg, image_buffer *pImage, int *pSkip)
{
    bool    bNoLength = FALSE;
    int     iComLength;
    u_int8  cTemp;
    armjpeg_error eRetcode = ERR_ok;
    u_int8  component_subsampling[3];
    
    switch(*pMarker)
    {
      /* Markers we are interested in */
      case 0xC0:    
      case 0xC2:

        pImage->height = BIG_ENDIAN_WORD(pMarker + 4);
        pImage->width  = BIG_ENDIAN_WORD(pMarker + 6);
        pJpeg->components  = *(pMarker+8);
        
        /* Too many components */
        if((pJpeg->components != 1) && (pJpeg->components != 3))  
          eRetcode = ERR_CONFIG_components;
        else
        {  
          component_subsampling[0] = (u_int8)0;
          component_subsampling[1] = (u_int8)0;
          component_subsampling[2] = (u_int8)0;
          
          for(iComLength = 0; iComLength < (int)pJpeg->components; iComLength++)
          {
            cTemp = *(pMarker + 9 + 3*(iComLength)); /* Component type */
            if (cTemp && (cTemp < 4))
            {
              component_subsampling[cTemp-1] = *(pMarker + 10 + 3*(iComLength));
              
              /* Look for invalid values */
              if((component_subsampling[cTemp-1] != 0x11) &&
                 (component_subsampling[cTemp-1] != 0x22) &&
                 (component_subsampling[cTemp-1] != 0x12) &&
                 (component_subsampling[cTemp-1] != 0x21))
                eRetcode = ERR_COL_not_supported;    
            }  
            else
            {
              /* Unrecognised or unsupported component type found */
              eRetcode = ERR_SOS_component;
              break;
            }  
          }
          
          /* We now have the subsampling factors for each component */
          /* Map this information into the correct ARM JPEG enum    */
          /* values.                                                */
          
          if (eRetcode == ERR_ok)
          {
            /* First. mono images */
            if (pJpeg->components == 1)
            {
              if(component_subsampling[0] == 0x11)
                pJpeg->samp = JPEGS_1H1V;
              else
                eRetcode = ERR_COL_not_supported;
            }
            else
            {
              /* Colour cases */
            
              /* We only support cases where Cr and Cb have the same subsampling */
              if (component_subsampling[1] != component_subsampling[2])
              {
                eRetcode = ERR_COL_not_supported;
              }
              else
              {
                /* Consider the Y subsampling first then look at Cr and Cb */
                switch (component_subsampling[0])
                {
                  case 0x11:
                   if(component_subsampling[1] == 0x11)
                     pJpeg->samp = JPEGS_1H1V;
                   else
                     eRetcode = ERR_COL_not_supported;
                   break;
                       
                  case 0x22:
                   switch (component_subsampling[1])
                   {
                     case 0x11:
                       pJpeg->samp = JPEGS_2H2V;
                       break;
                     case 0x12:
                       pJpeg->samp = JPEGS_1H2V;
                       break;
                     case 0x21:
                       pJpeg->samp = JPEGS_2H1V;
                       break;
                     case 0x22:
                       pJpeg->samp = JPEGS_2H2V;
                       break;
                   }
                   break;
                  
                  case 0x21:
                  case 0x12:
                    eRetcode = ERR_COL_not_supported;
                    break;
                }  
              }
            }      
          }  
        }
        
        /* Progressive JPEGs not supported */
        if(*pMarker != 0xC0)
          eRetcode = ERR_DNL_not_supported;
          
        /* Not 8 bits per sample */
        if(*(pMarker+3) != 8)
          eRetcode = ERR_SOF_precision;
        
        break;

      /* Error case - can't support arithmetic encoding */
      case 0xC9:    
        eRetcode = ERR_DAC_not_supported;
        break;
      
        break;
        
      /* Markers we ignore but which do not have payload sizes following them */
      case 0x01:
      case 0xD0:  
      case 0xD1:  
      case 0xD2:  
      case 0xD3:  
      case 0xD4:  
      case 0xD5:  
      case 0xD6:  
      case 0xD7:  
      case 0xD8:  
      case 0xD9:  
        bNoLength = TRUE;
        break;  
        
      /* Markers we ignore which do have payload sizes following them */  
      case 0xC1:    
      case 0xC3:    
      case 0xC5:    
      case 0xC6:    
      case 0xC7:    
      case 0xCA:    
      case 0xCB:    
      case 0xCD:    
      case 0xCE:    
      case 0xCF:    
      case 0xC4:    
      case 0xCC:    
      case 0xC8:    
      case 0xDA:  
      case 0xDB:
      case 0xDC: 
      case 0xDD:  
      case 0xDE:  
      case 0xDF:  
      case 0xE0:  
      case 0xE1:  
      case 0xE2:  
      case 0xE3:  
      case 0xE4:  
      case 0xE5:  
      case 0xE6:  
      case 0xE7:  
      case 0xE8:  
      case 0xE9:  
      case 0xEA:  
      case 0xEB:  
      case 0xEC:  
      case 0xED:  
      case 0xEE:  
      case 0xEF:  
      case 0xF0:  
      case 0xFD:  
      case 0xFE:  
        break;
        
      /* Unrecognised markers */  
      default:
        iComLength = BIG_ENDIAN_WORD(pMarker+1);  
        trace_new(JPG_ERROR, "JPEG:  Unrecognised marker 0x%02x, Payload length %d bytes\n", *pMarker, iComLength);
        break;  
    }
    
    /* Return the number of bytes to skip over this section */
    if (bNoLength)
    {
      *pSkip = 1;
      return(eRetcode);
    }  
    else
    {
      iComLength = BIG_ENDIAN_WORD(pMarker+1)+1;  
      *pSkip = iComLength;
      return(eRetcode);
    }
}

#endif /* USE_INTERNAL_READ_HEADER */

#ifdef STUB_OUT_ARM_LIBRARY

/***************************************************************************/
/* During rework involved in changing toolchains, it is useful to be able  */
/* to stub out the ARM JPEG library and try to get other parts of the      */
/* system linking. Hence, here I include stubs for all the entry points    */
/* we use. Building with STUB_OUT_ARM_LIBRARY means that the driver        */
/* will not work but, hopefully, we can get the image to link at least.    */
/***************************************************************************/

char * APICALL armjpeg_error_message(armjpeg_error e)
{
  return("STUBBED OUT");
}  

int APICALL armjpeg_create(armjpeg_handle *h, armjpeg_config *c, armjpeg_byte *mem)
{
  return(1024);
}

armjpeg_error APICALL armjpeg_destroy(armjpeg_handle h)
{
  return(ERR_ok);
}

armjpeg_error APICALL armjpeg_read_header(jpeg_buffer *jpeg, image_buffer *image)
{
  return(ERR_ok);
}
armjpeg_error APICALL armjpeg_decompress_start( armjpeg_handle h,
                                                jpeg_buffer *jpeg,
                                                image_buffer *image)
{
  return(ERR_ok);
}
armjpeg_error APICALL armjpeg_decompress_some(armjpeg_handle h, int nummcu)
{
  return(ERR_ok);
}
armjpeg_error APICALL armjpeg_decompress_stop(armjpeg_handle h)
{
  return(ERR_ok);
}
#endif
