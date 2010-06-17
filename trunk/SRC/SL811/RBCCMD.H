#include "basetype.h"
/*
// RBC commands
*/
#define RBC_CMD_FORMAT					0x04
#define RBC_CMD_READ10					0x28
#define RBC_CMD_READCAPACITY				0x25
#define RBC_CMD_STARTSTOPUNIT				0x1B
#define RBC_CMD_SYNCCACHE				0x35
#define RBC_CMD_VERIFY10				0x2F
#define RBC_CMD_WRITE10					0x2A

/*
// SPC-2 commands
*/
#define SPC_CMD_INQUIRY					0x12
#define SPC_CMD_MODESELECT6				0x15
#define SPC_CMD_MODESENSE6				0x1A
#define SPC_CMD_PERSISTANTRESERVIN			0x5E
#define SPC_CMD_PERSISTANTRESERVOUT			0x5F
#define SPC_CMD_PRVENTALLOWMEDIUMREMOVAL		0x1E
#define SPC_CMD_RELEASE6				0x17
#define SPC_CMD_REQUESTSENSE				0x03
#define SPC_CMD_RESERVE6				0x16
#define SPC_CMD_TESTUNITREADY				0x00
#define SPC_CMD_WRITEBUFFER				0x3B
#define SPC_CMD_READLONG				0x23


/*
// ATAPI Command Descriptor Block
*/

typedef struct _READ_10{
		u_int8 OperationCode;
		u_int8 Reserved1;
		u_int8 LBA_3;
		u_int8 LBA_2;
    	u_int8 LBA_1;
		u_int8 LBA_0;
		u_int8 Reserved2;
		u_int8 XferLen_1;
		u_int8 XferLen_0;
		u_int8 Reserved3[3];
} READ_10, * PREAD_10;



typedef struct _WRITE_10 {
		u_int8 OperationCode;
		u_int8 Reserved1;
		u_int8 LBA_3;
		u_int8 LBA_2;
    u_int8 LBA_1;
		u_int8 LBA_0;
		u_int8 Reserved2;
		u_int8 XferLen_1;
		u_int8 XferLen_0;
		u_int8 Reserved3[3];
} WRITE_10, *PWRITE_10;

typedef struct _MODE_SENSE_10 {
		u_int8 OperationCode;
		u_int8 Reserved1;
		
		//u_int8 PageCode : 6;
		//u_int8 Pc : 2;
		u_int8 PageCode;
		
		u_int8 Reserved2[4];
		u_int8 ParameterListLengthMsb;
		u_int8 ParameterListLengthLsb;
		u_int8 Reserved3[3];
} MODE_SENSE_10, *PMODE_SENSE_10;

typedef struct _MODE_SELECT_10 {
		u_int8 OperationCode;
		
		//u_int8 Reserved1 : 4;
		//u_int8 PFBit : 1;
		//u_int8 Reserved2 : 3;
		u_int8 Reserved1;
		
		u_int8 Reserved3[5];
		u_int8 ParameterListLengthMsb;
		u_int8 ParameterListLengthLsb;
		u_int8 Reserved4[3];
} MODE_SELECT_10, *PMODE_SELECT_10;
/*
////////////////////////////////////////////////////////////////////////////////////
// Command Descriptor Block
//      _RBC : Reduced Block Command
//      _SPC : SPC-2 SCSI primary Command - 2
////////////////////////////////////////////////////////////////////////////////////
*/

/*
// Generic
*/
typedef struct _GENERIC_CDB {
	u_int8 OperationCode;
    u_int8 Reserved[15];
} GENERIC_CDB,*PGENERIC_CDB;

typedef struct _GENERIC_RBC {
	u_int8 OperationCode;
    u_int8 Reserved[8];
	u_int8 Control;
} GENERIC_RBC,*PGENERIC_RBC;

/*
// format unit
*/
typedef struct _FORMAT_RBC {
	u_int8 OperationCode;	/* 04H */
	u_int8 VendorSpecific;
	
	//u_u_int8 Increment : 1;
	//u_u_int8 PercentorTime : 1;
	//u_u_int8 Progress : 1;
  //u_u_int8 Immediate : 1;
	//u_u_int8 VendorSpecific1 : 4;
	u_int8 Increment;
	
	u_int8 Reserved2[2];
	u_int8 Control;
} FORMAT_RBC, *PFORMAT_RBC;


