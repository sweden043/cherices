#include "common.h"
#include "SL811.H"


extern XXGFLAGS  bXXGFlags;

XXGPKG usbstack;
 u_int8 remainder;

 u_int8 DBUF[BUFFER_LENGTH];

/*
#define	SMSC_ISA_DESC					0x4							//ISA_DESC_Nmb
#define	SMSC_ISA_DESC_PRM_REG		(PCI_ISA_DESC_BASE+SMSC_ISA_DESC*4)     //30010000X + c
#define	SMSC_ISA_DESC_EXT_REG		(PCI_ISA_DESC_BASE+0x80+SMSC_ISA_DESC*4)//30010000X + 8c
#define	SMSC_ISA_IO_BASE			(PCI_IO_BASE+0x100000*SMSC_ISA_DESC)     31300000X
*/

pUSBDEV  	uDev;	// Multiple USB devices attributes, Max 5 devices

//*****************************************************************************************
// SL811H variables initialization
//*****************************************************************************************
 u_int8 SL811_GetRev(void)
{
	return SL811Read(0x0e);
}

void USBReset(void)
{
	u_int8 temp;
  temp=SL811Read(CtrlReg);   
 	SL811Write(CtrlReg,temp|0x08);
	//task_time_sleep(25);		
	SL811Write(CtrlReg,temp);    
}

