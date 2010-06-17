#include "basetype.h"
#define NULL		0
#define TRUE   		1
#define FALSE  		0

#define UARTBUF_LENGTH  16384
#define BUFFER_LENGTH	8192
#define MAX_READ_LENGTH 16384
#define MAX_WRITE_LENGTH 2048
// Disable all interrupts
#define DISABLE_INTERRUPTS		EA = 0
// Enable all interrupts
#define ENABLE_INTERRUPTS		EA = 1


typedef struct  
{
		  bool   bTimer;
		  bool   bIN_ISR;
  	  bool   bCOM_ERR;
		  bool   bTimeout;
		  bool   SLAVE_IS_ATTACHED;
		  bool   SLAVE_REMOVED;
		  bool  	SLAVE_DETECT;	// Slave USB device found
		  bool  	SLAVE_ENUMERATED;	// slave USB device enumeration done
		  bool   SLAVE_ONLINE;
		  bool 	TIMEOUT_ERR;				// timeout error during data endpoint transfer
		  bool	  DATA_STOP;					// device unplugged during data transfer
  	  bool   bData1;
		  bool   bMassDevice;
} XXGFLAGS;