/*
// Read Command
*/
typedef struct _READ_RBC {
	u_int8 OperationCode;	/* 10H */
	u_int8 VendorSpecific;
    	union{
        	struct
        	{
	        u_int8 LBA_3;
	        u_int8 LBA_2;
	        u_int8 LBA_1;
	        u_int8 LBA_0;
        	} LBA_W8 ;

       		int32 LBA_W32;
    	      }LBA;
	u_int8 Reserved;
    	//u_u_int8 XferLength_1;
	//u_u_int8 XferLength_0;
	int16 XferLength;
    	u_int8 Control;
    	//u_u_int8 Reserved1[3];
} READ_RBC, *PREAD_RBC;


/*
// Read Capacity Data - returned in Big Endian format
*/
typedef struct _READ_CAPACITY_DATA {

    u_int8 LBA_3;
    u_int8 LBA_2;
    u_int8 LBA_1;
    u_int8 LBA_0;
    u_int8 BlockLen_3;
    u_int8 BlockLen_2;
    u_int8 BlockLen_1;
    u_int8 BlockLen_0;
} READ_CAPACITY_DATA, *PREAD_CAPACITY_DATA;
//////////////////////////////////////////////////
typedef struct _READ_LONG_CMD{
    u_int8 OperationCode;
    
    //u_u_int8 LogicalUnitNum	:3;
    //u_u_int8 RES_1		:5;
    u_int8 LogicalUnitNum;
    
    u_int8 RES_2;
    u_int8 RES_3;
    u_int8 RES_4;
    u_int8 RES_5;
    u_int8 RES_6;
    u_int16 AllocationLen;
    u_int8 RES_7;
    u_int8 RES_8;
    u_int8 RES_9;
}READ_LONG_CMD, *PREAD_LONG_CMD;
typedef struct _READ_LONG {

    u_int8 RES_1;
    u_int8 RES_2;
    u_int8 RES_3;
    u_int8 CAP_LIST_LEN;
    
    u_int8 LBA_3;
    u_int8 LBA_2;
    u_int8 LBA_1;
    u_int8 LBA_0;

    u_int8 Descripter;
    u_int8 BlockLen_2;
    u_int8 BlockLen_1;
    u_int8 BlockLen_0;
} READ_LONG, *PREAD_LONG;
/*
// Read Capacity command
*/
typedef struct _READ_CAPACITY_RBC {
	u_int8                OperationCode;	/* 10H */
   	union  {
					//FLEXI_int32		l0[2];
					u_int32		l0[2];
        	u_int32         l[2];
        	READ_CAPACITY_DATA  CapData;       /* Reserved area, here is used as temp*/
    		} tmpVar;

	u_int8                Control;
} READ_CAPACITY_RBC, *PREAD_CAPACITY_RBC;

typedef struct _READ_CAPACITY_RSP {
	u_int32 LastLBA;
	u_int32 BlockSize;

} READ_CAPACITY_RSP, *PREAD_CAPACITY_RSP;
/*
// START_STOP_UNIT
*/
typedef struct _START_STOP_RBC {
    u_int8 OperationCode;    /*1BH*/
    
    //u_u_int8 Immediate: 1;
    //u_u_int8 Reserved1 : 7;
    u_int8 Immediate;
    
    u_int8 Reserved2[2];
	union _START_STOP_FLAGS
    {
    	/*
        struct
        {
            u_u_int8 Start          : 1;
            u_u_int8 LoadEject      : 1;
            u_u_int8 Reserved3      : 2;
            u_u_int8 PowerConditions: 4;
        } bits0;

        struct
        {
            u_u_int8 MediumState    : 2;
            u_u_int8 Reserved3      : 2;
            u_u_int8 PowerConditions: 4;
        } bits1;
        */
        u_int8 bits1;
    } Flags;
    u_int8 Control;
} START_STOP_RBC, *PSTART_STOP_RBC;

