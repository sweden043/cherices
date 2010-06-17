/* 
 * $Copyright: 
 * ----------------------------------------------------------------
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 *   (C) COPYRIGHT 1997,1998,1999,2000,2001,2002 ARM Limited
 *       ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 * ----------------------------------------------------------------
 * File:     armjapi.h,v
 * Revision: 1.23
 * ----------------------------------------------------------------
 * $
 * 
 * ARM JPEG API header file
 */
 
/* All library definitions are contained in this header which should
 * be included by any program wishing to call the library. Depending
 * on how the  library has been compiled, some features described may
 * not be available. If you have a source release then edit the file
 * "options.h" to determine which features are included in the library.
 *
 * For further documentation see the file "JPEG-API.pdf"
 */

#ifndef _ARM_JPEG_API_
#define _ARM_JPEG_API_

/* Type definitions */
typedef unsigned char armjpeg_byte;
typedef struct armjpeg_info *armjpeg_handle;

/* Some systems require externally available calls to be specially
 * marked. The APICALL macro can be used to set this marker for one
 * such system. Most users can leave this macro definition blank.
 */
#ifdef _WIN32_WCE
  #include <windows.h>
  #define APICALL CALLBACK
#else
  #define APICALL
#endif

/*******************************************/ 
/* COLOUR CONFIGURATION & PALETTE HANDLING */
/*******************************************/ 

/* If a pixel type with configurable palette is used (such as PIX_COL8) then
 * you will need to inform the JPEG library of the palette contents and set
 * up various colour tables.
 * 
 * An rgb palette entry has the following format.
 */

typedef struct {
  armjpeg_byte r;   /* red level 0-255 */
  armjpeg_byte g;   /* green level 0-255 */
  armjpeg_byte b;   /* blue level 0-255 */
  armjpeg_byte res; /* reserved byte */
} rgb_entry;

/* A palette consist of a list of rgb_entry entries, the n'th entry giving
 * the colour components for a pixel numbered 'n'. To convert in the inverse
 * direction, from an rgb_entry to the closest possible pixel number a function
 * of the following type is used:
 */

typedef int rgb_function(armjpeg_handle h, int r, int g, int b);

/* The function is called with the current instantiation handle, and the red,
 * green, blue level to match and should return the best pixel number. These
 * levels may be OUTSIDE the range 0-255. For example, if r=-100 then any match
 * should force r=0, where as r=+300 gives a strong indication that the closest
 * match will have r=255. It is essential - for a good colour match and good
 * dithering within the decompressor - that out of range colour values are NOT
 * clipped before doing the colour match.
 * 
 * If no colour function is provided then the default, rather slow, colour
 * matching function is used (to set up the tables - speed of decompression will
 * not be affected). This simply goes through the palette and chooses
 * the pixel number which minimises the value:
 * 
 *     2*(red_difference)^2 + 4*(green_difference)^2 + (blue_difference)^2
 *     
 */

/* IMAGE BUFFER DEFINITION */

/* Pixel types */

enum
{
  PIX_GREY8,    /* 8 bits per pixel & grey palette 0=black 255=white */
  PIX_COL8,     /* 8 bits per pixel & configurable (colour) palette */
  PIX_COL16,    /* 16 bits per pixel & 5 bits red,green,blue palette */
  PIX_COL24,    /* 24 bits per pixel & 8 bits red,green,blue palette */
  PIX_COL32,    /* 32 bits per pixel & 8 bits red,green,blue,0 palette */
  PIX_YCBCR,    /* NOT SUPPORTED */
  PIX_END
};

typedef int pixel_t;

/* To use the pixel formats which don't have a fixed palette (currently only
 * PIX_COL8) you will need to declare the palette (via the funtion
 * armjpeg_set_colour_tables) prior to using the rest of the JPEG library.
 * 
 * An image buffer is defined by the following structure.
 * The image buffer is of dimension pwidth x pheight (and thus pheight*bpl bytes
 * long). The image it contains need not take up the whole buffer (eg if the
 * buffer is some screen RAM) and is of dimension width x height.
 */

typedef struct {
  armjpeg_byte *base;   /* pointer to start of the buffer */
  armjpeg_byte *ptr;    /* current image pointer (line start) */
  pixel_t      format;  /* describes the format of an image pixel */
  int          bpp;     /* number of bits per image pixel */
  int          scalex;  /* horizontal rescale factor multiplied by 8 */
  int          scaley;  /* vertical rescale factor multiplied by 8 */
  int          pwidth;  /* width of the buffer in pixels */
  int          pheight; /* height of the buffer in lines */
  int          bpl;     /* number of bytes per image line (multiple of 4) */
  int          width;   /* width of the image (if buffer contains an image) */
  int          height;  /* height of the image (if buffer contains an image) */
  int          remx;    /* number of pixels left on current image row */
  int          remy;    /* number of image lines left */
  rgb_entry    *palette;/* palette used for some image formats */
} image_buffer;


/* JPEG BUFFER DEFINITION */

/* JPEG features */

typedef int feature_t;