//*****************************************************************************************
// usbXfer:
// successful transfer = return TRUE
// fail transfer = return FALSE
//*****************************************************************************************
  u_int8 usbXfer(void)
{  
	
	  u_int8	xferLen, data0, data1,cmd;
	  u_int8 intr,result,remainder,dataX,bufLen,addr,timeout;
	
	//------------------------------------------------
	// Default setting for usb trasnfer
	//------------------------------------------------
	dataX=timeout=0;
	//result 	  = SL811Read(EP0Status);	
	data0 = EP0_Buf;					// DATA0 buffer address
	data1 = data0 + (  u_int8)usbstack.wPayload;	// DATA1 buffer address
	bXXGFlags.DATA_STOP=FALSE;
	bXXGFlags.TIMEOUT_ERR=FALSE;
	//------------------------------------------------
	// Define data transfer payload
	//------------------------------------------------
	if (usbstack.wLen >= usbstack.wPayload)  		// select proper data payload
		xferLen = usbstack.wPayload;			// limit to wPayload size 
	else							// else take < payload len
		xferLen = usbstack.wLen;			//	
	
	// For IN token
	if (usbstack.pid==PID_IN)				// for current IN tokens
	{												//
		cmd = sDATA0_RD;			// FS/FS on Hub, sync to sof
	}
	// For OUT token
	else if(usbstack.pid==PID_OUT)				// for OUT tokens
	{  	
		if(xferLen)									// only when there are	
			{
			//intr=usbstack.setup.wLength;
			//usbstack.setup.wLength=WordSwap(usbstack.setup.wLength);
			SL811BufWrite(data0,usbstack.buffer,xferLen); 	// data to transfer on USB
			//usbstack.setup.wLength=intr;
			}
		cmd = sDATA0_WR;						// FS/FS on Hub, sync to sof
	// implement data toggle
		bXXGFlags.bData1 = uDev.bData1[usbstack.endpoint];
        	uDev.bData1[usbstack.endpoint] = (uDev.bData1[usbstack.endpoint] ? 0 : 1); // DataToggle
		
		if(bXXGFlags.bData1)
          		cmd |= 0x40;                              // Set Data1 bit in command
	}
	// For SETUP/OUT token
	else											// for current SETUP/OUT tokens
	{  	
		if(xferLen)									// only when there are	
			{
			intr=usbstack.setup.wLength;
			usbstack.setup.wValue=usbstack.setup.wValue;
			usbstack.setup.wIndex=usbstack.setup.wIndex;
			usbstack.setup.wLength=usbstack.setup.wLength;
			SL811BufWrite(data0,(  u_int8 *)&usbstack.setup,xferLen); 	// data to transfer on USB
			usbstack.setup.wLength=intr;
			}
		cmd = sDATA0_WR;						// FS/FS on Hub, sync to sof
	}
	//------------------------------------------------
	// For EP0's IN/OUT token data, start with DATA1
	// Control Endpoint0's status stage.
	// For data endpoint, IN/OUT data, start ????
	//------------------------------------------------
	if (usbstack.endpoint == 0 && usbstack.pid != PID_SETUP) 	// for Ep0's IN/OUT token
		cmd |= 0x40; 					// always set DATA1
	//------------------------------------------------
	// Arming of USB data transfer for the first pkt
	//------------------------------------------------
	SL811Write(EP0Status,((usbstack.endpoint&0x0F)|usbstack.pid));	// PID + EP address
	SL811Write(EP0Counter,usbstack.usbaddr);					// USB address
	SL811Write(EP0Address,data0);					// buffer address, start with "data0"
	SL811Write(EP0XferLen,xferLen);					// data transfer length
	SL811Write(IntStatus,INT_CLEAR); 				// clear interrupt status
	SL811Write(EP0Control,cmd);						// Enable ARM and USB transfer start here
	
	//------------------------------------------------
	// Main loop for completing a wLen data trasnfer
	//------------------------------------------------
	while(TRUE)
	{   
		//---------------Wait for done interrupt------------------
		while(TRUE)												// always ensure requested device is
		{														// inserted at all time, then you will
			//intr=SL811Read(cSOFcnt);
			//intr=SL811Read(IntEna);
			intr = SL811Read(IntStatus);	
								// wait for interrupt to be done, and 
			if((intr & USB_DETECT) || (intr & INSERT_REMOVE))	// proceed to parse result from slave 
			{													// device.
				bXXGFlags.DATA_STOP = TRUE;								// if device is removed, set DATA_STOP
				return FALSE;									// flag true, so that main loop will 
			}													// know this condition and exit gracefully
			if(intr & USB_A_DONE)								
				break;											// interrupt done !!!
		}

		SL811Write(IntStatus,INT_CLEAR); 						// clear interrupt status
		result 	  = SL811Read(EP0Status);						// read EP0status register
		remainder = SL811Read(EP0Counter);						// remainder value in last pkt xfer

		//-------------------------ACK----------------------------
		if (result & EP0_ACK)									// Transmission ACK
		{	

			// SETUP TOKEN
			if(usbstack.pid == PID_SETUP)								// do nothing for SETUP/OUT token 
				break;											// exit while(1) immediately

			// OUT TOKEN				
			else if(usbstack.pid == PID_OUT)
				break;

			// IN TOKEN
			else if(usbstack.pid == PID_IN)
			{													// for IN token only
				usbstack.wLen  -= (WORD)xferLen;	// update remainding wLen value
				cmd   ^= 0x40;    			// toggle DATA0/DATA1
				dataX++;				// point to next dataX

				//------------------------------------------------	
				// If host requested for more data than the slave 
				// have, and if the slave's data len is a multiple
				// of its endpoint payload size/last xferLen. Do 
				// not overwrite data in previous buffer.
				//------------------------------------------------	
				if(remainder==xferLen)			// empty data detected
					bufLen = 0;			// do not overwriten previous data
				else					// reset bufLen to zero
					bufLen = xferLen;		// update previous buffer length
				
				//------------------------------------------------	
				// Arm for next data transfer when requested data 
				// length have not reach zero, i.e. wLen!=0, and
				// last xferlen of data was completed, i.e.
				// remainder is equal to zero, not a short pkt
				//------------------------------------------------	
				if(!remainder && usbstack.wLen)							// remainder==0 when last xferLen
				{												// was all completed or wLen!=0
					addr    = (dataX & 1) ? data1:data0; 		// select next address for data
					xferLen = (u_int8)(usbstack.wLen>=usbstack.wPayload) ? usbstack.wPayload:usbstack.wLen;	// get data length required
					//if (FULL_SPEED)								// sync with SOF transfer
					cmd |= 0x20;							// always sync SOF when FS, regardless 
					SL811Write(EP0XferLen, xferLen); 			// select next xfer length
					SL811Write(EP0Address, addr);           	// data buffer addr 
					SL811Write(IntStatus,INT_CLEAR);			// is a LS is on Hub.
					SL811Write(EP0Control,cmd);					// Enable USB transfer and re-arm
				}				

				//------------------------------------------------
				// Copy last IN token data pkt from prev transfer
				// Check if there was data available during the
				// last data transfer
				//------------------------------------------------
				if(bufLen)										
				{	
					SL811BufRead(((dataX&1)?data0:data1), usbstack.buffer, bufLen);
					usbstack.buffer += bufLen;								
				}

				//------------------------------------------------
				// Terminate on short packets, i.e. remainder!=0
				// a short packet or empty data packet OR when 
				// requested data len have completed, i.e.wLen=0
				// For a LOWSPEED device, the 1st device descp,
				// wPayload is default to 64-u_int8, LS device will
				// only send back a max of 8-u_int8 device descp,
				// and host detect this as a short packet, and 
				// terminate with OUT status stage
				//------------------------------------------------
				if(remainder || !usbstack.wLen)
					break;
			}// PID IN							
		}
			
		//-------------------------NAK----------------------------
		if (result & EP0_NAK)									// NAK Detected
		{														
			if(usbstack.endpoint==0)										// on ep0 during enumeration of LS device
			{													// happen when slave is not fast enough,
				SL811Write(IntStatus,INT_CLEAR);				// clear interrupt status, need to
				SL811Write(EP0Control,cmd);						// re-arm and request for last cmd, IN token
                		result = 0;                                     // respond to NAK status only
			}
			else												// normal data endpoint, exit now !!! , non-zero ep
				break;											// main loop control the interval polling
		}
	
		//-----------------------TIMEOUT--------------------------
		if (result & EP0_TIMEOUT)								// TIMEOUT Detected
		{														
			if(usbstack.endpoint==0)										// happens when hub enumeration
			{
				if(++timeout >= TIMEOUT_RETRY)
				{	
				    timeout--;
					break;										// exit on the timeout detected	
				}
				SL811Write(IntStatus,INT_CLEAR);				// clear interrupt status, need to
				SL811Write(EP0Control,cmd);						// re-arm and request for last cmd again
			}
			else												
			{													// all other data endpoint, data transfer 
				bXXGFlags.TIMEOUT_ERR = TRUE;								// failed, set flag to terminate transfer
				break;											// happens when data transfer on a device
			}													// through the hub
		}

		//-----------------------STALL----------------------------
		if (result & EP0_STALL)  								// STALL detected
			return TRUE;										// for unsupported request.
																		
		//----------------------OVEFLOW---------------------------
		if (result & EP0_OVERFLOW)  							// OVERFLOW detected
			//result=result;
			break;
		//-----------------------ERROR----------------------------
		if (result & EP0_ERROR)  								// ERROR detected
			//result=result;
			break;
	}	// end of While(1)
   
	if (result & EP0_ACK) 	// on ACK transmission
		return TRUE;		// return OK

	return FALSE;			// fail transmission

}
//*****************************************************************************************
// Control Endpoint 0's USB Data Xfer
// ep0Xfer, endpoint 0 data transfer
//*****************************************************************************************
  u_int8 ep0Xfer(void)
{
	usbstack.endpoint=0;
	usbstack.pid=PID_SETUP;
	usbstack.wLen=8;
	if (!usbXfer()) 
   		return FALSE;
	usbstack.pid  = PID_IN;
	usbstack.wLen=usbstack.setup.wLength;
   	if (usbstack.wLen)											// if there are data for transfer
	{
		if (usbstack.setup.bmRequest & 0x80)		// host-to-device : IN token
		{
			usbstack.pid  = PID_IN;	
			
			if(!usbXfer())
				return FALSE;
			usbstack.pid  = PID_OUT;
		}
		else											// device-to-host : OUT token
   		{							
			usbstack.pid  = PID_OUT;
				
			if(!usbXfer())
				return FALSE;
			usbstack.pid  = PID_IN;
		}
	}
	usbstack.wLen=0;
	if(!usbXfer())
		return FALSE;

	return TRUE;											
					
}


  u_int8 epBulkSend(  u_int8 *pBuffer,  int len)
{
	usbstack.usbaddr=0x1;
	usbstack.endpoint=usbstack.epbulkout;
	usbstack.pid=PID_OUT;
	usbstack.wPayload=64;
	usbstack.wLen=len;
	usbstack.buffer=pBuffer;
	while(len>0)
	{
		if (len > usbstack.wPayload)
			usbstack.wLen = usbstack.wPayload;
		else				
			usbstack.wLen = len;	
		if(!usbXfer())
			return FALSE;
		len-=usbstack.wLen;
		usbstack.buffer=usbstack.buffer+usbstack.wLen;
	}
	return TRUE;	
}

  u_int8 epBulkRcv(  u_int8 *pBuffer,  int len)
{
	usbstack.usbaddr=0x1;
	usbstack.endpoint=usbstack.epbulkin;
	usbstack.pid=PID_IN;
	usbstack.wPayload=64;
	usbstack.wLen=len;
	usbstack.buffer=pBuffer;
	if(usbstack.wLen)
	{
		if(!usbXfer())
			return FALSE;
	}
	return TRUE;
}


