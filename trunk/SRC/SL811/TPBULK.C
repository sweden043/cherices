

#include "TPBULK.H"


//#include "common.h"

//////////////////////////////////
extern  u_int8  DBUF[BUFFER_LENGTH];

TPBLK_STRUC  TPBulk_Block;

///////////////////////////////////////////////////////////////////////////
u_int8 EnumMassDev(void)
{
	u_int8 intr=SL811Read(IntStatus);
	if(!intr & USB_DETECT)
		return FALSE;
	if(!EnumUsbDev(1))		
		return FALSE;
	
	if(!SPC_Inquiry())
		return FALSE;
	if(!SPC_TestUnit())
		return FALSE;
	if(!SPC_LockMedia())
		return FALSE;
	if(!SPC_RequestSense())
		return FALSE;
	if(!SPC_TestUnit())
		return FALSE;
	if(!RBC_ReadCapacity())
		return FALSE;

	return TRUE;
}


u_int8 SPC_Inquiry()
{
#define cdbInquirySPC TPBulk_Block.TPBulk_CommandBlock.cdbRBC.SpcCdb_Inquiry
	//u_int8 len;
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0x24000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x80;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(INQUIRY_SPC);
	
	/////////////////////////////////
	cdbInquirySPC.OperationCode=SPC_CMD_INQUIRY;
	
	cdbInquirySPC.EnableVPD=0;
	//cdbInquirySPC.CmdSupportData=0;
	cdbInquirySPC.PageCode=0;
	cdbInquirySPC.AllocationLen=0x24;
	cdbInquirySPC.Control=0;
	////////////////////////////////
	//if(!epBulkRcv(DBUF,5))
	//	return FALSE;
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(150);
	//len=36;
	if(!epBulkRcv(DBUF,38))
		return FALSE;
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
	////////////////////////////////
	return TRUE;	
#undef cdbInquirySPC
}

u_int8 SPC_READLONG(void)
{
#define cdbReadLong TPBulk_Block.TPBulk_CommandBlock.cdbRBC.SpcCdb_ReadLong	
	//nsigned char retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0xfc000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x80;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(READ_LONG_CMD);
	
	/////////////////////////////////////
	cdbReadLong.OperationCode=SPC_CMD_READLONG;
	cdbReadLong.LogicalUnitNum=0;
	cdbReadLong.AllocationLen=0xfc;
	//////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(5);
	//len=36;
	if(!epBulkRcv(DBUF,0xfc))
		return FALSE;
	
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
  ////////////////////////////
  return TRUE;
#undef cdbReadLong
}
u_int8 SPC_RequestSense(void)
{
#define cdbRequestSenseSPC TPBulk_Block.TPBulk_CommandBlock.cdbRBC.SpcCdb_RequestSense	
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0x0e000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x80;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(REQUEST_SENSE_SPC);
		
	/////////////////////////////////////
	cdbRequestSenseSPC.OperationCode=SPC_CMD_REQUESTSENSE;
	cdbRequestSenseSPC.AllocationLen=0x0e;
	//////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(5);
	//len=36;
	if(!epBulkRcv(DBUF,18))
		return FALSE;
	
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
/////////////////////////////
	return TRUE;
	
#undef cdbRequestSenseSPC
}
u_int8 SPC_TestUnit(void)
{
#define cdbTestUnit TPBulk_Block.TPBulk_CommandBlock.cdbRBC.SpcCdb_TestUnit	
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0x00000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x00;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(TEST_UNIT_SPC);
	/////////////////////////////////////
	cdbTestUnit.OperationCode=SPC_CMD_TESTUNITREADY;
	//////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(5);
	
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
#undef cdbTestUnit
////////////////////////////
	return TRUE;
}
u_int8 SPC_LockMedia(void)
{
#define cdbLockSPC TPBulk_Block.TPBulk_CommandBlock.cdbRBC.SpcCdb_Remove	
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0x00000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x00;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(MEDIA_REMOVAL_SPC);
	///////////////////////////////////////////
	cdbLockSPC.OperationCode=SPC_CMD_PRVENTALLOWMEDIUMREMOVAL;
	cdbLockSPC.Prevent=1;
	///////////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(5);
	
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
#undef cdbLockSPC
/////////////////////////////
	return TRUE;
}
u_int8 RBC_ReadCapacity(void)
{
#define cdbReadCap TPBulk_Block.TPBulk_CommandBlock.cdbRBC.RbcCdb_ReadCapacity	
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=0x08000000;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x80;
	
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(READ_CAPACITY_RBC);
	/////////////////////////////////////
	cdbReadCap.OperationCode=RBC_CMD_READCAPACITY;
	/////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(10);
	//len=36;
	if(!epBulkRcv(DBUF,8))
		return FALSE;
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
#undef cdbReadCap
/////////////////////////////
	return TRUE;
}
u_int8 RBC_Read(u_int32 lba,u_int16 len,void *pBuffer)
{
#define cdbRead TPBulk_Block.TPBulk_CommandBlock.cdbRBC.RbcCdb_Read	
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0x60a624de;
	//TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=SwapINT32(len*DeviceInfo.BPB_BytesPerSec);
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=(len*512);
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x80;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(READ_RBC);
	/////////////////////////////////////
	cdbRead.OperationCode=RBC_CMD_READ10;
	cdbRead.VendorSpecific=0;
	cdbRead.LBA.LBA_W32=lba;
	cdbRead.XferLength=len;
	//cdbRead.Reserved1[0]=0;
	//cdbRead.Reserved1[1]=0;
	//cdbRead.Reserved1[2]=0x40;
	//////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(5);
	//len=36;
	//if(!epBulkRcv(pBuffer,len*DeviceInfo.BPB_BytesPerSec))
	if(!epBulkRcv(pBuffer,len*512))
		return FALSE;
	//DelayMs(1);
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
#undef cdbRead
/////////////////////////////
	return TRUE;
}