enum
{
  JPEGF_none        = 0,        /* No features supported */
  JPEGF_compress    = (1<<0),   /* Baseline (DCT & Huffman) compression */
  JPEGF_decompress  = (1<<1),   /* Baseline (DCT & Huffman) decompression */
  JPEGF_extended    = (1<<2),   /* Extended sequential DCT (4 Huffman tables)*/
  JPEGF_restarts    = (1<<3),   /* NOT SUPPORTED */
  JPEGF_progressive = (1<<4),   /* NOT SUPPORTED */
  JPEGF_arithmetic  = (1<<5),   /* NOT SUPPORTED */
  JPEGF_lossless    = (1<<6),   /* NOT SUPPORTED */
  JPEGF_reflect     = (1<<7),   /* reflect funny sized images - NOT SUPPORTED */
  /* composite flags */
  JPEGF_duplex = (JPEGF_compress | JPEGF_decompress),
  JPEGF_END
};
 
/* YCbCr sampling formats */

typedef int samp_t;

enum
{
  JPEGS_2H2V = 0x22,   /* downsample Cb/Cr by 2 on both axes */
  JPEGS_2H1V = 0x21,   /* Cb/Cr vertically downsampled */
  JPEGS_1H2V = 0x12,   /* Cb/Cr horizontally downsampled */
  JPEGS_1H1V = 0x11,   /* no downsampling */
  JPEGS_END
};

/* Thumbnail extension codes used by JFIF 1.02 */

enum
{
  THUMB_NONE = 0,       /* No thumbnail */
  THUMB_JPEG = 0x10,    /* Thumbnail encoded using JPEG */
  THUMB_1BPP = 0x11,    /* Thumbnail stored at 1 byte per pixel */
  THUMB_3BPP = 0x13     /* Thumbnail stored at 3 bytes per pixel */
};

/* The JPEG buffer structure */

typedef struct {
  armjpeg_byte *base;           /* address of the jpeg file start */
  armjpeg_byte *ptr;            /* current position in the output buffer */
  int           size;           /* size of the buffer (for overflow detect) */
  int           blk_width;      /* width of the smallest decode block in pixels */
  int           blk_height;     /* height of the smallest decode block in pixels */
  samp_t        samp;           /* downsampling format */
  feature_t     features;       /* additional features to use */
  int           restarti;       /* length of a restart interval (if used) */
  armjpeg_byte  *thumb_ptr;     /* JFIF thumbnail address in the header */
  armjpeg_byte  thumb_type;     /* JFIF thumbnail type (0=none) */
  armjpeg_byte  components;     /* number of colour components stored in the file */
  armjpeg_byte  quality;        /* image quality (0-100%) */
} jpeg_buffer;


/* ERROR HANDLING */

/* The following enum defines the recognised error codes. The comments are
 * used as error strings and the #define enables them to be used elsewhere.
 */

#define ERROR_TYPES() \
 ERROR( ERR_ok=0,               "No error" ) \
 ERROR( ERR_dst_full=-255,      "Destination buffer full" ) \
 ERROR( ERR_Unsupported,        "Unsupported feature used" ) \
 ERROR( ERR_short_JPEG_hdr,     "JPEG header not complete" ) \
 ERROR( ERR_SOI_none,           "Start of image marker missing" ) \
 ERROR( ERR_bad_marker,         "Next marker doesn't start 0xFF") \
 ERROR( ERR_DAC_not_supported,  "Arithmetic coding used (not supported)") \
 ERROR( ERR_DRI_not_supported,  "Restart intervals not supported") \
 ERROR( ERR_DNL_not_supported,  "Define number of lines used (not supported)") \
 ERROR( ERR_SOF_precision,      "Unsupported DCT precision used") \
 ERROR( ERR_SOF_not_supported,  "Unsupported coding method used") \
 ERROR( ERR_SOF_component,      "Illegal specifiers for component in SOF section") \
 ERROR( ERR_DHT_bad_table,      "Bad Huffman table number in DHT section") \
 ERROR( ERR_DHT_bad_codes,      "Too many Huffman symbols in DHT section") \
 ERROR( ERR_DQT_bad_table,      "Bad Quantisation table number in DQT section") \
 ERROR( ERR_SOS_component,      "Too many components specified in SOS section") \
 ERROR( ERR_SOS_huffman,        "Bad Huffman table number in SOS section") \
 ERROR( ERR_SOS_samp,           "Colour components 1,2 (Cb,Cr) not sampled at 8x8 in MCU") \
 ERROR( ERR_Out_of_memory,      "Memory claim failed (out of memory)") \
 ERROR( ERR_Buffer_too_small,   "JPEG output buffer full") \
 ERROR( ERR_CONFIG_components,  "Too many image components (more than cofigured for)" ) \
 ERROR( ERR_MCU_not_supported,  "Unsupported MCU type used") \
 ERROR( ERR_COL_not_supported,  "Unsupported pixel type for current MCU") \
 ERROR( ERR_PIX_unknown,        "Unknown pixel type selected") \
 ERROR( ERR_bad_scale,          "Scale factor not supported") \
 /* blank line */

 /* now define the typedef */
