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
#ifndef _IPANEL_MIDDLEWARE_PORTING_TYPEDEF_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_TYPEDEF_API_FUNCTOTYPE_H_

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_OK               0
#define IPANEL_ERR             -1

#define IPANEL_NULL             0

#define VOID                    void
#define CONST                   const

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
#ifndef INT32_T
typedef long                    INT32_T;
#endif

#ifndef UINT32_T
typedef unsigned long           UINT32_T;
#endif

#ifndef INT16_T
typedef short                   INT16_T;
#endif

#ifndef UINT16_T
typedef unsigned short          UINT16_T;
#endif

#ifndef CHAR_T
typedef char                    CHAR_T;
#endif

#ifndef BYTE_T
typedef unsigned char           BYTE_T;
#endif

#ifndef UINT64_T
typedef unsigned long long      UINT64_T;
#endif

#ifndef INT64_T 
typedef signed long long        INT64_T;
#endif

#ifndef u_int64
typedef unsigned long long      u_int64;
#endif

typedef struct
{
    UINT32_T    uint64_32h;
    UINT32_T    uint64_32l;
} IPANEL_UINT64_T;

typedef struct
{
    INT32_T     int64_32h;
    UINT32_T    int64_32l;
} IPANEL_INT64_T;

typedef enum
{
    IPANEL_AV_SOURCE_DEMUX,
    IPANEL_AV_SOURCE_MANUAL
} IPANEL_AV_SOURCE_TYPE_e;

typedef enum
{
    IPANEL_DISABLE,
    IPANEL_ENABLE
} IPANEL_SWITCH_e;

typedef enum
{
    IPANEL_XMEM_PCM = 1,
    IPANEL_XMEM_MP3 = 2,
    IPANEL_XMEM_TS  = 3,
    IPANEL_XMEM_ES  = 4,
    IPANEL_XMEM_GEN = 5
} IPANEL_XMEM_PAYLOAD_TYPE_e;

typedef struct
{
	IPANEL_XMEM_PAYLOAD_TYPE_e  destype;
	UINT32_T                    len;
} IPANEL_XMEM_GEN_DES;

typedef VOID (*IPANEL_XMEM_FREE)(VOID *pblk);

typedef struct
{
	IPANEL_XMEM_PAYLOAD_TYPE_e  destype;
	UINT32_T                    samplerate;
	UINT16_T                    channelnum;
	UINT16_T                    bitspersample;
	UINT16_T                    bsigned;
	UINT16_T                    bmsbf;
	UINT32_T                    samples;
} IPANEL_PCMDES, *pIPANEL_PCMDES;

typedef struct
{
    VOID               *pdes;
    IPANEL_XMEM_FREE    pfree;
    UINT32_T           *pbuf;
    UINT32_T            len;
} IPANEL_XMEMBLK, *pIPANEL_XMEMBLK;


typedef enum
{
	IPANEL_EVENT_TYPE_TIMER    =0 ,
	IPANEL_EVENT_TYPE_SYSTEM = 1,
	IPANEL_EVENT_TYPE_KEYDOWN =2,
	IPANEL_EVENT_TYPE_KEYUP =3,
	IPANEL_EVENT_TYPE_NETWORK =4,
	IPANEL_EVENT_TYPE_CHAR = 5,
	IPANEL_EVENT_TYPE_MOUSE =6,
	IPANEL_EVENT_TYPE_IRKEY =7,
	IPANEL_EVENT_TYPE_DVB          = 0x100
}IPANEL_EVENT_MSG;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//

#endif  // _IPANEL_MIDDLEWARE_PORTING_TYPEDEF_API_FUNCTOTYPE_H_