u_int8 RBC_Write(unsigned long lba,u_int16 len,void *pBuffer)
{
#define cdbWrite TPBulk_Block.TPBulk_CommandBlock.cdbRBC.RbcCdb_Write	
	//len=2;
	//unsigned int idata temp;
	//u_int8 i;
	//len=1;
	//SPC_TestUnit();
	//u_int8 retStatus=FALSE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Signature=CBW_SIGNATURE;
	TPBulk_Block.TPBulk_CommandBlock.dCBW_Tag=0xb4D977c1;
	//TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=SwapINT32(len*DeviceInfo.BPB_BytesPerSec);
	TPBulk_Block.TPBulk_CommandBlock.dCBW_DataXferLen=len*512;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_Flag=0x0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_LUN=0;
	TPBulk_Block.TPBulk_CommandBlock.bCBW_CDBLen=sizeof(WRITE_RBC);
	/////////////////////////////////////
	cdbWrite.OperationCode=RBC_CMD_WRITE10;
	cdbWrite.VendorSpecific=0;
	cdbWrite.LBA.LBA_W32=lba;
	cdbWrite.XferLength=len;
	cdbWrite.Reserved2=0;
	cdbWrite.Control=0;
	//////////////////////////////////////
	if(!epBulkSend((u_int8 *)&TPBulk_Block.TPBulk_CommandBlock,sizeof(TPBulk_Block.TPBulk_CommandBlock)))
		return FALSE;
	//DelayMs(15);
	
	//if(!epBulkSend(pBuffer,DeviceInfo.BPB_BytesPerSec))
	if(!epBulkSend(pBuffer,512))
		return FALSE;
	//DelayMs(10);
	if(!epBulkRcv((u_int8 *)&TPBulk_Block.TPBulk_CommandStatus,13))
		return FALSE;
	
#undef cdbWrite

/////////////////////////////
	return TRUE;
}