#undef ERROR
#define ERROR(a,b) a,
typedef enum {
  ERROR_TYPES()
  ERR_END
} armjpeg_error;
 
/* The following function takes a standard error code and returns a pointer to a
 * zero terminated textual message describing the error.
 */
 
char * APICALL armjpeg_error_message(armjpeg_error e);

/******************/
/* INITIALISATION */
/******************/

/* Before use each JPEG instantiation must be created and initialised. The
 * initialisation routines take a pointer to an 'armjpeg_config' structure
 * describing the limits expected of the instantiation. This determines the
 * amount of memory the JPEG instantiation will need.
 */

typedef struct {
  int max_components;   /* maximum number of colour components */
  feature_t features;   /* features to be supported */
} armjpeg_config;

/* This function creates an instantiation with features given by c and writes
 * the handle to *h. The instantiation uses memory starting at address mem and
 * returns the number of bytes used. If mem==NULL then no instantiation is
 * created, but the number of bytes needed is still returned. Hence this
 * function can also be used to determine the size of workspace required.
 * If the integer returned is negative then it is an error code of type
 * armjpeg_error.
 */
int APICALL armjpeg_create(armjpeg_handle *h, armjpeg_config *c, armjpeg_byte *mem);

/* This function is similar to armjpeg_create except that it claims the
 * workspace for you via the malloc library command. It calls armjpeg_create
 * twice, first to determine the size to malloc (by calling with mem=NULL) and
 * secondly to create the instantiation. It returns the number of bytes claimed
 * or a negative armjpeg_error code as armjpeg_create does.
 */
int APICALL armjpeg_claim(armjpeg_handle *h, armjpeg_config *c);


/* This destroys an armjpeg instantiation. If the memory was malloced (via using
 * armjpeg_claim) then the memory used is also freed.
 */
armjpeg_error APICALL armjpeg_destroy(armjpeg_handle h);


/******************/
/*   COMPRESSION  */
/******************/


/* This function starts compression for a given image. The image_buffer
 * structure contains details of the image to be compressed and the jpeg_buffer
 * structure details of the buffer to write the JFIF file to. The pointers jpeg
 * and image are copied and the two stuctures must exist until
 * armjpeg_compress_stop is called.
 */
armjpeg_error APICALL armjpeg_compress_start(
  armjpeg_handle h,
  jpeg_buffer   *jpeg,
  image_buffer  *image
);


/* Continue with JPEG compression. Compress up to the next 'nlines' of the input
 * image (which must be a multiple of 16). The lines are read from the current
 * position (specified by ptr - you can change this) and written to the current
 * output position. If nlines<=0 then the whole image is compressed.
 * 
 * The 'ptr' fields in both image_buffer and jpeg_buffer structures are updated.
 */
armjpeg_error APICALL armjpeg_compress_some(armjpeg_handle h, int nlines);


/* The following function frees up the compressor for another image, and writes
 * the tail end markers of the JFIF file. The 'ptr' field of the jpeg_buffer is
 * updated.
 */
armjpeg_error APICALL armjpeg_compress_stop(armjpeg_handle h);

/*****************/
/* DECOMPRESSION */
/*****************/


/* Before decompression, or even initialising the JPEG library, it is often
 * useful to know the dimensions and type of image being decompressed. The
 * following call examines a JPEG file header and returns information about
 * its contents.
 */
armjpeg_error APICALL armjpeg_read_header(jpeg_buffer *jpeg, image_buffer *image);


/* This function starts decompression for a given image, by reading the header
 * of the JFIF file and setting up any tables required for this image. Again the
 * decompressor takes a copy of the pointers to jpeg_buffer and image_buffer so
 * these structures must exist until armjpeg_decompress_stop has been called.
 */
armjpeg_error APICALL armjpeg_decompress_start(
  armjpeg_handle h,
  jpeg_buffer *jpeg,
  image_buffer *image
);

/* Continue with JPEG decompression. Decompress up to the next nummcu
 * blocks of the output image. If nummcu<0 then the whole image is
 * decompressed. The image is read from the ptr in the jpeg_buffer and
 * written to the ptr in the image_buffer. The image buffer ptr may be altered
 * between calls.
 */
armjpeg_error APICALL armjpeg_decompress_some(armjpeg_handle h, int nummcu);

/* The following function frees up the decompressor for another image. It also
 * checks the end marker of the JFIF file.
 */
armjpeg_error APICALL armjpeg_decompress_stop(armjpeg_handle h);

/* This function generates the colour tables needed for decompression. The
 * colour tables are of course fixed given a fixed palette and so can be placed
 * in ROM or read in from a file without being calculated for a specific
 * application.
 */
armjpeg_error APICALL armjpeg_create_colour_tables(
        armjpeg_handle h,       /* instantiation handle */
        rgb_entry *palette,     /* palette */
        rgb_function *rgbfn,    /* colour conversion function as above */
        int n);                 /* number of entries in the palette */


#endif  /* _ARM_JPEG_API_ */
