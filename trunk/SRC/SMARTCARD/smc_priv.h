/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        smc_priv.h
 *
 *
 * Description:     Public header file for smart card driver
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: smc_priv.h, 9, 9/17/03 3:58:46 PM, Lucy C Allevato$
 ****************************************************************************/

#ifndef _SMC_PRIV_H_
#define _SMC_PRIV_H_


/********************************/
/* Symbol and Macro definitions */
/********************************/
/* #define SCARD_FIX */

#define NUM_SCARD_CLIENTS        SMC_NUM_CLIENTS

#if (SMC_SLOT0 != SMC_NONE) && (SMC_SLOT1 != SMC_NONE)

   #define NUM_SCARD_SLOTS       2
   #if (SMC_SLOT0 == SMC_CONTROLLER0)
     #define BANK_FROM_SLOT_ID(s) (s)
     #define INT_VENDOR_SLOT_0  INT_SCD0
     #define INT_VENDOR_SLOT_1  INT_SCD1
   #else
     #define BANK_FROM_SLOT_ID(s) (1-(s))
     #define INT_VENDOR_SLOT_0  INT_SCD1
     #define INT_VENDOR_SLOT_1  INT_SCD0
   #endif

#elif (SMC_SLOT0 == SMC_NONE) && (SMC_SLOT1 == SMC_NONE)

   #define NUM_SCARD_SLOTS       0

#elif (SMC_SLOT0 != SMC_NONE) && (SMC_SLOT1 == SMC_NONE)

   #define NUM_SCARD_SLOTS       1
   #define INT_VENDOR_SLOT_1  0xffffffff
   #if (SMC_SLOT0 == SMC_CONTROLLER0)
     #define BANK_FROM_SLOT_ID(s) (s)
     #define INT_VENDOR_SLOT_0  INT_SCD0
   #else
     #define BANK_FROM_SLOT_ID(s) (1-(s))
     #define INT_VENDOR_SLOT_0  INT_SCD1
   #endif

#else
   #error Using SMC_SLOT1 without SMC_SLOT0 is unsupported
#endif

#define SMC_DISABLE 0
#define SMC_ENABLE  1
#define SMC_CLEAR   1

#define SMC_REF_CLK       54000000

#define RFU        -1

#if NUM_SCARD_SLOTS == 1
#define IS_VALID_SLOT_ID(s) ((s)==0)
#else
#define IS_VALID_SLOT_ID(s) (((s)==0) || ((s)==1))
#endif

#define SMC_ATR_MAX_LENGTH  32

#ifndef CNXT_SMC_NUM_UNITS
#define CNXT_SMC_NUM_UNITS  NUM_SCARD_SLOTS  /* change this as needed */
#endif

#ifndef CNXT_SMC_MAX_HANDLES
#define CNXT_SMC_MAX_HANDLES   5 /* !!!! change this as needed */
#endif

#ifndef CNXT_SMC_MAX_PENDING_RW
#define CNXT_SMC_MAX_PENDING_RW 5
#endif

/*****************/
/* Data Types    */
/*****************/
/* Instance data types */
typedef struct _cnxt_smc_inst
{
   CNXT_HANDLE_PREFACE   Preface; 
   CNXT_SMC_PFNNOTIFY    pNotifyFn; /* Function to handle Inst notifications */
   void                  *pUserData; /* Appl data needed by Inst */
   u_int32               uUnitNumber;

   /* Add all other handle (instance) specific data should be declared here */

} CNXT_SMC_INST;


/* Card descriptor type */
typedef enum
{
   SMC_NOT_INSERT,
   SMC_INSERT,
   SMC_MUTE,
   SMC_ATR_RECEIVED,
   SMC_ATR_PARSED
} SMC_STATE;

typedef enum
{
   SMC_VCC_3V,
   SMC_VCC_5V
} SMC_VCC_TYPE;

typedef struct
{
   u_int8  uConvention;
   u_int8  uProtocol;
   u_int8  uFI;  /* clock rate conversion rate */
   u_int8  uDI;  /* bit rate adjustment factor */
   u_int8  uPI1; /* programming voltage factor */
   u_int8  uPI2; /* programming voltage factor */
   u_int8  uII;  /* maximum programming current factor */
   u_int8  uN;   /* extra guardtime */
   u_int8  *pHistorical;
   u_int8  uHistoricalLength;
   u_int8  uRetry;
   u_int16 uTimeout;
} SMC_COMM_PARAMETERS;

typedef struct _smc_rw_job_descr
{
   struct _smc_rw_job_descr *pNext;           /* for queuing pending rw jobs */
   CNXT_SMC_INST            *pInst;
   bool                     bAsync;
   sem_id_t                 SyncSem;
   u_int8                   *pTxPtr;
   u_int32                  uBytesToTx;
   u_int8                   *pRxPtr;
   u_int32                  *pBytesRecved;
   u_int8                   *pRxBufEnd;
   CNXT_SMC_STATUS          Status;           /* result of the job */
   void                     *Tag;
} SMC_RW_JOB_DESCRIPTOR;

typedef struct
{
   CNXT_SMC_INST *pInst;
   bool          bAsync;
   sem_id_t      SyncSem;
   void          *Tag;
} SMC_CTRL_JOB_DESCRIPTOR;

