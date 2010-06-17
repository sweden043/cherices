#include "HAL.H"
#define WORD   short
#define ISO_BIT     0x10
//#define MAX_DEV    	0x06     // maximum number of devices (Address #0 + Slave + 4 Hub's device)
#define MAX_EP    	0x05     // maximum number of endpoints (Ep0 + max of 4 data endpoints)

//*****************************************************************************************
// Constant Defines
//*****************************************************************************************

#define TIMEOUT_RETRY		0x06	// Maximum no. of timeout retry during USB xfer

typedef struct
{
    WORD  wVID, wPID;       // Vendor ID and Product ID
    u_int8  bClass;           // 
    u_int8  bNumOfEPs;        // actual number endpoint from slave
    u_int8  iMfg;				// Manufacturer ID
    u_int8  iPdt;				// Product ID
    u_int8  bId1;
    u_int8  bId2;
    u_int8  bEPAddr[MAX_EP];   // bit 7 = 1 = use PID_IN, 
    u_int8  bAttr[MAX_EP];     // ISO | Bulk | Interrupt | Control
    WORD  wPayLoad[MAX_EP];  // ISO range: 1-1023, Bulk: 1-64, etc
    WORD  bInterval[MAX_EP]; // polling interval (for LS)
    u_int8  bData1[MAX_EP];    // DataToggle value
} pUSBDEV, *PUSBDEV;

// USB specific request
typedef struct
{
    u_int8 bmRequest;
    u_int8 bRequest;
    WORD wValue;
    WORD wIndex;
    WORD wLength;
} SetupPKG, *pSetupPKG;

typedef struct
{
      u_int8 usbaddr;
      u_int8 endpoint;
      u_int8 pid;
      u_int8 wPayload;
      int wLen;
      u_int8 *buffer;	
    SetupPKG setup;
      u_int8 epbulkin;
      u_int8 epbulkout;
} XXGPKG, *pXXGPKG;
// Standard Device Descriptor
typedef struct
{   u_int8 bLength;
    u_int8 bDescriptorType;
    WORD bcdUSB;
    u_int8 bDeviceClass;
    u_int8 bDeviceSubClass;
    u_int8 bDeviceProtocol;
    u_int8 bMaxPacketSize0;
    WORD idVendor;
    WORD idProduct;
    WORD bcdDevice;
    u_int8 iManufacturer;
    u_int8 iProduct;
    u_int8 iSerialNumber;
    u_int8 bNumConfigurations;
} sDevDesc, *pDevDesc;

// Standard Configuration Descriptor
typedef struct
{	
  u_int8 bLength;                 // Size of descriptor in u_int8
	u_int8 bType;					 // Configuration
	WORD wLength;                // Total length
	u_int8 bNumIntf;				 // Number of interface
	u_int8 bCV;             		 // bConfigurationValue
	u_int8 bIndex;          		 // iConfiguration
	u_int8 bAttr;                  // Configuration u_int8acteristic
	u_int8 bMaxPower;				 // Power config
} sCfgDesc, *pCfgDesc;

// Standard Interface Descriptor
typedef struct
{	u_int8 bLength;
	u_int8 bType;
	u_int8 iNum;
	u_int8 iAltString;
	u_int8 bEndPoints;
	u_int8 iClass;
	u_int8 iSub; 
	u_int8 iProto;
	u_int8 iIndex; 
} sIntfDesc, *pIntfDesc;

// Standard EndPoint Descriptor
typedef struct
{	u_int8 bLength;
	u_int8 bType;
	u_int8 bEPAdd;
	u_int8 bAttr;
	WORD wPayLoad;               // low-speed this must be 0x08
	u_int8 bInterval;
} sEPDesc, *pEPDesc;

// Standard String Descriptor
typedef struct
{	u_int8 bLength;
	u_int8 bType;
	WORD wLang;
} sStrDesc, *pStrDesc;
//------------------------------------------------------------------------
// EP0 use for configuration and Vendor Specific command interface
//------------------------------------------------------------------------
#define EP0_Buf		    0x10	// define start of EP0 64-u_int8 buffer
#define EP1_Buf		    0x40	// define start of EP1 64-u_int8 buffer

/*-------------------------------------------------------------------------
 * SL811H Register Control memory map
 * --Note: 
 *      --SL11H only has one control register set from 0x00-0x04
 *      --SL811H has two control register set from 0x00-0x04 and 0x08-0x0c
 *------------------------------------------------------------------------*/

#define EP0Control      0x00
#define EP0Address      0x01
#define EP0XferLen      0x02
#define EP0Status       0x03
#define EP0Counter      0x04