//*****************************************************************************************
// Set Device Address : 
//*****************************************************************************************
  u_int8 SetAddress(  u_int8 addr)
{
	usbstack.usbaddr=0;
	usbstack.setup.bmRequest=0;
	usbstack.setup.bRequest=SET_ADDRESS;
	usbstack.setup.wValue=addr;
	usbstack.setup.wIndex=0;
	usbstack.setup.wLength=0;
	return ep0Xfer();

}

//*****************************************************************************************
// Set Device Configuration : 
//*****************************************************************************************
  u_int8 Set_Configuration(void)
{
	usbstack.setup.bmRequest=0;
	usbstack.setup.bRequest=SET_CONFIG;
	usbstack.setup.wIndex=0;
	usbstack.setup.wLength=0;
	usbstack.buffer=NULL;
	return ep0Xfer();

}

//*****************************************************************************************
// Get Device Descriptor : Device, Configuration, String
//*****************************************************************************************
  u_int8 GetDesc(void)
{ 
	
	usbstack.setup.bmRequest=0x80;
	usbstack.setup.bRequest=GET_DESCRIPTOR;
	usbstack.setup.wValue=(usbstack.setup.wValue);
	
	usbstack.wPayload=uDev.wPayLoad[0];
	//usbstack.buffer=&usbstack.setup;
	return ep0Xfer();
}