typedef struct
{
   SMC_STATE                State;
   SMC_VCC_TYPE             Vcc;
   SMC_COMM_PARAMETERS      Config;
   SMC_CTRL_JOB_DESCRIPTOR  ResetJob;
   SMC_CTRL_JOB_DESCRIPTOR  PowerdownJob;
   SMC_RW_JOB_DESCRIPTOR    *pRwJob;    /* list of rw job pending for the card */
   u_int32                  uAtrLength;
   u_int8                   uAtr[SMC_ATR_MAX_LENGTH+1];
} SMC_DESCRIPTOR;

/* Clock rate table - indicates the F value to be used. */
typedef struct
{
   int Fi;
   int Fmax;
} SMC_CLK_RATE_TAG;

typedef struct
{
   CNXT_SMC_CAPS         *pCaps;
   bool                  bExclusive;
   SMC_DESCRIPTOR        *pCard;
   CNXT_SMC_INST         *pFirstInst;  /* Pointer to list of open Inst */
} CNXT_SMC_UNIT_INST;

typedef struct
{
   CNXT_SMC_INST         *pInstList;    /* Memory pool to get inst */
   bool                  bInit;
   sem_id_t              DriverSem;
   CNXT_SMC_UNIT_INST    *pUnitInst;
   SMC_RW_JOB_DESCRIPTOR *pFreeRwJobHead;    /* list of rw job resources */
   SMC_RW_JOB_DESCRIPTOR *pFreeRwJobTail;
} CNXT_SMC_DRIVER_INST;

/**********************/
/* Utility prototypes */
/**********************/
SMC_DESCRIPTOR *cnxt_smc_get_card_descriptor ( u_int32 uSlotId );
bool cnxt_smc_check_drv_init ( void );
void cnxt_smc_notify_unit_clients(CNXT_SMC_EVENT Event, u_int32 uUnit);
void cnxt_smc_free_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob );

void smc_access_enable ( void );
void smc_hw_init ( SMC_DESCRIPTOR *pCard, u_int32 uSlotId );
void smc_descriptor_init ( SMC_DESCRIPTOR *pCard, u_int32 uSlotId );
bool smc_atr_complete ( SMC_DESCRIPTOR *pCard );
bool smc_parse_atr ( u_int32 slot_id, SMC_DESCRIPTOR *pCard );
void smc_set_fi ( u_int32 uBank, u_int32 uFi );
void smc_set_di ( u_int32 uBank, u_int32 uDi );
bool smc_start_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob );
void smc_term_rw_job ( SMC_RW_JOB_DESCRIPTOR *pRwJob, CNXT_SMC_STATUS Status );
#ifdef SCARD_FIX
void smc_tx_last_char ( SMC_DESCRIPTOR *pCard, u_int32 iBank );
#endif

u_int32 cnxt_smc_isr ( u_int32 int_id, bool fiq, PFNISR *fn_chain );
#endif   /* _SMC_PRIV_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  9    mpeg      1.8         9/17/03 3:58:46 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the smartcard module to use the new handle lib.
 *        
 *  8    mpeg      1.7         6/30/03 5:02:28 PM     Miles Bintz     SCR(s) 
 *        6807 :
 *        changes to support milano IRD (added smart card hw & sw config 
 *        parameters)
 *        
 *  7    mpeg      1.6         2/6/03 8:50:46 AM      Larry Wang      SCR(s) 
 *        5324 :
 *        undefine SCARD_FIX.
 *        
 *  6    mpeg      1.5         2/6/03 8:34:06 AM      Larry Wang      SCR(s) 
 *        5324 :
 *        Add "#ifdef SCARD_FIX" segments to fix "fast turnaround" problem.
 *        
 *  5    mpeg      1.4         2/4/03 2:36:46 PM      Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  4    mpeg      1.3         2/4/03 12:17:44 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  3    mpeg      1.2         1/31/03 3:39:46 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  2    mpeg      1.1         1/28/03 9:28:36 AM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  1    mpeg      1.0         1/27/03 12:42:58 PM    Larry Wang      
 * $
 * 
 *    Rev 1.8   17 Sep 2003 14:58:46   goldenx
 * SCR(s) 7482 :
 * update the smartcard module to use the new handle lib.
 * 
 *    Rev 1.7   30 Jun 2003 16:02:28   bintzmf
 * SCR(s) 6807 :
 * changes to support milano IRD (added smart card hw & sw config parameters)
 * 
 *    Rev 1.6   06 Feb 2003 08:50:46   wangl2
 * SCR(s) 5324 :
 * undefine SCARD_FIX.
 * 
 *    Rev 1.5   06 Feb 2003 08:34:06   wangl2
 * SCR(s) 5324 :
 * Add "#ifdef SCARD_FIX" segments to fix "fast turnaround" problem.
 * 
 *    Rev 1.4   04 Feb 2003 14:36:46   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.3   04 Feb 2003 12:17:44   wangl2
 * SCR(s) 5324 :
 * 
 * 
 *    Rev 1.2   31 Jan 2003 15:39:46   wangl2
 * SCR(s) 5324 :
 * 
 * 
 ****************************************************************************/