#define EP1Control      0x08
#define EP1Address      0x09
#define EP1XferLen      0x0a
#define EP1Status       0x0b
#define EP1Counter      0x0c

#define CtrlReg         0x05
#define IntEna          0x06
                               // 0x07 is reserved
#define IntStatus       0x0d
#define cDATASet        0x0e
#define cSOFcnt         0x0f   // Master=1 Slave=0, D+/D-Pol Swap=1 0=not [0-5] SOF Count 
                               // 0xAE = 1100 1110
                               // 0xEE = 1110 1110

#define IntMask         0x57   /* Reset|DMA|EP0|EP2|EP1 for IntEna */
#define HostMask        0x47   /* Host request command  for IntStatus */
#define ReadMask        0xd7   /* Read mask interrupt   for IntStatus */

//Interrupt Status Mask
#define USB_A_DONE		0x01
#define USB_B_DONE		0x02
#define BABBLE_DETECT		0x04
#define INT_RESERVE		0x08
#define SOF_TIMER		0x10
#define INSERT_REMOVE		0x20
#define USB_DETECT		0x40
#define USB_DPLUS		0x80
#define INT_CLEAR		0xFF

//EP0 Status Mask
#define EP0_ACK			0x01	// EPxStatus bits mask during a read
#define EP0_ERROR		0x02
#define EP0_TIMEOUT		0x04
#define EP0_SEQUENCE	0x08
#define EP0_SETUP		0x10
#define EP0_OVERFLOW	0x20
#define EP0_NAK			0x40
#define EP0_STALL		0x80

/*-------------------------------------------------------------------------
 * Standard Chapter 9 definition
 *-------------------------------------------------------------------------
 */
#define GET_STATUS      0x00																  
#define CLEAR_FEATURE   0x01
#define SET_FEATURE     0x03
#define SET_ADDRESS     0x05
#define GET_DESCRIPTOR  0x06
#define SET_DESCRIPTOR  0x07
#define GET_CONFIG      0x08
#define SET_CONFIG      0x09
#define GET_INTERFACE   0x0a
#define SET_INTERFACE   0x0b
#define SYNCH_FRAME     0x0c


#define DEVICE          0x01
#define CONFIGURATION   0x02
#define STRING          0x03
#define INTERFACE       0x04
#define ENDPOINT        0x05

#define STDCLASS        0x00
//#define HIDCLASS        0x03
//#define HUBCLASS	 	0x09      // bDeviceClass, bInterfaceClass

/*-------------------------------------------------------------------------
 * SL11H/SL811H definition
 *-------------------------------------------------------------------------
 */

                           // USB-A, USB-B Host Control Register [00H, 08H]
                           // Pre  Reserved
                           //  DatT Dir [1=Trans, 0=Recv]
                           //   SOF Enable
                           //    ISO  Arm 
#define DATA0_WR    0x07   // 0000 0111 (      Data0 +       OUT + Enable + Arm)
#define sDATA0_WR   0x27   // 0010 0111 (      Data0 + SOF + OUT + Enable + Arm)
//#define pDATA0_WR   0x87   // 1000 0111 (Pre + Data0 +       OUT + Enable + Arm)
//#define psDATA0_WR  0xA7   // 1010 0111 (Pre + Data0 + SOF + OUT + Enable + Arm)

#define DATA0_RD    0x03   // 0000 0011 (      Data0 +       IN +  Enable + Arm)
#define sDATA0_RD   0x23   // 0010 0011 (      Data0 + SOF + IN +  Enable + Arm)
//#define pDATA0_RD   0x83   // 1000 0011 (Pre + Data0 +       IN +  Enable + Arm)
//#define psDATA0_RD  0xA3   // 1010 0011 (Pre + Data0 + SOF + IN +  Enable + Arm)

#define PID_SETUP   0xD0 
#define PID_IN      0x90
#define PID_OUT     0x10


  u_int8 SL811_GetRev(void);
void USBReset(void);
  u_int8 usbXfer(void);
  u_int8 ep0Xfer(void);
  u_int8 epBulkSend(  u_int8 *pBuffer,  int len);
  u_int8 epBulkRcv(  u_int8 *pBuffer,  int len);
  u_int8 SetAddress(  u_int8 addr);
  u_int8 Set_Configuration(void);
  u_int8 GetDesc(void);
  u_int8 DataRW(u_int8 epaddr);
  u_int8 EnumUsbDev(u_int8 usbaddr);
void speed_detect(void);
void slave_detect(void);
  u_int8 Slave_Detach(void);
void Set_ezDEV(u_int8 chg);

void SL811_Init(void);

void check_key_LED(void);

