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
#ifndef _IPANEL_MIDDLEWARE_PORTING_TUNER_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_TUNER_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
	TUNER_IDLE,
	TUNER_NEW_INPUT,
	TUNERRING,
	TUNER_FAIL,
	TUNER_SUCCESS,
	TUNER_LOST_SIGN,
	TUNER_RECOVER_SIGN,
	TUNER_UNDEFINE
} TunerStatus;

typedef struct
{
	INT32_T     frequency;
	INT32_T     symbol_rate;
	INT32_T     modulation;	
	INT32_T     request_id;
	unsigned int start_tunering; /*是否要求锁频标记，1为是,0为否*/
} STunerPara;

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
    IPANEL_MODULATION_QAM16     = 1,
    IPANEL_MODULATION_QAM32     = 2,
    IPANEL_MODULATION_QAM64     = 3,
    IPANEL_MODULATION_QAM128    = 4,
    IPANEL_MODULATION_QAM256    = 5
} IPANEL_MODULATION_MODE_e;

typedef enum
{
    IPANEL_TUNER_GET_QUALITY    = 1,
    IPANEL_TUNER_GET_STRENGTH   = 2,
    IPANEL_TUNER_GET_BER        = 3,
    IPANEL_TUNER_GET_LEVEL      = 4,
    IPANEL_TUNER_GET_SNR        = 5,
    IPANEL_TUNER_GET_STATUS     = 6,
    IPANEL_TUNER_LOCK           = 7
} IPANEL_TUNER_IOCTL_e;

typedef enum
{
    IPANEL_TUNER_LOST           = 0,
    IPANEL_TUNER_LOCKED         = 1,
    IPANEL_TUNER_LOCKING        = 2
} IPANEL_TUNER_STATUS_e;

typedef enum
{
    IPANEL_ACC_NONE     = 0,
    IPANEL_ACC_DVB_C    = 1,
    IPANEL_ACC_DVB_S    = 2,
    IPANEL_ACC_DVB_T    = 3
} IPANEL_ACCESS_TYPE_e;

typedef struct
{
    UINT32_T                        tuningfreqhz;
    UINT32_T                        symbolrate;
    IPANEL_MODULATION_MODE_e        qam;
} IPANEL_DVBC_PARAMS;

typedef enum
{
    IPANEL_PLR_UNDEFINE     = 0,
    IPANEL_PLR_HORIZONTAL   = 1,
    IPANEL_PLR_VERTICAL     = 2,
    IPANEL_PLR_LEFT         = 3,
    IPANEL_PLR_RIGHT        = 4
} IPANEL_POLARIZATION_e;

typedef enum
{
    IPANEL_IMQ_UNDEFINE     = 0,
    IPANEL_IMQ_POSITIVE     = 1,
    IPANEL_IMQ_NEGATIVE     = 2,
    IPANEL_IMQ_AUTO         = 3
} IPANEL_IMQ_VALUE_e;

typedef enum
{
    IPANEL_VCR_1_2          = 0,
    IPANEL_VCR_2_3          = 1,
    IPANEL_VCR_3_4          = 2,
    IPANEL_VCR_5_6          = 3,
    IPANEL_VCR_6_7          = 4,
    IPANEL_VCR_7_8          = 5,
    IPANEL_VCR_AUTO         = 6
} IPANEL_CODE_RATE_e;

typedef struct
{
    UINT32_T                        tuningfreqhz;
    UINT32_T                        symbolrate;
    IPANEL_MODULATION_MODE_e        modulation;
    IPANEL_POLARIZATION_e           polarization;
    IPANEL_IMQ_VALUE_e              imqsign;
    IPANEL_CODE_RATE_e              viterbicoderate;
} IPANEL_DVBS_PARAMS;

typedef enum
{
    IPANEL_BANDWIDTH_UNDEFINE       = 0,
    IPANEL_COFDM_BANDWIDTH_8        = 1,
    IPANEL_COFDM_BANDWIDTH_7        = 2,
    IPANEL_COFDM_BANDWIDTH_6        = 3
} IPANEL_COFDM_BANDWIDTH_e;

typedef enum
{
    IPANEL_HIERARCHY_UNDEFINE       = 0,
    IPANEL_COFDM_HIER_NONE          = 1,
    IPANEL_COFDM_HIER_ALPHA_1       = 2,
    IPANEL_COFDM_HIER_ALPHA_2       = 3,
    IPANEL_COFDM_HIER_ALPHA_4       = 4
} IPANEL_COFDM_HIERARCHY_e;

typedef enum
{
    IPANEL_GUARDINTERVAL_UNDEFINE   = 0,
    IPANEL_COFDM_GI_1_32            = 1,
    IPANEL_COFDM_GI_1_16            = 2,
    IPANEL_COFDM_GI_1_8             = 3,
    IPANEL_COFDM_GI_1_4             = 4
} IPANEL_COFDM_GUARDINTERVAL_e;

typedef struct
{
    UINT32_T                        tuningFreqHz;
    IPANEL_COFDM_BANDWIDTH_e        bandwidth;
    IPANEL_MODULATION_MODE_e        modulation;
    IPANEL_COFDM_HIERARCHY_e        hierarchyinfo;
    IPANEL_CODE_RATE_e              hpcode;
    IPANEL_CODE_RATE_e              lpcode;
    IPANEL_COFDM_GUARDINTERVAL_e    guardinterval;
    IPANEL_MODULATION_MODE_e        transmode;
} IPANEL_DVBT_PARAMS;

typedef struct
{
    UINT32_T                id;
    IPANEL_ACCESS_TYPE_e    type;
    union
    {
        IPANEL_DVBC_PARAMS  dvbc;
        IPANEL_DVBS_PARAMS  dvbs;
        IPANEL_DVBT_PARAMS  dvbt;
    } transport;
} IPANEL_ACC_TRANSPORT;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
INT32_T ipanel_porting_tuner_lock_delivery(
        INT32_T tunerid,
        INT32_T frequency,
        INT32_T symbol_rate,
        INT32_T modulation,
        INT32_T request_id
    );

INT32_T ipanel_porting_tuner_ioctl(INT32_T tunerid, IPANEL_TUNER_IOCTL_e op, VOID *arg);

INT32_T ipanel_porting_tuner_get_status(INT32_T tunerid);

int  ipanel_tuner_init(void);

void  ipanel_tuner_exit(void);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_TUNER_API_FUNCTOTYPE_H_