//*****************************************************************************************
// USB Device Enumeration Process
// Support 1 confguration and interface #0 and alternate setting #0 only
// Support up to 1 control endpoint + 4 data endpoint only
//*****************************************************************************************
  u_int8 EnumUsbDev(u_int8 usbaddr)
{  
	  u_int8 i;											// always reset USB transfer address 
	  u_int8 uAddr = 0;							// for enumeration to Address #0
	  u_int8 epLen;
	//  short strLang;
	
	pDevDesc  pDev;	
	pCfgDesc pCfg;
	pIntfDesc pIfc;
	pEPDesc pEnp;
	//------------------------------------------------
	// Reset only Slave device attached directly
	//------------------------------------------------
	uDev.wPayLoad[0] = 64;	// default 64-u_int8 payload of Endpoint 0, address #0
	if(usbaddr == 1)		// bus reset for the device attached to SL811HS only
		USBReset();		// that will always have the USB address = 0x01 (for a hub)
    	
//    task_time_sleep(25);
	
	pDev =(pDevDesc)DBUF;					// ask for 64 u_int8s on Addr #0
	
	usbstack.usbaddr=uAddr;
	usbstack.setup.wValue=DEVICE;
	usbstack.setup.wIndex=0;
	usbstack.setup.wLength=18;
	//usbstack.setup.wLength=sbstack.setup.wLength);
	usbstack.buffer=DBUF;
	
	if (!GetDesc())			// and determine the wPayload size
		return FALSE;								// get correct wPayload of Endpoint 0
	uDev.wPayLoad[0]=pDev->bMaxPacketSize0;// on current non-zero USB address

	//------------------------------------------------
	// Set Slave USB Device Address
	//------------------------------------------------
	if (!SetAddress(usbaddr)) 						// set to specific USB address
		return FALSE;								//
	uAddr = usbaddr;								// transfer using this new address

	//------------------------------------------------
	// Get USB Device Descriptors on EP0 & Addr X
	//------------------------------------------------
	pDev =(pDevDesc)DBUF;
	usbstack.usbaddr=uAddr;
	
	usbstack.setup.wLength=pDev->bLength;
	usbstack.setup.wValue=DEVICE;
	usbstack.setup.wIndex=0;
	
	//usbstack.setup.wLength=0x12;//(  short)DBUF[0];//pDev->bLength;
	usbstack.buffer=DBUF;
	
	if (!GetDesc()) 	
		return FALSE;								// For this current device:
	uDev.wVID  = pDev->idVendor;			// save VID
	uDev.wPID  = pDev->idProduct;			// save PID
	uDev.iMfg  = pDev->iManufacturer;		// save Mfg Index
	uDev.iPdt  = pDev->iProduct;			// save Product Index

	//------------------------------------------------
	// Get Slave USB Configuration Descriptors
	//------------------------------------------------
	
	pCfg = (pCfgDesc)DBUF;	
	
	usbstack.usbaddr=uAddr;
	usbstack.setup.wValue=CONFIGURATION;
	usbstack.setup.wIndex=0;
	usbstack.setup.wLength=64;
	usbstack.buffer=DBUF;	
	if (!GetDesc()) 		
		return FALSE;	
	
	pIfc = (pIntfDesc)(DBUF + 9);					// point to Interface Descp
	uDev.bClass 	= pIfc->iClass;			// update to class type
	uDev.bNumOfEPs = (pIfc->bEndPoints <= MAX_EP) ? pIfc->bEndPoints : MAX_EP;
	
	if(uDev.bClass==8) //mass storage device
		bXXGFlags.bMassDevice=TRUE;
	//------------------------------------------------
	// Set configuration (except for HUB device)
	//------------------------------------------------
	usbstack.usbaddr=uAddr;
	usbstack.setup.wValue=DEVICE;
	//if (uDev[usbaddr].bClass!=HUBCLASS)				// enumerating a FS/LS non-hub device
		if (!Set_Configuration())		// connected directly to SL811HS
				return FALSE;

	//------------------------------------------------
	// For each slave endpoints, get its attributes
	// Excluding endpoint0, only data endpoints
	//------------------------------------------------
	
	epLen = 0;
	for (i=1; i<=uDev.bNumOfEPs; i++)				// For each data endpoint
	{
		pEnp = (pEPDesc)(DBUF + 9 + 9 + epLen);	   			// point to Endpoint Descp(non-HID)
		uDev.bEPAddr[i]  	= pEnp->bEPAdd;			// Ep address and direction
		uDev.bAttr[i]		= pEnp->bAttr;			// Attribute of Endpoint
		uDev.wPayLoad[i] 	= (pEnp->wPayLoad);		// Payload of Endpoint
		uDev.bInterval[i] 	= pEnp->bInterval;		// Polling interval
	    	uDev.bData1[i] = 0;			            // init data toggle
		epLen += 7;
		//////////////////////////////
		if(uDev.bAttr[i]==0x2)
		{
		    if(uDev.bEPAddr[i]&0x80)
		    	usbstack.epbulkin=uDev.bEPAddr[i];
		    else
		    	usbstack.epbulkout=uDev.bEPAddr[i];
		}
		//////////////////////////////
	}
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////
void SL811_Init(void)
{	
	bXXGFlags.SLAVE_ONLINE = FALSE;
	bXXGFlags.SLAVE_DETECT = FALSE;
	bXXGFlags.SLAVE_REMOVED=FALSE;
	
	bXXGFlags.SLAVE_ENUMERATED = FALSE;
	bXXGFlags.SLAVE_IS_ATTACHED = FALSE;
	
	SL811Write(cDATASet,0xe0);    //for 1ms interval
	SL811Write(cSOFcnt,0xae);
	SL811Write(CtrlReg,0x5);
			
	SL811Write(EP0Status,0x50);
	SL811Write(EP0Counter,0);
	SL811Write(EP0Control,0x01);
			
	
	SL811Write(IntEna,0x20);      		// USB-A, Insert/Remove, USB_Resume.
	SL811Write(IntStatus,INT_CLEAR);	// Clear Interrupt enable status
}

void check_key_LED(void)
{
	  u_int8 intr;
	intr=SL811Read(IntStatus);
	if(intr & USB_DETECT)
		   {
		    if(bXXGFlags.SLAVE_ONLINE ==TRUE)
		   	{bXXGFlags.SLAVE_REMOVED=TRUE;
		   	 bXXGFlags.SLAVE_ONLINE =FALSE;
		   	}
		   }
	else	{
		   if(bXXGFlags.SLAVE_ONLINE == FALSE)
		   	{bXXGFlags.SLAVE_DETECT=TRUE;
		   	 bXXGFlags.SLAVE_ONLINE =TRUE;
		   	}
		   }
	SL811Write(IntStatus,INT_CLEAR);
	SL811Write(IntStatus,INSERT_REMOVE);
}