/*
// Synchronize Cache
*/
typedef struct _SYNCHRONIZE_CACHE_RBC {

	u_int8 OperationCode;    /* 0x35 */
	u_int8 Reserved[8];
	u_int8 Control;
} SYNCHRONIZE_CACHE_RBC, *PSYNCHRONIZE_CACHE_RBC;

/*
// Write Command
*/
typedef struct _WRITE_RBC {
    u_int8 OperationCode;	/* 2AH      */
    //u_u_int8 Reserved0 : 3;
    //u_u_int8 FUA : 1;
    //u_u_int8 Reserved1 : 4;
    u_int8 VendorSpecific;
    union{
	 struct
         {
	        u_int8 LBA_3;
	        u_int8 LBA_2;
	        u_int8 LBA_1;
	        u_int8 LBA_0;
          } LBA_W8 ;

	 int32 LBA_W32;
	}   LBA;
    u_int8 Reserved2;
    u_int16 XferLength;
    u_int8 Control;
} WRITE_RBC, *PWRITE_RBC;

/*
// VERIFY Command
*/
typedef struct _VERIFY_RBC {
    u_int8 OperationCode;	/* 2FH */
    u_int8 Reserved0;
	u_int8 LBA_3;			/* Big Endian */
	u_int8 LBA_2;
	u_int8 LBA_1;
	u_int8 LBA_0;
	u_int8 Reserved1;
    u_int8 VerifyLength_1;		/* Big Endian */
	u_int8 VerifyLength_0;
	u_int8 Control;
} VERIFY_RBC, *PVERIFY_RBC;

// SPC-2 of SCSI-3 commands
// INQUIRY Command
typedef struct _INQUIRY_SPC {
	u_int8 OperationCode;	/* 12H */
	
	//u_int8 EnableVPD:1 ;
  //u_int8 CmdSupportData:1 ;
	//u_int8 Reserved0:6 ;
	u_int8 EnableVPD;
	
	u_int8 PageCode;
	u_int8 Reserved1;
	u_int8 AllocationLen;
    	u_int8 Control;
} INQUIRY_SPC, *PINQUIRY_SPC;

/*
// Request Sense
*/
typedef struct _REQUEST_SENSE_SPC {
    u_int8 OperationCode;    /* 03H */
    u_int8 Reserved[3];
    u_int8 AllocationLen;
    u_int8 Control;
} REQUEST_SENSE_SPC, *PREQUEST_SENSE_SPC;

/*
// Test Unit Ready
*/
typedef struct _TEST_UNIT_SPC {
	u_int8 OperationCode;    /* 00H */
	u_int8 Reserved[4];
	u_int8 Control;
} TEST_UNIT_SPC, *PTEST_UNIT_SPC;
typedef struct _MEDIA_REMOVAL_SPC {
	u_int8 OperationCode;    /* 1EH */
	u_int8 Reserved0[3];
	u_int8 Prevent;
	//u_int8 Reserved1:6 ;
	//u_int8 Control;
} MEDIA_REMOVAL_SPC, *PMEDIA_REMOVAL_SPC;


typedef union  {
    GENERIC_CDB             Cdb_Generic;
  
     // RBC commands
    GENERIC_RBC             RbcCdb_Generic;

    FORMAT_RBC              RbcCdb_Format;
    READ_RBC                RbcCdb_Read;
    READ_CAPACITY_RBC       RbcCdb_ReadCapacity;
    START_STOP_RBC          RbcCdb_OnOffUnit;
    SYNCHRONIZE_CACHE_RBC   RbcCdb_SyncCache;
    VERIFY_RBC              RbcCdb_Verify;
    WRITE_RBC               RbcCdb_Write;

// SPC-2 commands
   
    INQUIRY_SPC             SpcCdb_Inquiry;
    REQUEST_SENSE_SPC       SpcCdb_RequestSense;
		TEST_UNIT_SPC           SpcCdb_TestUnit;
    MEDIA_REMOVAL_SPC       SpcCdb_Remove;
    
    
    // ATAPI Commands
    READ_10         CmdRead10;
    WRITE_10        CmdWrite10;
    MODE_SELECT_10  CmdModeSel10;
    MODE_SENSE_10   CmdModeSen10;
    //////////////////////////////////////
    READ_LONG_CMD	SpcCdb_ReadLong;

} CDB_RBC, *PCDB_RBC;
