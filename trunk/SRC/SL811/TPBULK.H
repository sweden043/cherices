///////////////////////////////////
//#include "common.h"
#include "RBCCMD.H"
#include "SL811.H"
#include "HAL.H"
#include "common.h"
///////////////////////////////////
#define CBW_SIGNATURE   0x55534243
#define CSW_SIGNATURE   0x55534253

typedef struct {
    int32   dCBW_Signature;
    int32   dCBW_Tag;
    int32   dCBW_DataXferLen;
    u_int8    bCBW_Flag;
    int8    bCBW_LUN;
    int8    bCBW_CDBLen;
    CDB_RBC cdbRBC;
} CBW, *PCBW;///CommandBlock

typedef struct {
    int32   dCSW_Signature;
    int32   dCSW_Tag;
    int32   dCSW_DataResidue;
    int8    bCSW_Status;
} CSW, *PCSW;///CommandStatus

typedef union {
    CBW     TPBulk_CommandBlock;
    CSW     TPBulk_CommandStatus;

}TPBLK_STRUC, * PTPBLK_STRUC;
///////////////////////////////////
  u_int8 EnumMassDev(void);
  u_int8 SPC_Inquiry();
  u_int8 SPC_READLONG(void);
  u_int8 SPC_RequestSense(void);
  u_int8 SPC_TestUnit(void);
  u_int8 SPC_LockMedia(void);
  u_int8 RBC_ReadCapacity(void);
  u_int8 RBC_Read(  u_int32 lba,  u_int16 len,  void *pBuffer);
  u_int8 RBC_Write(  u_int32 lba,  u_int16 len,  void *pBuffer);

