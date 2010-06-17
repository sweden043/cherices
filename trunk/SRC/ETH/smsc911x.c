/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                 				*/
/*                       SOFTWARE FILE/MODULE HEADER                        					*/
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          				*/
/*                              Austin, TX                                  								*/
/*                         All Rights Reserved                              							*/
/****************************************************************************/
/*
 * Filename:        smsc911x.c
 *
 *
 * Description:     SMSC LAN911X Ethernet Driver  Source File, 
 *
 *
 * Author:          Sam Chen
 *
 ****************************************************************************/
/* $Id:smsc911x.c,v 1.0, 3/29/2005 16:00:00 PM  Sam Chen
 ****************************************************************************/
#include "string.h"

#include "..\nupnet\target.h"
#include "..\nupnet\dev.h"
#include "..\nupnet\net.h"
#include "..\nupnet\mem_defs.h"
#include "..\nupnet\externs.h"
#include "..\nupnet\netevent.h"

#include "retcodes.h"

#include "smsc911x.h"
#include "board.h"
#include "gpio.h"

#ifndef USE_DEBUG
#define	USE_DEBUG
#endif

#ifdef USE_DEBUG
#define USE_WARNING
#define USE_TRACE
#define USE_ASSERT
#endif //USE_DEBUG


#ifdef USE_ASSERT
#define SMSC_ASSERT(condition)		\
	do{									\
		if(!(condition)) {																\
			trace("SMSC_ASSERTION_FAILURE: File=" __FILE__ ", Line=%d\n",__LINE__);	\
			while(1);																	\
		}	\
	}while(0);	

#define	SMSC_TRACE trace
#define	SMSC_ISR_TRACE(msg,arg1,arg2)	isr_trace(msg,arg1,arg2)
#define	SMSC_WARNING  trace
	
#else
#define 	SMSC_ASSERT(condition)
#define	SMSC_TRACE(args...)	 
#define	SMSC_ISR_TRACE(msg,arg1,arg2)	
#define	SMSC_WARNING(args...) 
#endif

#define	SMSC_LOCK(lock,ms) \
    do {				\
             sem_get(lock,ms); \
    }while(0);	

#define	SMSC_UNLOCK(lock) \
    do { \
	sem_put(lock); \
    }while(0);	


#define	SMSC_ENTER_CRITICAL_REGION(old_status)  \
	do {	\
		old_status=critical_section_begin(); \
	}while(0);	

#define	SMSC_LEAVE_CRITICAL_REGION(old_status)  \
	do {	\
		critical_section_end(old_status);	\
	}while(0);	


/* Defines for Electra GPIO */
#if (GPIO_CONFIG == GPIOM_BRAZOS)
   #define SMSC_INT_GPIO_PORT	4	
   #define SMSC_HOST_INTERRUPT   (SMSC_INT_GPIO_PORT +GPIO_DEVICE_ID_INTERNAL+GPIO_PIN_IS_INPUT )
#endif   


#define	NEW_BOARD		/* New Board with A1 conected to IOA01 */	
#ifndef 	NEW_BOARD
#define	OLD_BOARD		/* Old Board with A1 conected to IOA00 */
#endif   

#ifdef		OLD_BOARD
#define	DATA_WIDTH	32
#else
#define	DATA_WIDTH	16
#endif

/* SMSC FIFO Threshold */

#define	SMSC_TDFA_LEVEL	(0xFFU<<24)
#define	SMSC_TSFL_LEVEL	(0xFFU<<16)
#define	SMSC_RDFL_LEVEL	(0x00<<8)

/* 
* when smsc receive more than SMSC_RSFL_LEVEL 
* frame, INT_RSFL will be triggered
*/
#define	SMSC_RSFL_LEVEL	0x0

/* this is the value that How many times that Rx Task will Poll the 
	RX status FIFO.the larger this value is , the lower load spent in
	Interrupt switching.but since the Rx Task's priority is very high,
	may delay the running of other task.
	so, this is a trade-off.
*/	
#define	SMSC_RX_POLLING_TIMES	50

/* SMSC Control MSG */
#define	RX_MSG_RSFL	0x1		/* RX FIFO LEVEL reach */
#define	RX_MSG_RXD	0x2		/* RX DMA Completed */
#define	RX_MSG_RDFF	0x3		/* RX FIFO FULL */

#define	TX_MSG_TDFA	0x4		/* TX fifo available */
#define	SMSC_RESET		0x5

/* SMSC FLOW Control Level */
#define	SMSC_AFC_HI	0x67	
#define	SMSC_AFC_LO	0x3E

#define	SMSC_STACK_SIZE	0x800
#define	SMSC_TASK_PRIO	200

#define	SMSC_ISA_DESC					0x4							//ISA_DESC_Nmb
#define	SMSC_ISA_DESC_PRM_REG		(PCI_ISA_DESC_BASE+SMSC_ISA_DESC*4)     //30010000X + c
#define	SMSC_ISA_DESC_EXT_REG		(PCI_ISA_DESC_BASE+0x80+SMSC_ISA_DESC*4)//30010000X + 8c
#define	SMSC_ISA_IO_BASE			(PCI_IO_BASE+0x100000*SMSC_ISA_DESC)     /*31300000X*/

#define	E2PROM_ADDRESS			    I2C_ADDR_EEPROM1

#define	TX_FIFO_LOW_THRESHOLD	(1600U)
/* 
  * according SMSC Lan9115 datasheet , the WR/RD assertion time must > 32ns 
  * and the whole access time must > 165ns 
  * Assume the Memory clock is 166Mhz ( 1 Cycle= 6 ns)
  * we can get that the WR/RD WAIT should > (32/6) = 6 Cycles and the whole
  * should > (165/6 ) = 28 
  */
 #ifdef OLD_BOARD
#define	SMSC_WR_WAIT			(0x8<<24)
#define	SMSC_RD_WAIT			(0x8<<16)
 #else
#define	SMSC_WR_WAIT			(0xa<<24)//a
#define	SMSC_RD_WAIT			(0xa<<16)//a
#endif
#define	SMSC_CS_SETUP			(0x4<<17)
#define	SMSC_CTL_SETUP		(0x4<<5)
#define	SMSC_CS_HOLD			(0x4<<12)
#define	SMSC_ADDR_HOLD		(0x1)

#define	SMSC_REG_SETUP		(0x0<<28)
#define	SMSC_REG_ACCESS		(0x10<<24)


#define	SMSC_CS_SEL			0x3


#define	SMSC_PIO				0
#define	SMSC_DMA				1
#define	SMSC_TRANSFER_MODE    SMSC_PIO


#if (SMSC_TRANSFER_MODE==SMSC_DMA)
#define	DMA_POLLING				0
#define	SMSC_RX_DMA_CHANNEL		0	
#define	SMSC_TX_DMA_CHANNEL		1	
#define	SMSC_DMA_THRESHOLD		128 /*in byte*/
#endif

#define	BURST_SIZE		4	/*4 byte*/


/***********************/
/* global variables             */
/***********************/

struct _DV_DEVICE_ENTRY *smsc_dev;
smsc_priv_t smsc_priv;

/***********************/
/*   function prototype        */
/***********************/

static  void smsc_clear_bits(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 bits);
static bool smsc_mac_notbusy(struct _DV_DEVICE_ENTRY *dev);
static void smsc_set_bits(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 bits);

/*MAC func*/
//static void smsc_mac_init(struct _DV_DEVICE_ENTRY *dev);
static void smsc_mac_set_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6]);
static void smsc_mac_get_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6]);

/*Phy Func*/
static void smsc_phy_set_link(struct _DV_DEVICE_ENTRY *dev,u_int32 option); 
static void smsc_phy_polling_timer(tick_id_t hTick, void *pUserData);
static ETH_STATUS smsc_phy_init(struct _DV_DEVICE_ENTRY *dev);
static void smsc_phy_update_link(struct _DV_DEVICE_ENTRY *dev);
static void smsc_phy_get_link_mode(struct _DV_DEVICE_ENTRY *dev);
static ETH_STATUS smsc_phy_reset(DV_DEVICE_ENTRY *dev);


static ETH_STATUS smsc_chip_reset(struct _DV_DEVICE_ENTRY *dev);
static ETH_STATUS smsc_chip_init(struct _DV_DEVICE_ENTRY *dev);
static ETH_STATUS smsc_ISR_init(struct _DV_DEVICE_ENTRY *dev);
#if (SMSC_TRANSFER_MODE==SMSC_DMA)
static ETH_STATUS smsc_DMA_init();
#endif

/*TX Func*/
static ETH_STATUS smsc_Tx_init(struct _DV_DEVICE_ENTRY *dev);
static int32 smsc_Tx_copy_frame(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf);
static u_int32 smsc_Tx_status_count(struct _DV_DEVICE_ENTRY *dev);
static void smsc_Tx_update_stats(struct _DV_DEVICE_ENTRY *dev);
//static ETH_STATUS smsc_Tx_FIFO_dump(struct _DV_DEVICE_ENTRY *dev);
static STATUS smsc_Tx_pause_xmit(struct _DV_DEVICE_ENTRY *dev);
static void smsc_Tx_resume_xmit(struct _DV_DEVICE_ENTRY *dev);

/*Rx Func*/
static ETH_STATUS smsc_Rx_init(struct _DV_DEVICE_ENTRY *dev);
//static ETH_STATUS smsc_Rx_process_status(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwFrameLen);
static ETH_STATUS smsc_Rx_fast_forward(struct _DV_DEVICE_ENTRY *dev,u_int32 nDwords);
static STATUS smsc_Rx_transfer_frame(struct _DV_DEVICE_ENTRY *dev);
static void smsc_task(struct _DV_DEVICE_ENTRY *dev) ;
//static ETH_STATUS smsc_Rx_FIFO_dump(struct _DV_DEVICE_ENTRY *dev);
//static void smsc_Rx_pause(struct _DV_DEVICE_ENTRY *dev);
//static void smsc_Rx_resume(struct _DV_DEVICE_ENTRY *dev);

/*ISR func*/
static int32 smsc_ISR(u_int32 irq, bool FIQ, PFNISR *shared);
static ETH_STATUS smsc_Tx_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn);
static ETH_STATUS smsc_Rx_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn);
static ETH_STATUS smsc_other_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn);
static ETH_STATUS smsc_phy_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn);
#if (SMSC_TRANSFER_MODE==SMSC_DMA)
static int32 smsc_DMA_ISR(u_int32 irq, bool FIQ, PFNISR *shared);
#endif
/*Other func*/
STATUS smsc_set_multicast(struct _DV_DEVICE_ENTRY *dev);
//static const ETH_STATS*  smsc_get_stats (struct _DV_DEVICE_ENTRY *dev);
static void smsc_nop(struct _DV_DEVICE_ENTRY *dev);
static void smsc_fast_recover(struct _DV_DEVICE_ENTRY *dev);

#ifdef USE_DEBUG
//static void smsc_print_reg(struct _DV_DEVICE_ENTRY *dev);
//static void smsc_dump(u_int8 * buf, u_int32 len);
//STATUS smsc_ioctl(struct _DV_DEVICE_ENTRY *dev,int32 option, DV_REQ *request);
#endif

#ifdef 	OLD_BOARD
#define	SMSC_READ		smsc_read
#else
#define	SMSC_READ(dev,reg)	\
				(*(volatile u_int32*)(dev->dev_io_addr+reg))
#endif

#ifdef 	OLD_BOARD
#define	SMSC_WRITE 	smsc_write
#else
#define	SMSC_WRITE(dev,reg,data) \
				(*((volatile u_int32 *)(dev->dev_io_addr+reg)) = data)
#endif


/***********************/
/* function implementation */
/***********************/

#ifdef OLD_BOARD
/********************************************************************
    FUNCTION:    smsc_read

    PARAMETERS:  
                 N/A.
                 
    DESCRIPTION: 
                 Read the SMSC System Control and Status Register.

    RETURNS:     
                 The 32-bit register value.
    
    NOTES:
    		   If the A0 of Brozos was connected to the A1 of SMSC911X, the WIDTH
    		   field, Bits[5:4] must set as 32-bit.
                
    CONTEXT:
    
*********************************************************************/
u_int32 smsc_read(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg)
{
	u_int32 hi=0;
	u_int32 lo=0;
	lo=*((volatile u_int32 *)(dev->dev_io_addr+(reg>>1)*2+0)) & 0xFFFF ;
	hi=*((volatile u_int32 *)(dev->dev_io_addr+(reg>>1)*2+1)) & 0xFFFF ;	
	return (hi<<16|lo);	
}

/********************************************************************
    FUNCTION:    smsc_write

    PARAMETERS:  
                 N/A.
                 
    DESCRIPTION: 
                 write a 32-bit data into  the SMSC System Control and Status Register.

    RETURNS:     
                 N/A.
    
    NOTES:
    		   If the A0 of Brozos was connected to the A1 of SMSC911X, the WIDTH
    		   field, Bits[5:4] must set as 32-bit.
                
    CONTEXT:
    
*********************************************************************/
void smsc_write(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 data)
{
	u_int32 hi=(data >>16 )& 0xFFFF;
	u_int32 lo=(data & 0xFFFF);
	*((volatile u_int32 *)(dev->dev_io_addr+(reg>>1)+0)) = lo;
	*((volatile u_int32 *)(dev->dev_io_addr+(reg>>1)+1)) = hi;	
}
#endif


/********************************************************************
    FUNCTION:    smsc_clear_bits

    PARAMETERS:  
                 N/A.
                 
    DESCRIPTION: 
                 clear some bits of  the SMSC System Control and Status Register.

    RETURNS:     
                 N/A.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static   void smsc_clear_bits(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 bits)
{
	SMSC_WRITE(dev,reg,SMSC_READ(dev,reg) & (~bits));
}

/********************************************************************
    FUNCTION:    smsc_set_bits

    PARAMETERS:  
                 N/A.
                 
    DESCRIPTION: 
                 set some bits of  the SMSC System Control and Status Register.

    RETURNS:     
                 N/A.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static  void smsc_set_bits(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 bits)
{
	SMSC_WRITE(dev,reg,SMSC_READ(dev,reg)|bits);
}

/********************************************************************
    FUNCTION:    smsc_nop

    PARAMETERS:  
                 N/A.
                 
    DESCRIPTION: 
                  dummy operation to meet the timing restrictions 

    RETURNS:     
                 N/A.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_nop(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 t;
	t=SMSC_READ(dev,BYTE_TEST);
}

/********************************************************************
    FUNCTION:    smsc_e2prom_ops

    PARAMETERS:  
                type: operation type.
                 		E2P_CMD_EPC_CMD_READ_		read
				E2P_CMD_EPC_CMD_WRITE_	write
				E2P_CMD_EPC_CMD_EWDS_	erase/write disable
				E2P_CMD_EPC_CMD_EWEN_		erase/read enable
				E2P_CMD_EPC_CMD_WRAL_		write data to all the eeprom memory if erase/write was enabled.
				E2P_CMD_EPC_CMD_ERASE_	erase the memory specified by the addr.
				E2P_CMD_EPC_CMD_ERAL_		erase all the eeprom memory.
				E2P_CMD_EPC_CMD_RELOAD_	MAC address reload.
				
               addr: the eeprom address.
               data: 
               
    DESCRIPTION: 
                  eeprom operations                  

    RETURNS:     
                 N/A.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
u_int8 smsc_e2prom_ops(struct _DV_DEVICE_ENTRY *dev,u_int32 type,u_int8 addr, u_int8 data)
{
	u_int32 cmd;
	u_int32 timeout=1000;
	u_int32 tmp,tmp2;

	
	cmd=0x80000000;
	cmd|=type;
	cmd|=addr;
	//cmd|=E2P_CMD_EPC_TIMEOUT_;

	if(type == E2P_CMD_EPC_CMD_WRITE_ || type == E2P_CMD_EPC_CMD_WRAL_)
		SMSC_WRITE(dev,E2P_DATA,data);
	SMSC_WRITE(dev,E2P_CMD,cmd);
	NU_Sleep(30);
	timeout=1000;
	do{
		timeout--;
		tmp = 0;
		tmp = SMSC_READ(dev,E2P_CMD);
		tmp2= tmp & E2P_CMD_EPC_BUSY_;
	}while( (tmp2>0) && timeout>0);
	tmp=0;
	if(type == E2P_CMD_EPC_CMD_READ_)
		 tmp=SMSC_READ(dev,E2P_DATA);
	return tmp;
}

/********************************************************************
    FUNCTION:    smsc_read_fifo

    PARAMETERS:  
                dest: the destination buffer, whose address should be 32-bit allignment.
                port: smsc fifo port address.
                nDword: the total number of data want to read from the fifo port, in Dwords (32-bits)
               
    DESCRIPTION: 
                  smsc fifo read.                  

    RETURNS:     
                 Always return success.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static  ETH_STATUS smsc_read_fifo(struct _DV_DEVICE_ENTRY *dev, u_int32* dest,SMSC_PORT port, u_int32 nDwords)
{
	
#if (SMSC_TRANSFER_MODE==SMSC_PIO)
	while (nDwords--)
	{				
		*dest= SMSC_READ(dev,port);
		dest++;		
	}
#else
	u_int32 ch;
	bool ks;
	u_int32 dwSrc,dwDest;
	dwDest=(u_int32)dest & 0xFFFFFFFC;	

	if (nDwords <= (SMSC_DMA_THRESHOLD>>2))
	{			
		while (nDwords--)
		{				
			*dest= SMSC_READ(dev,port);
			dest++;		
		}	
	}
	else
	{

		/* 
		 *	Wether other task is using this channel, if ture, wait ...  
		 */		
		 ch=priv->dma_rx_channel;
		while (*((volatile u_int32 *)DMA_STATUS_REG_CH(ch))&DMA_ACTIVE);

		SMSC_ENTER_CRITICAL_REGION(ks);		
		*(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG |  DMA_MULTIPLE;

		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) =
			DMA_CHANNEL_RECOVERY_TIME(15) | /* Recvry Time after DMA Op */
			DMA_XACT_SZ(1)     						| /* 1 words    */
			DMA_CHANNEL_NON_PACED					| /* Not use the DMAREQ*/
			DMA_XFER_MODE_SINGLE ;                         /*  not Req line triggers xfer   */	

		dwSrc=(dev->dev_io_addr+port);


		dwDest|=0x10000000;

		/* Set the RX_DATA_FIFO port I/O address as the SRC address*/
		*((volatile u_int32 *)DMA_SRCBASE_REG_CH(ch)) = dwSrc;
		*((volatile u_int32 *)DMA_SRCADDR_REG_CH(ch)) = dwSrc;
		*((volatile u_int32 *)DMA_SRCBUF_REG_CH(ch)) =0;        	
		
		/* Set DMA Destination Base Address to Base of Buffer to receive DATA	*/
		*((volatile u_int32 *)DMA_DSTBASE_REG_CH(ch)) =	 dwDest |0x1;
		*((volatile u_int32 *)DMA_DSTADDR_REG_CH(ch)) = dwDest ;
		*((volatile u_int32 *)DMA_DSTBUF_REG_CH(ch)) =(nDwords)<<2;

		/*
		* There are two methods to see if a DMA transfer finish.
		* One, Wait the DST_FULL INT or polling this bit in CX2415X Register (DMA_CHn_STAT_REG).
		* Two, Wait the RXD_INT  INT or polling this bit in SMSC9118 Register(INT_STS)
		* we use the first way, Since SMSC only support rx dma interrupt, nevertheless, we need tx 
		* and rx int;
		*
		*/
		
		/* Clear Status */	
		*((volatile u_int32 *)DMA_STATUS_REG_CH(ch))|= 0x7F;
		
#if    (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)
		FlushAndInvalDCacheRegion((u_int8*)dest,nDwords<<2);
#elif (CPU_TYPE==CPU_ARM940T)
		CleanDCache();
		FlushDCache();
		DrainWriteBuffer();    
#endif
		/*
		* Start DMA
		*/
#ifndef DMA_POLLING
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) |= DMA_ENABLE | DMA_INT_ENABLE;
#else
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) |= DMA_ENABLE ;
#endif

		SMSC_LEAVE_CRITICAL_REGION(ks);
#ifndef DMA_POLLING
		SMSC_LOCK(priv->dma_rx_sem,KAL_WAIT_FOREVER);	
		SMSC_LOCK(priv->dma_rx_sem,0);		
#else
		while((*(volatile u_int32 *)DMA_STATUS_REG_CH(ch)&DMA_DST_FULL)==0);
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) =0; 
#endif		


	}
#endif
	return ETH_SUCCESS;
}	
/********************************************************************
    FUNCTION:    smsc_write_fifo

    PARAMETERS:  
                src: the source buffer, whose address should be 32-bit allignment.
                port: smsc fifo port address.
                nDword: the total number of data want to write to the fifo port, in Dwords (32-bits)
               
    DESCRIPTION: 
                  write data into the smsc fifo.                 

    RETURNS:     
                 Always return success.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static  ETH_STATUS smsc_write_fifo(struct _DV_DEVICE_ENTRY *dev, u_int32* src,SMSC_PORT port, u_int32 nDwords)
{
	
#if (SMSC_TRANSFER_MODE==SMSC_PIO)
	while (nDwords--)
		{				
			SMSC_WRITE(dev,port,*src);
			src++;		
		}
#else

	u_int32 dwSrc;
	bool ks;
	u_int32 ch;
	
	dwSrc=(u_int32)src & 0xFFFFFFFC;	
	if (nDwords <= (SMSC_DMA_THRESHOLD>>2))
	{			
		while (nDwords--)
		{				
			SMSC_WRITE(dev,port,*src);
			src++;	
		}	
	}
	
	else
	{
		ch=priv->dma_tx_channel;
		
		SMSC_ENTER_CRITICAL_REGION(ks);
		*(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG | DMA_MULTIPLE;

		 *((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) =
		     DMA_CHANNEL_RECOVERY_TIME(15) | /* Recvry Time after DMA Op */
		     DMA_XACT_SZ(1) 				| /* 1 words    */
		     DMA_CHANNEL_NON_PACED					| /* Not use the DMAREQ*/
		     DMA_XFER_MODE_SINGLE;                                  /* Req line triggers xfer   */

		/* Set the TX_DATA_FIFO port I/O address as the SRC address*/
		/* Since the SRC is port, increamental address should be disabled */
	    *((volatile u_int32 *)DMA_DSTBASE_REG_CH(ch)) = 	dev->dev_io_addr + port;
	    *((volatile u_int32 *)DMA_DSTADDR_REG_CH(ch)) = 	dev->dev_io_addr + port;
		*((volatile u_int32 *)DMA_DSTBUF_REG_CH(ch)) =0;        	

		 /* Set DMA Destination Base Address to Base of Buffer to receive DATA */
		 dwSrc|=0x10000000;
	    *((volatile u_int32 *)DMA_SRCBASE_REG_CH(ch)) = 	dwSrc |DMA_SRCINCR;
	    *((volatile u_int32 *)DMA_SRCADDR_REG_CH(ch)) =	dwSrc;
	    *((volatile u_int32 *)DMA_SRCBUF_REG_CH(ch)) =(nDwords<<2);

#if    (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)
		FlushAndInvalDCacheRegion((u_int8*)src,nDwords<<2);
#elif (CPU_TYPE==CPU_ARM940T)
		CleanDCache();
		FlushDCache();
		DrainWriteBuffer();    
#endif		

		/* Clear Status */	
		*((volatile u_int32 *)DMA_STATUS_REG_CH(ch))|= 0x7 ;

	    /* Start TX DMA */
#ifndef DMA_POLLING
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) |= DMA_ENABLE | DMA_INT_ENABLE;
#else
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) |= DMA_ENABLE ;
#endif
		SMSC_LEAVE_CRITICAL_REGION(ks); 

#ifndef DMA_POLLING
		SMSC_LOCK(priv->dma_tx_sem,KAL_WAIT_FOREVER);	
		SMSC_LOCK(priv->dma_tx_sem,0);		
#else
		while((*(volatile u_int32 *)DMA_STATUS_REG_CH(ch)&DMA_SRC_EMPTY)==0);
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) =0; 
#endif				
					
	}
#endif

	return ETH_SUCCESS;
}


/********************************************************************
    FUNCTION:    smsc_mac_read_reg

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  read MAC Control and Status register.

    RETURNS:     
                 the value of specified  register.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
u_int32 smsc_mac_read_reg(struct _DV_DEVICE_ENTRY *dev, MAC_REG reg)
{
	u_int32 dwVal=0xFFFFFFFF;
	
	// wait until not busy or timeout.
	if (smsc_mac_notbusy(dev)==FALSE)
	{
		SMSC_ISR_TRACE("smsc_mac_read_reg() failed, the MAC is busy now\n",0,0);
		goto DONE;
	}

	// send the MAC Cmd w/ offset
	SMSC_WRITE(dev,MAC_CSR_CMD,
		((reg & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_ | MAC_CSR_CMD_R_NOT_W_));

	smsc_nop(dev);

	// wait for the read to happen, w/ timeout
	if (smsc_mac_notbusy(dev)!=TRUE)
	{
		SMSC_ISR_TRACE("smsc_mac_read_reg() failed, the MAC is busy all the time before read\n",0,0);
		goto DONE;
	} else {
		dwVal=SMSC_READ(dev,MAC_CSR_DATA);
	}
DONE:
	return dwVal;

}


/********************************************************************
    FUNCTION:    smsc_mac_write_reg

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  write data into the MAC Control and Status register.

    RETURNS:     
                 Always return success.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
void smsc_mac_write_reg(struct _DV_DEVICE_ENTRY *dev, MAC_REG reg,u_int32 dwVal)
{
	//Assuming MacPhyAccessLock has already been acquired
	if (smsc_mac_notbusy(dev)==FALSE)
	{
		SMSC_ISR_TRACE("smsc_mac_read_reg() failed, the MAC is busy now\n",0,0);
		goto DONE;
	}
	
	// send the data to write
	SMSC_WRITE(dev,MAC_CSR_DATA,dwVal);

	// do the actual write
	SMSC_WRITE(dev,MAC_CSR_CMD,((reg & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_));
	smsc_nop(dev);
	
	// wait for the write to complete, w/ timeout
	if (smsc_mac_notbusy(dev)==FALSE)
	{
		SMSC_ISR_TRACE("Mac_SetRegDW() failed, waiting for MAC not busy after write\n",0,0);
	}
DONE:
	return;
}


/********************************************************************
    FUNCTION:    smsc_mac_init

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  Initialize the MAC address from eeprom.

    RETURNS:     
                 N/A
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
extern u_int8 mac_addr[6];
#if 0
static void smsc_mac_init(struct _DV_DEVICE_ENTRY *dev)
{	
	/* set MAC address */
	smsc_mac_set_address(dev,mac_addr);

	/*Multicasting filtering init.*/
	/* to do */
}
#endif

static void smsc_mac_get_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6])
{
	u_int32 hi,lo;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);
	hi=smsc_mac_read_reg(dev,ADDRH);
	lo=smsc_mac_read_reg(dev,ADDRL);
	SMSC_UNLOCK(priv->MacPhyAccessLock);
	
	mac[0]=lo&0xFF;
	mac[1]=(lo>>8)&0xFF;
	mac[2]=(lo>>16)&0xFF;
	mac[3]=(lo>>24)&0xFF;
	mac[4]=hi&0xFF;
	mac[5]=(hi>>8)&0xFF;	
}
static void smsc_mac_set_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6])
{
	u_int32 hi,lo;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);
	
	hi=mac[4] | mac[5]<<8;
	lo=mac[0] | mac[1]<<8 | mac[2]<<16 | mac[3]<<24;

	smsc_mac_write_reg(dev,ADDRH,hi);
	smsc_mac_write_reg(dev,ADDRL,lo);
	
	dev->dev_mac_addr[0]=(lo&0xFF);
	dev->dev_mac_addr[1]=(lo>>8)&0xFF;
	dev->dev_mac_addr[2]=(lo>>16)&0xFF;
	dev->dev_mac_addr[3]=(lo>>24)&0xFF;
	dev->dev_mac_addr[4]=(hi&0xFF);
	dev->dev_mac_addr[5]=(hi>>8)&0xFF;

}

static void smsc_Get_ip(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo= dev->dev_addr.dev_ip_addr;
	ip[3]=lo&0xFF;
	ip[2]=(lo>>8)&0xFF;
	ip[1]=(lo>>16)&0xFF;
	ip[0]=(lo>>24)&0xFF;
	
}
static void smsc_Set_ip(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo=ip[3] | ip[2]<<8 | ip[1]<<16 | ip[0]<<24;

	dev->dev_addr.dev_ip_addr=lo;
}

static void smsc_Get_mask(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo= dev->dev_addr.dev_netmask;
	ip[3]=lo&0xFF;
	ip[2]=(lo>>8)&0xFF;
	ip[1]=(lo>>16)&0xFF;
	ip[0]=(lo>>24)&0xFF;
	
}
static void smsc_Set_mask(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo=ip[3] | ip[2]<<8 | ip[1]<<16 | ip[0]<<24;

	dev->dev_addr.dev_netmask=lo;
}
/********************************************************************
    FUNCTION:    smsc_mac_notbusy

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  Check if the MAC is not busy now.
                  
    RETURNS:     
                 TRUE: not busy.
                 FALSE: busy now.    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static bool smsc_mac_notbusy(struct _DV_DEVICE_ENTRY *dev)
{
	int32 i=0;
	int32 timeout=100000;

	//Assuming MacPhyAccessLock has already been acquired
	for(i=0;i<timeout;i++)
	{
		if((SMSC_READ(dev,MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)==(0UL)) {
			return TRUE;
		}
	}
	return FALSE;
}


/********************************************************************
    FUNCTION:    smsc_phy_read_reg

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  read Physical Control and Status register.

    RETURNS:     
                 the value of specified physical register.
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
u_int32 smsc_phy_read_reg(struct _DV_DEVICE_ENTRY *dev, PHY_REG reg)
{
	u_int32 dwVal=0;
	u_int32 dwAddr;
	int32 timeout=1000;
	int i=0;
   	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);

     SMSC_ASSERT(priv!=NULL);

	while ((smsc_mac_read_reg(dev,MII_ACC) & MII_ACC_MII_BUSY_) != 0UL && timeout)
	{
          timeout--;  
      }

	if (timeout<=0)
	{		
		SMSC_ISR_TRACE("smsc_phy_read_reg: MII is busy !\n",0,0);
		/* 
			Since this function was freqently called by the smsc_phy_get_link_mode(),
			returning the PHY_BSR_LINK_STATUS which indication that the link is up now 
			will avoid the displaying the message " LINK DOWN" because this situation is 
			not a really link down.
		*/		
		return PHY_BSR_LINK_STATUS_;
	}
	
	// set the address, index & direction (read from PHY)
	dwAddr = ((priv->PhyAddessId)<<11) | ((reg & 0x1FUL)<<6);
	smsc_mac_write_reg(dev, MII_ACC, dwAddr);

	// wait for read to complete w/ timeout
	for(i=0;i<timeout;i++) {
		// see if MII is finished yet
		if ((smsc_mac_read_reg(dev, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{
			// get the read data from the MAC & return i
			dwVal=((u_int32)smsc_mac_read_reg(dev, MII_DATA));
			goto DONE;
		}
	}
	SMSC_ISR_TRACE("smsc_phy_read_reg:timeout waiting for MII write to finish\n",0,0);

DONE:
	return dwVal;
}


/********************************************************************
    FUNCTION:    smsc_phy_write_reg

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  write data into Physical Control and Status register.

    RETURNS:     
                 N/A
                 
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
void smsc_phy_write_reg(struct _DV_DEVICE_ENTRY *dev, PHY_REG reg,u_int32 dwVal)
{
	u_int32 dwAddr;
	int i=0;
	u_int32 timeout=10000;
     smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
     SMSC_ASSERT(priv!=NULL);

	// confirm MII not busy
	if ((smsc_mac_read_reg(dev,MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
	{
		SMSC_ISR_TRACE("smsc_phy_write_reg: MII is busy !\n",0,0);
		return;
	}

	// put the data to write in the MAC
	smsc_mac_write_reg(dev, MII_DATA, dwVal);

		// set the address, index & direction (read from PHY)
	dwAddr = ((priv->PhyAddessId)<<11) | ((reg & 0x1FUL)<<6)|MII_ACC_MII_WRITE_;
	smsc_mac_write_reg(dev, MII_ACC, dwAddr);

	// wait for write to complete w/ timeout
	for(i=0;i<timeout;i++) {
		// see if MII is finished yet
		if ((smsc_mac_read_reg(dev, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
		{			
			return;
		}
	}
	SMSC_ISR_TRACE("smsc_phy_write_reg:timeout waiting for MII write to finish\n",0,0);
	
}


/********************************************************************
    FUNCTION:    smsc_phy_set_link

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  set the physical link mode.

    RETURNS:     
                 N/A 
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_phy_set_link(struct _DV_DEVICE_ENTRY *dev,u_int32 option) 
{
	u_int32 dwBCR,dwANA;
		
	dwBCR=smsc_phy_read_reg(dev,PHY_BCR);
	dwANA=smsc_phy_read_reg(dev,PHY_ANEG_ADV);
	
	dwBCR&=~(PHY_BCR_AUTO_NEG_ENABLE_
					|PHY_BCR_DUPLEX_MODE_
					|PHY_BCR_RESTART_AUTO_NEG_
					|PHY_BCR_SPEED_SELECT_);
	dwANA&=~(PHY_ANEG_ADV_PAUSE_|PHY_ANEG_ADV_SPEED_);

	/*Auto Negotiation ADV
		Bit9  Bit8 Bit7 Bit6
		 1	   0     X     X    100M FD
		 0      1     X    X     100M HD
		 X      X     1    0     10M   FD
		 X      X     0    1     10M   HD
	 */	
	
	switch(option)
	{
		case LINK_SPEED_100FD:
			dwBCR|=(BIT13|BIT8);
			dwANA|=(PHY_ANEG_ADV_100F_);
			break;
		case LINK_SPEED_100HD:
			dwBCR|=(BIT13);
			dwANA|=(PHY_ANEG_ADV_100H_);
			break;
		case LINK_SPEED_10FD:
			dwBCR|=(BIT8);
			dwANA|=(PHY_ANEG_ADV_10F_);
			break;
		case LINK_SPEED_10HD:			
			dwBCR|=0;
			dwANA|=(PHY_ANEG_ADV_10H_);
			break;
		default:
			dwBCR|=(PHY_BCR_AUTO_NEG_ENABLE_|PHY_BCR_RESTART_AUTO_NEG_);
			dwANA|=(PHY_ANEG_ADV_PAUSE_|PHY_ANEG_ADV_SPEED_);
			break;			
	}	

	smsc_phy_write_reg(dev,PHY_BCR,dwBCR);	
	smsc_phy_write_reg(dev,PHY_ANEG_ADV,dwANA);
						
}


/********************************************************************
    FUNCTION:    smsc_phy_polling_timer

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  the timer handler to polling current physical link status.

    RETURNS:     
                 N/A 
    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_phy_polling_timer(tick_id_t hTick, void *pUserData)
{
	struct _DV_DEVICE_ENTRY *dev=smsc_dev;	

	SMSC_ASSERT(dev!=NULL);

	//must call this twice
	smsc_phy_update_link(dev);
	smsc_phy_update_link(dev);
}

/********************************************************************
    FUNCTION:    smsc_phy_init

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  Initialize the physical layer.
                  
    RETURNS:     
                     
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_phy_init(struct _DV_DEVICE_ENTRY *dev)
{

	ETH_STATUS result=ETH_ERROR;    
	u_int16 wPhyId1=0;
	u_int16 wPhyId2=0;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
     u_int32 dwIdRev;
     u_int32 dwHwCfg;
     int32 i;
     int32 flag = 0;

	SMSC_ASSERT(priv!=NULL);
     priv->PhyAddessId = 1;
   
 	dwIdRev = SMSC_READ(dev, ID_REV);

     if (dwIdRev & 0xFFFF == 0) 
     {
         goto DONE;
     }

     switch (dwIdRev & 0xFFFF0000)
     {
        case 0x01150000:
        case 0x01170000: 
         
                  /* external phy */                  
                  dwHwCfg = SMSC_READ(dev,HW_CFG);
                  if(dwHwCfg&HW_CFG_EXT_PHY_DET_) 
                  {

                         SMSC_TRACE("Detected External Phy !");
                         wPhyId1=0;
				    wPhyId2=0;

                         //Disable phy clocks to the mac
				    dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
				    dwHwCfg|= HW_CFG_PHY_CLK_SEL_CLK_DIS_;
			         SMSC_WRITE(dev,HW_CFG,dwHwCfg);

                         /* wait for clock to actually stop */
                         task_time_sleep(100);

                         /* enable external phy */
                         dwHwCfg|=HW_CFG_EXT_PHY_EN_;
				    SMSC_WRITE(dev,HW_CFG,dwHwCfg);

                         dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
				    dwHwCfg|= HW_CFG_PHY_CLK_SEL_EXT_PHY_;
				    SMSC_WRITE(dev,HW_CFG,dwHwCfg);
                         task_time_sleep(100);

                         dwHwCfg|=HW_CFG_SMI_SEL_;
				    SMSC_WRITE(dev,HW_CFG,dwHwCfg);

                         for (i=0;i<32;i++)
                         {
                              priv->PhyAddessId = i;
                              wPhyId1 = smsc_phy_read_reg(dev,PHY_ID1);
                              wPhyId2 = smsc_phy_read_reg(dev,PHY_ID2);
                              if ((wPhyId1!=0x0U)||(wPhyId2!=0x0U)) 
                              {
						   SMSC_TRACE("External Phy at address = %ld\n",i);
                                   flag = 1;
                                   break;
					    }
                         }								
                         if (flag == 1)
                         {
                           break;
                         }
                         else
                         {
                               SMSC_TRACE("Init external Phy error , Use Internal Phy Instead!\n"); 
                               	dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
						dwHwCfg|= HW_CFG_PHY_CLK_SEL_CLK_DIS_;
						SMSC_WRITE(dev,HW_CFG,dwHwCfg);
                                task_time_sleep(100);

						dwHwCfg&=(~HW_CFG_EXT_PHY_EN_);
						SMSC_WRITE(dev,HW_CFG,dwHwCfg);
	
						dwHwCfg&= (~HW_CFG_PHY_CLK_SEL_);
						dwHwCfg|= HW_CFG_PHY_CLK_SEL_INT_PHY_;
						SMSC_WRITE(dev,HW_CFG,dwHwCfg);
                                task_time_sleep(100);

						dwHwCfg&=(~HW_CFG_SMI_SEL_);
						SMSC_WRITE(dev,HW_CFG,dwHwCfg);
                         }
                  
                     }         

        /* If external phy detected failed or it is 9116 or 9118*/    
        case 0x01160000: 
        case 0x01180000:  
          priv->PhyAddessId = 1;
         	wPhyId1=smsc_phy_read_reg(dev,PHY_ID1);
	     wPhyId2=smsc_phy_read_reg(dev,PHY_ID2);
	     if((wPhyId1==0xFFFFU)&&(wPhyId2==0xFFFFU)) 
          {
		   result=ETH_NO_HARDWARE;
		   SMSC_WARNING("Phy Not detected.\n");
		   goto DONE;
	     }
               
           break;
           
        default:
            break;
     }

	priv->dwLinkSpeed=LINK_OFF;
	priv->dwLinkSettings=LINK_OFF;
	
	if(ETH_SUCCESS!=smsc_phy_reset(dev)) {
		result=ETH_ERROR;
		SMSC_WARNING("PHY reset failed to complete.\n");
		goto DONE;
	}

	/*set link mode*/
	smsc_phy_set_link(dev,0); //0->Auto Neogotiation.

	
	result=ETH_SUCCESS;
DONE:
	return result;
}

/********************************************************************
    FUNCTION:    smsc_phy_update_link

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  Update the current link status.
                  
    RETURNS:     
                  N/A   
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_phy_update_link(struct _DV_DEVICE_ENTRY *dev)
{
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	
	u_int32 dwOldLinkSpeed=priv->dwLinkSpeed;
	u_int32 linkPartner=0;
	u_int32 localLink=0;
	u_int32 dwRegVal=0;

	//extern sem_id_t  netStatusSemID;
	//extern u_int8	gNetStatus;
	
	SMSC_ASSERT(priv!=NULL);

	smsc_phy_get_link_mode(dev);


#ifdef	OLD_BOARD
	/*
		maybe another hardware bug too.  Due to current bus connecting way,the
		smsc_read and smsc_write are  not	atomic operations, some exceptions 
		may occur during the two 16-bits bus access of the two operations. some 
		bits of SMSC register may be changed.		
	 */
	if (0==(SMSC_READ(dev,AFC_CFG)&0xFF0000))
	{					
		SMSC_ISR_TRACE("AFC_CFG=0x%x,FLOW=0x%x\n",
					SMSC_READ(dev,AFC_CFG),
					smsc_mac_read_reg(dev,FLOW));
		SMSC_WRITE(dev,AFC_CFG,(SMSC_AFC_HI<<16 | SMSC_AFC_LO<<8 | 0xF ));

		return;
	}
#endif	
	
	if(dwOldLinkSpeed!=(priv->dwLinkSpeed)) { /*Link Status Changed*/
		
		if(priv->dwLinkSpeed!=LINK_OFF) {	/*Link Speed changed*/					

			dwRegVal=0;
			switch(priv->dwLinkSpeed) 
			{				
				case LINK_SPEED_10HD:
					SMSC_ISR_TRACE("Link is now UP at 10Mbps HD\n",0,0);
					break;
				case LINK_SPEED_10FD:
					SMSC_ISR_TRACE("Link is now UP at 10Mbps FD\n",0,0);
					break;
				case LINK_SPEED_100HD:
					SMSC_ISR_TRACE("Link is now UP at 100Mbps HD\n",0,0);
					break;
				case LINK_SPEED_100FD:
					SMSC_ISR_TRACE("Link is now UP at 100Mbps FD INT_STS=0x%x, INT_EN=0x%x\n",SMSC_READ(dev,INT_STS),SMSC_READ(dev,INT_EN));
					break;
				default:
					SMSC_ISR_TRACE("Link is now UP at Unknown Link Speed, dwLinkSpeed=0x%08lX\n",priv->dwLinkSpeed,0);
				break;
			}
		
			dwRegVal=smsc_mac_read_reg(dev,MAC_CR);
			dwRegVal&=~(MAC_CR_FDPX_|MAC_CR_RCVOWN_);
			
			switch(priv->dwLinkSpeed) {
				
				case LINK_SPEED_10HD:
				case LINK_SPEED_100HD:
					dwRegVal|=MAC_CR_RCVOWN_;
					break;
				case LINK_SPEED_10FD:
				case LINK_SPEED_100FD:
					dwRegVal|=MAC_CR_FDPX_;
					break;
				default:
					SMSC_ISR_TRACE("Unknown Link Speed, dwLinkSpeed=0x%08lX\n",priv->dwLinkSpeed,0);
					break;
			}

			smsc_mac_write_reg(dev,MAC_CR,dwRegVal);

			if (priv->dwLinkSettings&LINK_AUTO_NEGOTIATE) 
			{
				localLink=smsc_phy_read_reg(dev,PHY_ANEG_ADV);
				linkPartner=smsc_phy_read_reg(dev,PHY_ANEG_LPA);
				
				switch(priv->dwLinkSpeed) {
					
					case LINK_SPEED_10FD:
					case LINK_SPEED_100FD:
						if(((localLink&linkPartner)&((u_int32)0x0400U)) != ((u_int32)0U)) 
						{
							//	Local & parter : Symmetric supported.
							// Enable PAUSE receive and transmit
							smsc_mac_write_reg(dev,FLOW,0xFFFF0002UL);
							//smsc_set_bits(dev,AFC_CFG,0x0000000FUL);							
						} 
						else if(((localLink&((u_int32)0x0C00U))==((u_int32)0x0C00U)) &&
								((linkPartner&((u_int32)0x0C00U))==((u_int32)0x0800U)))
						{
							// local: Both Symmetric and Asymmetric supported.
							// partner: Asymmetric supported.
							// Enable PAUSE receive, disable PAUSE transmit
							smsc_mac_write_reg(dev,FLOW,0xFFFF0002UL);
							//smsc_clear_bits(dev,AFC_CFG,0x0000000FUL);
						} 
						else 
						{
							//Disable PAUSE receive and transmit
							smsc_mac_write_reg(dev,FLOW,0UL);
							//smsc_clear_bits(dev,AFC_CFG,0x0000000FUL);
						}
						break;
					case LINK_SPEED_10HD:
					case LINK_SPEED_100HD:
						//smsc_mac_write_reg(dev,FLOW,0xFFFF0002UL);
						smsc_mac_write_reg(dev,FLOW,0x0);
						//smsc_set_bits(dev,AFC_CFG,0x0000000FUL);
						break;
					default:
						SMSC_ISR_TRACE("Unknown Link Speed, dwLinkSpeed=0x%08lX\n",priv->dwLinkSpeed,0);
						break;
					}
			} 
			else 
			{
				switch(priv->dwLinkSpeed) {
				case LINK_SPEED_10HD:
				case LINK_SPEED_100HD:
					smsc_mac_write_reg(dev,FLOW,0x0UL);
					//smsc_set_bits(dev,AFC_CFG,0x0000000FUL);
					break;
				default:
					smsc_mac_write_reg(dev,FLOW,0x0UL);
					//smsc_clear_bits(dev,AFC_CFG,0x0000000FUL);
					break;
				}
			}

			//gNetStatus = 1;
			//sem_put(netStatusSemID);
		}
		else {
		
			/* link down */
			SMSC_ISR_TRACE("Link Down ! IRQ_CFG=0x%x, AFC_CFG=0x%x, ",SMSC_READ(dev,IRQ_CFG),SMSC_READ(dev,AFC_CFG));
			SMSC_ISR_TRACE("INT_EN=0x%x, INT_STS=0x%x\n",SMSC_READ(dev,INT_EN),SMSC_READ(dev,INT_STS));
			smsc_mac_write_reg(dev,FLOW,0);
			//smsc_clear_bits(dev,AFC_CFG,0x0F);

			//gNetStatus = 0;
			//sem_put(netStatusSemID);
		}
	}
	//SMSC_UNLOCK(priv->MacPhyAccessLock);
}


/********************************************************************
    FUNCTION:    smsc_phy_get_link_mode

    PARAMETERS:  
                
               
    DESCRIPTION: 
                  get the current link mode.
                  
    RETURNS:     
                  N/A   
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_phy_get_link_mode(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 result=LINK_OFF;
	u_int32 wRegBCR=0;
	u_int32 wRegBSR=0;

	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	wRegBSR=smsc_phy_read_reg(dev,PHY_BSR);
	
	priv->dwLinkSettings=LINK_OFF;
	
	if(wRegBSR&PHY_BSR_LINK_STATUS_) {

		/* link is up */
		
		wRegBCR=smsc_phy_read_reg(dev,	PHY_BCR);
				
		if( (wRegBCR&PHY_BCR_AUTO_NEG_ENABLE_) ){  			/* Auto Negotiate */
			
			u_int32 linkSettings=LINK_AUTO_NEGOTIATE;
			u_int32 wRegADV=smsc_phy_read_reg(dev,PHY_ANEG_ADV);
			u_int32 wRegLPA=smsc_phy_read_reg(dev,PHY_ANEG_LPA);

			if(wRegADV&PHY_ANEG_ADV_ASYMP_) {
				linkSettings|=LINK_ASYMMETRIC_PAUSE;
			}
			
			if(wRegADV&PHY_ANEG_ADV_SYMP_) {
				linkSettings|=LINK_SYMMETRIC_PAUSE;
			}
			
			if(wRegADV&PHY_ANEG_LPA_100FDX_) {
				linkSettings|=LINK_SPEED_100FD;
			}
			
			if(wRegADV&PHY_ANEG_LPA_100HDX_) {
				linkSettings|=LINK_SPEED_100HD;
			}
			
			if(wRegADV&PHY_ANEG_LPA_10FDX_) {
				linkSettings|=LINK_SPEED_10FD;
			}
			
			if(wRegADV&PHY_ANEG_LPA_10HDX_) {
				linkSettings|=LINK_SPEED_10HD;
			}
			
			priv->dwLinkSettings=linkSettings;
			wRegLPA&=wRegADV;
			
			if(wRegLPA&PHY_ANEG_LPA_100FDX_) {
				result=LINK_SPEED_100FD;
			} else if(wRegLPA&PHY_ANEG_LPA_100HDX_) {
				result=LINK_SPEED_100HD;
			} else if(wRegLPA&PHY_ANEG_LPA_10FDX_) {
				result=LINK_SPEED_10FD;
			} else if(wRegLPA&PHY_ANEG_LPA_10HDX_) {
				result=LINK_SPEED_10HD;
			}
		} 
		else { 										

			if(wRegBCR&PHY_BCR_SPEED_SELECT_) {					/*100M*/
				if(wRegBCR&PHY_BCR_DUPLEX_MODE_) {
					priv->dwLinkSettings=result=LINK_SPEED_100FD;	
				} else {
					priv->dwLinkSettings=result=LINK_SPEED_100HD;
				}
			} else {												/*10M*/
				if(wRegBCR&PHY_BCR_DUPLEX_MODE_) {
					priv->dwLinkSettings=result=LINK_SPEED_10FD;
				} else {
					priv->dwLinkSettings=result=LINK_SPEED_10HD;
				}
			}
		}
	}
	priv->dwLinkSpeed=result;
}


static ETH_STATUS smsc_phy_reset(DV_DEVICE_ENTRY *dev)
{
	int32 timeout=1000;
	u_int32 dwVal;
	ETH_STATUS result=ETH_SUCCESS;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	smsc_phy_write_reg(dev,PHY_BCR,PHY_BCR_RESET_);
	smsc_nop(dev);
	do{
		timeout--;
		dwVal=smsc_phy_read_reg(dev,PHY_BCR);
	}while (timeout>0 && (dwVal & PHY_BCR_RESET_));
	
	if (timeout<=0)
		result=ETH_ERROR;

	return result;

}


/********************************************************************
    FUNCTION:    smsc_ioctl

    PARAMETERS:  
                  option : control option.
                  request: 
               
    DESCRIPTION: 
                  provide the I/O control interface with upper application.
                  
    RETURNS:     
                  N/A   
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_ioctl( DV_DEVICE_ENTRY *dev, int option, DV_REQ *request )
{
	STATUS result  = ETH_SUCCESS;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);
	
	switch (option)
	{
		case DEV_ADDMULTI:
			NET_Add_Multi(dev, request);
			result=smsc_set_multicast(dev);
			break;
		case DEV_DELMULTI:
			NET_Del_Multi(dev, request);
			result=smsc_set_multicast(dev);
			break;
		case DEV_GET_MAC:
			smsc_mac_get_address(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_MAC:	
			smsc_mac_set_address(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_GET_IP:
			smsc_Get_ip(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_IP:
			smsc_Set_ip(dev, request->dvr_dvru.dvru_data);
		break;
		case DEV_GET_MASK:
			smsc_Get_mask(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_MASK:
			smsc_Set_mask(dev, request->dvr_dvru.dvru_data);
			break;
						
		case DEV_GET_LINK_STATE:
			request->dvr_dvru.drvu_flags = priv->dwLinkSpeed;
			break;
	}
   return result;
}

/********************************************************************
    FUNCTION:    smsc_init
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Initialize the smsc911x driver.
                  
    RETURNS:     
                  N/A   
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_init( struct _DV_DEVICE_ENTRY *dev )
{
	DV_REQ 						 req;
	//u_int32 i,dwV;
	volatile u_int32 *dwAddr=0;
   
	dev->dev_open = smsc_open;
	dev->dev_start = smsc_Tx_hard_xmit;
	dev->dev_ioctl =  smsc_ioctl;
	dev->dev_output = NET_Ether_Send;
	dev->dev_input = NET_Ether_Input;

	dev->dev_type = DVT_ETHER;
	dev->dev_addrlen = DADDLEN;
	dev->dev_hdrlen = sizeof( DLAYER );
	dev->dev_mtu = ETHERNET_MTU;
	dev->dev_flags |= DV_BROADCAST;

	dev->dev_io_addr=SMSC_ISA_IO_BASE;
	
	dev->user_defined_1=(u_int32)(&smsc_priv);

	/*Initialize the ISA Desc*/
	dwAddr=(volatile u_int32 *)SMSC_ISA_DESC_PRM_REG;

#ifdef	OLD_BOARD
	*dwAddr=(PCI_ISA_DESC_TYPE_ISA |
			  	SMSC_WR_WAIT |
			  	SMSC_RD_WAIT |
                     PCI_ISA_DESC_NO_EXT_WAIT_STATES |
                     PCI_ISA_DESC_IO_TYPE_IORD_IOWR|
                     PCI_ISA_DESC_REG_ASSERT_DISABLE|
                     ( SMSC_CS_SEL << PCI_ISA_DESC_CS_SHIFT)|
                     PCI_ROM_DESC_WIDTH_32  
				);
#else
	*dwAddr=(PCI_ISA_DESC_TYPE_ISA|
			  	SMSC_WR_WAIT |
			  	SMSC_RD_WAIT |
			  	PCI_ROM_DESC_BURST_ENABLE |
                     PCI_ISA_DESC_NO_EXT_WAIT_STATES |
                     PCI_ISA_DESC_IO_TYPE_IORD_IOWR |
                     ( SMSC_CS_SEL << PCI_ISA_DESC_CS_SHIFT)|
                     PCI_ROM_DESC_WIDTH_16  
				);
#endif	

	dwAddr=(volatile u_int32 *)SMSC_ISA_DESC_EXT_REG;
	*dwAddr=(SMSC_REG_SETUP|
				SMSC_REG_ACCESS|
				SMSC_CS_SETUP |
				SMSC_CS_HOLD |
				SMSC_CTL_SETUP|
				SMSC_ADDR_HOLD);
	smsc_dev=dev;
	/*
	for(i=0;i<5;i++)
	{
		mac2[i]=smsc_e2prom_ops(dev,E2P_CMD_EPC_CMD_READ_,i+1,0XA5);
	}
	smsc_e2prom_ops(dev,E2P_CMD_EPC_CMD_WRITE_,0,0XA5);
*/
	req.dvr_dvru.dvru_data=dev->dev_mac_addr;
	smsc_ioctl(dev,DEV_GET_MAC,&req);	
	
	/* Initialize the device. */	
	return ( (*(dev->dev_open)) (dev) );

} 


/********************************************************************
    FUNCTION:    smsc_verify_chip

    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  verify if this is a valid smsc911x chip.
                  
    RETURNS:     
                  ETH_SUCESS:    
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_verify_chip(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result;
	u_int32 chip_id;
	
	chip_id=(0xFFFF0000&SMSC_READ(dev, ID_REV))>>16;
	switch (chip_id)
	{
		case 0x0115:
		case 0x0116:
		case 0x0117:
		case 0x0118:
			result=ETH_SUCCESS;
			SMSC_TRACE("smsc_verify_chip: smsc Lan9%x detected!\n",chip_id);
			break;
		default:
			SMSC_WARNING("smsc_verify_chip: Not a smsc Lan911X!\n");
			result=ETH_NO_HARDWARE;
			break;
	}
	
	return result;	
}


/********************************************************************
    FUNCTION:    smsc_chip_reset

    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Soft Reset
                  
    RETURNS:     
                  ETH_SUCESS:   
                  ETH_ERROR:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_chip_reset(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result=ETH_ERROR;
	u_int32 timeout=100;
	u_int32 dwVal;
	smsc_priv_t *priv=NULL;

	priv=(smsc_priv_t *)(dev->user_defined_1);

	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);

	dwVal=SMSC_READ(dev,PMT_CTRL);
	if (dwVal & PMT_CTRL_PM_MODE_)
	{
		SMSC_WRITE(dev,BYTE_TEST,0x0);
		do{
			timeout--;
			dwVal=SMSC_READ(dev,PMT_CTRL);
		}while (timeout>0 && (dwVal & PMT_CTRL_READY_));
			
	}

	SMSC_WRITE(dev,HW_CFG,HW_CFG_SRST_);
	do{
		timeout--;
		dwVal=SMSC_READ(dev,HW_CFG) & HW_CFG_SRST_;
	} while (timeout>0 && dwVal!=0);
	
	SMSC_UNLOCK(priv->MacPhyAccessLock);

	if (timeout==0)
	{
		SMSC_WARNING("smsc_chip_reset() error!");
		result=ETH_ERROR;
	}
	else
	{

		result=ETH_SUCCESS;
	}

	
	return result;	
}


/********************************************************************
    FUNCTION:    smsc_chip_init
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Initialize the chip register.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  ETH_ERROR:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_chip_init(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result=ETH_SUCCESS;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	u_int32 dwVal=0;

	/*disable all int*/
	SMSC_WRITE(dev,INT_EN,0); 

	/* clean the status*/
	SMSC_WRITE(dev,INT_STS,0xFFFFFFFFUL);

	/*push-pull*/
	dwVal=IRQ_CFG_IRQ_TYPE_|IRQ_CFG_IRQ_POL_;
	SMSC_WRITE(dev,IRQ_CFG,dwVal);
	
	/*Flow control as recomanded in AN12.12 rev1.0*/
	SMSC_WRITE(dev,AFC_CFG,(SMSC_AFC_HI<<16 | SMSC_AFC_LO<<8 | 0xF));

	/*Enable Flow Control*/
	smsc_mac_write_reg(dev,FLOW,2);
	
	/*gpio set*/
	SMSC_WRITE(dev,GPIO_CFG,0x70070000UL);	

	/* FIFO Size */
	SMSC_WRITE(dev,HW_CFG,(0x5<<16)|HW_CFG_SF_);
	
	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);

	//smsc_mac_init(dev);
	result = smsc_phy_init(dev);	

	SMSC_UNLOCK(priv->MacPhyAccessLock);
	return result;
}


/********************************************************************
    FUNCTION:    smsc_open
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Open the device and start the Tx/Rx service.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_open(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result;
	smsc_priv_t *priv;
	
	SMSC_ASSERT(dev!=NULL);

	priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	memset((void*)&(priv->stats),0,sizeof(priv->stats));//根据王丽坤邮件修改smsc_priv_t为priv->stats

#if (SMSC_TRANSFER_MODE==SMSC_DMA)
	priv->dma_rx_channel=SMSC_RX_DMA_CHANNEL;
	priv->dma_tx_channel=SMSC_TX_DMA_CHANNEL;
	priv->dma_rx_sem=sem_create(0, "DMAR");
	SMSC_ASSERT(priv->dma_rx_sem!=0);

	priv->dma_tx_sem=sem_create(0, "DMAT");
	SMSC_ASSERT(priv->dma_tx_sem!=0);	

	if (smsc_DMA_init()!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open: smsc_DMA_init() error!\n");
		result=ETH_ERROR;
		goto DONE;
	}			
	
#endif

	priv->msg_id=qu_create(1,"RXQU");
	SMSC_ASSERT(priv->msg_id!=0);

	priv->MacPhyAccessLock=sem_create(1,"MPLK");
	SMSC_ASSERT(priv->MacPhyAccessLock!=0);

	priv->ResetLock=sem_create(1,"RSTL");
	SMSC_ASSERT(priv->ResetLock!=0);

	priv->smsc_task_id=task_create(
						(PFNTASK)smsc_task ,dev,NULL,
						SMSC_STACK_SIZE, SMSC_TASK_PRIO,
						"SMSC" );
	SMSC_ASSERT(priv->smsc_task_id!=0);	

	/* Verify if it is the correct chip */
	result=smsc_verify_chip(dev); 
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open:smsc_verify_chip() error!\n");
		result=ETH_ERROR;
		goto DONE;
	}		 

	/* Reset the chip */
	result=smsc_chip_reset(dev);
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open: smsc_chip_reset() error!\n");
		result=ETH_ERROR;
		goto DONE;
	}	

	/* Init the chip */
	result=smsc_chip_init(dev);
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open: smsc_chip_init(dev) error !\n");
		result=ETH_ERROR;
		goto DONE;
	}		 

	/*create timer to polling the link status*/
	priv->polling_tick_id=tick_create(smsc_phy_polling_timer, NULL, "STCK");
	SMSC_ASSERT(priv->polling_tick_id!=0);
	tick_set(priv->polling_tick_id,1000,FALSE);


	result=smsc_Rx_init(dev);	
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open :smsc_Rx_init(dev) error !\n");
		result=ETH_ERROR;
		goto DONE;
	}
	
	result=smsc_Tx_init(dev);
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open :smsc_Tx_init(dev) error !\n");
		result=ETH_ERROR;
		goto DONE;
	}

	result=smsc_ISR_init(dev);
	if (result!=ETH_SUCCESS)
	{
		SMSC_TRACE("smsc_open: smsc_ISR_init(dev) error !\n");
		result=ETH_ERROR;
		goto DONE;
	}


		
	/*Start working*/
	smsc_set_bits(dev,IRQ_CFG,IRQ_CFG_IRQ_EN_);
	
	tick_start(priv->polling_tick_id);

	DONE:
	return result;
}



/********************************************************************
    FUNCTION:    smsc_fast_recover
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Do a reset and init work to recover smsc from serious error.
                  
    RETURNS:     
                  N/A
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_fast_recover(struct _DV_DEVICE_ENTRY *dev)
{		

	smsc_priv_t *priv=(smsc_priv_t *)(dev->user_defined_1);

	u_int32 dwIntEn;	
	dwIntEn=SMSC_READ(dev,INT_EN);

	dwIntEn&=(INT_EN_RSFL_EN_ | INT_EN_TDFA_EN_);

	SMSC_ASSERT(priv!=NULL);

	smsc_clear_bits(dev,IRQ_CFG,IRQ_CFG_IRQ_EN_);

	/* Make sure no Tx task was running when Reseting */
	SMSC_LOCK(priv->ResetLock,KAL_WAIT_FOREVER);

	smsc_chip_reset(dev);
	smsc_chip_init(dev);	
	smsc_Rx_init(dev);
	smsc_Tx_init(dev);
	SMSC_WRITE(dev,INT_EN,dwIntEn);
	
	smsc_set_bits(dev,IRQ_CFG,IRQ_CFG_IRQ_EN_);
	
	SMSC_UNLOCK(priv->ResetLock);

}



/********************************************************************
    FUNCTION:    smsc_ISR_init
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  initialize and register the smsc Interrupt service routine for RX.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_ISR_init(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result=ETH_SUCCESS;
	PFNISR            pFnChain=NULL;
	/*Set the GPIO INT*/
	int32 tmp;
	cnxt_gpio_set_input(SMSC_HOST_INTERRUPT);
	cnxt_gpio_set_int_edge(SMSC_HOST_INTERRUPT, POS_EDGE);
	cnxt_gpio_clear_pic_interrupt(SMSC_HOST_INTERRUPT);
	tmp=cnxt_gpio_int_register_isr( SMSC_HOST_INTERRUPT, 
						(PFNISR)smsc_ISR, FALSE, FALSE, &pFnChain );
	if( CNXT_GPIO_STATUS_OK != tmp )
	{
		SMSC_TRACE("Error registering ISR handler %s.\n",__FILE__); 
		return ETH_ERROR;
	}

	/*enable GPIO INT*/
	if (CNXT_GPIO_STATUS_OK != cnxt_gpio_int_enable(SMSC_HOST_INTERRUPT))
	{
		SMSC_TRACE("Error enable GPIO INT %s.\n",__FILE__); 
		result= ETH_ERROR ;
	}   
	
#if 0
	/*Enable TX INT */
	
	smsc_set_bits(dev,INT_EN,
		 INT_EN_TXSO_EN_	// status overflow
		| INT_EN_TXE_EN_		// tx error
		| INT_EN_TDFA_EN_	// data free available
		| INT_EN_TDFO_EN_	// data overfow
		| INT_EN_TDFU_EN_	// data underrun
		);

	/*Enable PHY_INT*/
	smsc_phy_write_reg(dev,
				PHY_IMR,
				PHY_INT_MASK_ANEG_COMP_|
				PHY_INT_MASK_LINK_DOWN_|
				PHY_INT_MASK_REMOTE_FAULT_);
	smsc_set_bits(dev,INT_EN,INT_EN_PHY_INT_EN_);

#if (SMSC_TRANSFER_MODE==SMSC_DMA)

	smsc_set_bits(dev,INT_EN,INT_EN_RXD_INT_EN_);  //enable DMA Int.
#endif

#endif
	/*Enable RX INT*/
	smsc_set_bits(dev,INT_EN,INT_EN_RSFL_EN_);


	return result;
}


/********************************************************************
    FUNCTION:    smsc_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  the Interrupt service routine .
                  
    RETURNS:			
                  RC_ISR_HANDLED
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static int32 smsc_ISR(u_int32 irq, bool FIQ, PFNISR *shared)
{
	u_int32 dwIntCfg=0;
	u_int32 dwIntSts=0;
	u_int32 dwIntEn=0;
	u_int32 reservedBits=0x00FFCEEEUL;
	struct _DV_DEVICE_ENTRY *dev=smsc_dev;
	SMSC_ASSERT(dev!=NULL);

	*shared=NULL;

	dwIntCfg=SMSC_READ(dev,IRQ_CFG);
	if(0==(dwIntCfg & (IRQ_CFG_IRQ_EN_|IRQ_CFG_IRQ_INT_))) {
		SMSC_ISR_TRACE("smsc_ISR: dwIntCfg=0x%x\n",dwIntCfg,0);
		return RC_ISR_HANDLED;
	}

	smsc_clear_bits(dev,IRQ_CFG,IRQ_CFG_IRQ_EN_);

	/* reserved Bits check */
	if(dwIntCfg&reservedBits) 
	{
			SMSC_ISR_TRACE("smsc_ISR:  reserved bits are high. 0x%x\n",dwIntCfg&reservedBits,0);
			cnxt_gpio_int_enable(SMSC_HOST_INTERRUPT);
			return (RC_ISR_HANDLED);
	}
		
	/* get status and clear the STS reg*/
	dwIntSts=SMSC_READ(dev,INT_STS);
	dwIntEn=SMSC_READ(dev,INT_EN);

#ifdef	OLD_BOARD
	smsc_nop(dev);
	smsc_nop(dev);
	
	if (dwIntEn & (~(INT_EN_RSFL_EN_ |INT_EN_TDFA_EN_)))
	{
		dwIntEn &= (INT_EN_RSFL_EN_ |INT_EN_TDFA_EN_);
		SMSC_WRITE(dev,INT_EN,dwIntEn);
	}	
#endif

	SMSC_WRITE(dev,INT_STS,dwIntSts);

	smsc_phy_ISR(dev,&dwIntSts,dwIntEn);

	smsc_Tx_ISR(dev,&dwIntSts,dwIntEn);

	smsc_Rx_ISR(dev,&dwIntSts,dwIntEn);

	smsc_other_ISR(dev,&dwIntSts,dwIntEn);

	if(dwIntSts) {
		//SMSC_TRACE("unserviced interrupt dwIntCfg=0x%08x,dwIntSts=0x%08x,INT_EN=0x%08x",dwIntCfg,dwIntSts,SMSC_READ(dev,INT_EN));
	}

#ifdef	OLD_BOARD
	if (0==(SMSC_READ(dev,AFC_CFG)&0xFF0000))
	{					
		SMSC_ISR_TRACE("smsc_ISR: AFC_CFG (%d->%d) was changed, Fixed it !\n",SMSC_AFC_HI,SMSC_READ(dev,AFC_CFG));

		SMSC_WRITE(dev,AFC_CFG,
						(SMSC_READ(dev,AFC_CFG)&0xFF)|
						(SMSC_AFC_HI<<16) |
						( SMSC_AFC_LO<<8));
	}
#endif

	smsc_set_bits(dev,IRQ_CFG,IRQ_CFG_IRQ_EN_);

	return (RC_ISR_HANDLED);
}

#if (SMSC_TRANSFER_MODE==SMSC_DMA)
static ETH_STATUS smsc_DMA_init()
{
	PFNISR	pFnChain;

#if DMXDMA==YES
	pFnChain=&gDemuxDriverInfo.dma_input_intr_chain;
#else
	pFnChain=NULL;
#endif

	if(int_register_isr(INT_DMA,
	                   (PFNISR)smsc_DMA_ISR,
	                   FALSE,
	                   FALSE,
	                   &pFnChain) != RC_OK)
	{
	   SMSC_TRACE("smsc_dma_init: Failed to hook SMSC DMA ISR!\n");
	   return (ETH_ERROR);
	}

	/*DMA Mode*/
	*(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG & ~(DMA_BUFFERED|DMA_MULTIPLE | DMA_BUS_LOCK) ;
	*(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG | DMA_BUFFERED;
	
	if (int_enable(INT_DMA) != RC_OK)
	{
	   SMSC_TRACE("smsc_dma_init: Failed to enable DMA INT!\n");
	   return (ETH_ERROR);
	}

	return ETH_SUCCESS;
}
#endif


#if (SMSC_TRANSFER_MODE==SMSC_DMA)
/********************************************************************
    FUNCTION:    smsc_DMA_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  the Interrupt service routine .
                  
    RETURNS:			
                  RC_ISR_HANDLED
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static int32 smsc_DMA_ISR(u_int32 irq, bool FIQ, PFNISR *shared)
{	
	u_int32 dwInt,ch;
	smsc_priv_t *priv;
	u_int32 status;
	
	struct _DV_DEVICE_ENTRY *dev=smsc_dev;
	SMSC_ASSERT(dev!=NULL);

	priv=(smsc_priv_t*)dev->user_defined_1;	

#if DMXDMA==YES
	shared=&gDemuxDriverInfo.dma_input_intr_chain;
#else
	shared=NULL;
#endif
	
	dwInt = *(LPREG)DMA_INT_REG ;
	*(LPREG)DMA_INT_REG &= ~dwInt ;

	if ( dwInt & (1<<priv->dma_rx_channel) )
	{
		ch=priv->dma_rx_channel;
		status = *((volatile u_int32 *)DMA_STATUS_REG_CH(ch));

		if (status & DMA_DST_FULL)
		{
			SMSC_UNLOCK(priv->dma_rx_sem);
		}	

		if (status & (DMA_READ_ERROR|DMA_WRITE_ERROR))
		{
			SMSC_WARNING("smsc_DMA_ISR: RX DMA Fatal Error \n");
		}
			
		*((volatile u_int32 *)DMA_STATUS_REG_CH(ch)) = 0x7F;
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) = 0;		
		
	}
	
	if  ( dwInt & (1<<priv->dma_tx_channel) )
	{
		ch=priv->dma_tx_channel;
		status = *((volatile u_int32 *)DMA_STATUS_REG_CH(ch));

		if (status & DMA_SRC_EMPTY)
		{
			SMSC_UNLOCK(priv->dma_tx_sem);
		}	

		if (status & (DMA_READ_ERROR|DMA_WRITE_ERROR))
		{
			SMSC_WARNING("smsc_DMA_ISR: TX DMA Fatal Error \n");
		}
			
		*((volatile u_int32 *)DMA_STATUS_REG_CH(ch)) = 0x7F;
		*((volatile u_int32 *)DMA_CONTROL_REG_CH(ch)) &= 0;	
	}	

	return (RC_ISR_HANDLED);
}
#endif

/********************************************************************
    FUNCTION:    smsc_Tx_pause_xmit
    PARAMETERS:  
                                 
    DESCRIPTION: 
                  Pause the xmit proccess.
                  
    RETURNS:     
                  N/A
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static STATUS smsc_Tx_pause_xmit(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwMsg[4];
	u_int32 fifo_int;
	smsc_priv_t *priv;
	STATUS result;

	priv=(smsc_priv_t *)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	fifo_int=SMSC_READ(dev,FIFO_INT)&(0x00FFFFFF);
	fifo_int|=(0x20<<24);
	smsc_nop(dev);
	smsc_nop(dev);
	SMSC_WRITE(dev,FIFO_INT,fifo_int);

	smsc_set_bits(dev,INT_EN,INT_EN_TDFA_EN_);	
	result=qu_receive(priv->msg_id,2000,(void*)dwMsg);
	return result;
	
}

/********************************************************************
    FUNCTION:    smsc_Tx_resume_xmit
    PARAMETERS:  
                                 
    DESCRIPTION: 
                 resume the xmit proccess.
                  
    RETURNS:     
                  N/A
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_Tx_resume_xmit(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwMsg[4];
	smsc_priv_t *priv;

	priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	smsc_clear_bits(dev,INT_EN,INT_EN_TDFA_EN_);	
	smsc_set_bits(dev,FIFO_INT,0xFF000000);
	smsc_set_bits(dev,INT_STS,INT_STS_TDFA_);
	
	/*resume the xmit proccess*/
	dwMsg[0]=TX_MSG_TDFA;
	if (RC_OK!=qu_send(priv->msg_id,(void*)dwMsg))
		SMSC_ISR_TRACE("smsc_Tx_resume_xmit: qu_send Failled  INT_EN=0x%x INT_STS=0x%x!!!\n",
								SMSC_READ(dev,INT_EN),
								SMSC_READ(dev,INT_STS));	
}


/********************************************************************
    FUNCTION:    smsc_Tx_hard_xmit
    PARAMETERS:  
                  buf : the Nucleus' structure that contain the data to be sent.
               
    DESCRIPTION: 
                  This is the hard transmit function calling by Necleus Net to send data 
                  into ethernet.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_Tx_hard_xmit(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf )
{
	smsc_priv_t *priv;
	u_int32 free_space;
	
	SMSC_ASSERT(dev!=NULL);

	priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	if (buf==NULL)
		return ETH_BAD_PARAMETER;

	free_space=SMSC_READ(dev,TX_FIFO_INF);
	
	free_space&=TX_FIFO_INF_TDFREE_;

	if(free_space<TX_FIFO_LOW_THRESHOLD) 
	{		
		priv->tx_congested=TRUE;	
		if (RC_OK!=smsc_Tx_pause_xmit(dev))
		{
			priv->stats.tx_serious_error++;
			priv->stats.tx_discarded++;
			priv->stats.tx_errors++;

			/*
				the executing of the following sentence maybe means a hardware exception occur.
				the data in Tx fifo simply can not be sent to the line any more except for a reset 
				operation.
		 	*/
			if (priv->stats.tx_serious_error>5000)
			{	
				SMSC_TRACE("Tx Serious Error!!!! Reset !\n");
				if (RC_OK!=event_send(priv->smsc_task_id,priv->event_reset))
				{	
					SMSC_TRACE("Task: event_send error!!!\n");
				}
				priv->stats.tx_serious_error=0;						
			}
			
			return ETH_NOFIFO;
		}		
	
	}			
	
	SMSC_LOCK(priv->ResetLock,KAL_WAIT_FOREVER);
	free_space-=smsc_Tx_copy_frame(dev,buf);
	SMSC_UNLOCK(priv->ResetLock);
	
	priv->tx_congested=FALSE;

	return ETH_SUCCESS;
}

/********************************************************************
    FUNCTION:    smsc_Tx_init
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Initialize the relevant register for TX service.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_Tx_init(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwVal=0;
			
	/* Store-Forward Mode*/
	smsc_set_bits(dev,HW_CFG,HW_CFG_SF_);
	
	dwVal=SMSC_READ(dev,FIFO_INT) & 0x0000FFFF;
	SMSC_WRITE(dev,FIFO_INT,dwVal |SMSC_TDFA_LEVEL | SMSC_TSFL_LEVEL);
	
	//Because this is part of the single threaded initialization
	//  path there is no need to acquire the MacPhyAccessLock
	{
		dwVal=smsc_mac_read_reg(dev,MAC_CR);
		dwVal|=(MAC_CR_TXEN_|MAC_CR_HBDIS_);
		smsc_mac_write_reg(dev,MAC_CR,dwVal);
		SMSC_WRITE(dev,TX_CFG,TX_CFG_TX_ON_|TX_CFG_TXSAO_);
	}

	return ETH_SUCCESS;
}

/********************************************************************
    FUNCTION:    smsc_Tx_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Deal with the Tx Interrupts.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_Tx_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn)
{
	
	u_int32 dwTxSts;
	//smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);

	dwTxSts= *dwStatus &
		      (INT_STS_TDFA_
		      |INT_STS_TDFO_
		      |INT_STS_TDFU_
		      |INT_STS_TSFF_
		      |INT_STS_TSFL_
		      |INT_STS_TXE_
		      |INT_STS_TXSO_
		      |INT_STS_TXSTOP_INT_
		      );
		
	if (dwTxSts & dwIntEn)
	{		
		if (dwTxSts & INT_STS_TXE_ & dwIntEn)
		{
			SMSC_ISR_TRACE("Tx_ISR : INT_STS_TXE 0x%x,INT_EN=0x%x\n",
										dwTxSts,
										SMSC_READ(dev,INT_EN));
			smsc_set_bits(dev,INT_STS,INT_STS_TXE_);
			*dwStatus &=~(INT_STS_TXE_);
		}

		if (dwTxSts & INT_STS_TDFA_ & dwIntEn) /*fifo buffer available*/	 	
		{         
			smsc_Tx_resume_xmit(dev);
			*dwStatus &=~(INT_STS_TDFA_);
		}

		if (dwTxSts & INT_STS_TDFO_ & dwIntEn) /*data fifo overrun*/
		{
			SMSC_ISR_TRACE("Tx_ISR : INT_STS_TDFO 0x%x!!!!\n",dwTxSts,0);
			smsc_set_bits(dev,INT_STS,INT_STS_TDFO_);
			*dwStatus &=~(INT_STS_TDFO_);
		}
		
		if (dwTxSts & INT_STS_TDFU_ & dwIntEn) /*data fifo underrun*/
		{
			SMSC_ISR_TRACE("Tx_ISR : INT_STS_TDFU 0x%x!!!!\n",dwTxSts,0);
			smsc_set_bits(dev,INT_STS,INT_STS_TDFU_);
			*dwStatus &=~(INT_STS_TDFU_);
		}		
	}
	return ETH_SUCCESS;
}


/********************************************************************
    FUNCTION:    smsc_Tx_copy_frame
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  transfer data from Nucleus Net protocol stack to the Ethernet device fifo.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static int32 smsc_Tx_copy_frame(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf)
{
	smsc_priv_t *priv;

	u_int32 tx_cmd_A=0;
	u_int32 tx_cmd_B=0;

	NET_BUFFER *work_buf=buf;
	u_int32 segment_buf;
	u_int32 frame_len=0;   
	u_int16 segment_len;
	u_int16 first_segment=0;
	u_int16 last_segment=0;
	u_int16 segments=0;
	u_int32 len=0;
	u_int32 i;
	u_int32 status_used_space;   

	priv=(smsc_priv_t *)dev->user_defined_1;
	SMSC_ASSERT(dev!=NULL);   

	/*how mang segments in current buffer chain*/
	while (work_buf)
	{
		segments++;
		frame_len+=work_buf->data_len;
		work_buf=work_buf->next_buffer;
	}

	work_buf=buf;
	
	for (i=0;i<segments;i++)
	{       			  
		segment_len=work_buf->data_len;
		segment_buf=(u_int32)work_buf->data_ptr;
		segment_buf=(segment_buf & 0xFFFFFFFC);
		tx_cmd_A=tx_cmd_B=0;

		first_segment=(i==0)? 1:0;
		last_segment=(i==segments-1)?1:0;

		tx_cmd_A=((segment_buf&0x03UL)<<16) | // data start offset
		(first_segment<<13)| 
		(last_segment<<12) |
		segment_len;

		smsc_write_fifo(dev, &tx_cmd_A, TX_DATA_FIFO, 1);       	

		tx_cmd_B= (frame_len << 16 | frame_len);
		smsc_write_fifo(dev,&tx_cmd_B,TX_DATA_FIFO,1);
		
		len=((segment_len+(segment_buf & 0x3)+(BURST_SIZE-1)) & (~(BURST_SIZE-1))) >>2 ;
		smsc_write_fifo(dev,(u_int32*)segment_buf,
							TX_DATA_FIFO,
							len);
		
		work_buf=work_buf->next_buffer ;	

	}


	/* After data was transfered to device's buffer
		free the uplayer buffer.
	*/	
	DEV_Recover_TX_Buffers (dev);
	priv->stats.tx_packets++;

	status_used_space=smsc_Tx_status_count(dev);
	if ( status_used_space > 10 )
	{
		smsc_Tx_update_stats(dev);
	}
		
	return (frame_len+segments*8+3)&(~3);

}


/********************************************************************
    FUNCTION:    smsc_Tx_status_count
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  get current TX status number in words.
                  
    RETURNS:     
                  the number of status words(32-bit) in the TX Status FIFO.
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static u_int32 smsc_Tx_status_count(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwVal=0;
	SMSC_ASSERT(dev!=NULL);

	smsc_nop(dev);
	smsc_nop(dev);
	smsc_nop(dev);
	dwVal=SMSC_READ(dev,TX_FIFO_INF);
	dwVal&=TX_FIFO_INF_TSUSED_;
	dwVal>>=16;
	return dwVal;
}

/********************************************************************
    FUNCTION:    smsc_Tx_update_stats
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Update current Tx stats.
                  
    RETURNS:     
                  N/A
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_Tx_update_stats(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 tx_status=0;
	smsc_priv_t *priv=NULL;
	u_int32 dwCnt;
	
	priv=(smsc_priv_t *)dev->user_defined_1;
	SMSC_ASSERT(priv!=NULL);
	
	dwCnt=(SMSC_READ(dev,TX_FIFO_INF) & 0x00FF0000)>>16;

	while(dwCnt--)
	{
		smsc_read_fifo(dev,&tx_status,TX_STATUS_FIFO_PORT,1);	
		
		if(tx_status&0x80000000UL) {
			SMSC_TRACE("Packet tag reserved bit is high");
			//In this driver the packet tag is used as the packet
			//  length. Since a packet length can never reach
			//  the size of 0x8000, I made this bit reserved
			//  so if I ever decided to use packet tracking 
			//  tags then those tracking tags would set the 
			//  reserved bit. And I would use this control path
			//  to look up the packet and perhaps free it.
			//  As you can see I never persued this idea.
			//  because it never provided any benefit in this
			//  linux environment.
			//  But it is worth noting that the "reserved bit"
			//  in the warning above does not reference a
			//  hardware defined reserved bit but rather a 
			//  driver defined reserved bit. 
		} else {
			if( (tx_status & ~(0xFFFF0000|TX_STS_ES_|TX_STS_NOC_|TX_STS_DF_|TX_STS_CCOUNT_)) ) {
				SMSC_TRACE("Tx %d ERROR 0x%x!\n",priv->stats.tx_packets,tx_status);
				priv->stats.tx_errors++;
			} else {
				//SMSC_TRACE("Tx %d Success send %d bytes!\n",priv->stats.tx_packets,((tx_status>>16) & 0xFFFF));
				priv->stats.tx_bytes+= ((tx_status>>16) & 0xFFFF);
			}
			if(tx_status & TX_STS_EC_) {
				priv->stats.collisions+=16;
				priv->stats.tx_aborted_errors+=1;
			} else {
				priv->stats.collisions+=
				((tx_status>>3)&0xFUL);
			}
			if(tx_status& TX_STS_LOC_) {
				priv->stats.tx_carrier_errors++;
			}
			if(tx_status & TX_STS_LC_) {
				priv->stats.collisions++;
				priv->stats.tx_aborted_errors++;
			}
		}
		
	}
}

/********************************************************************
    FUNCTION:    smsc_Tx_FIFO_dump
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Dump the Tx data and status fifo and reset the fifo pointer.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
/*static ETH_STATUS smsc_Tx_FIFO_dump(struct _DV_DEVICE_ENTRY *dev)
{
	int32 timeout=1000;
	ETH_STATUS result=ETH_SUCCESS;
	bool bOldStatus;
	smsc_priv_t *priv;
	
	priv=(smsc_priv_t*)(dev->user_defined_1);

	SMSC_ENTER_CRITICAL_REGION(bOldStatus);
#if 1
	smsc_set_bits(dev,TX_CFG,TX_CFG_STOP_TX_);
	smsc_nop(dev);
	smsc_nop(dev);
	while ( (SMSC_READ(dev,TX_CFG)&TX_CFG_STOP_TX_) && timeout--);
	if (timeout<=0)
	{
		result=ETH_ERROR;
		goto DONE;
	}
#endif	
	timeout=1000;
	smsc_set_bits(dev,TX_CFG,TX_CFG_TXD_DUMP_|TX_CFG_TXS_DUMP_);
	smsc_nop(dev);
	smsc_nop(dev);
	while ( (SMSC_READ(dev,TX_CFG)&(TX_CFG_TXD_DUMP_|TX_CFG_TXS_DUMP_))
		 	&& timeout--);
	if (timeout<=0)
		result= ETH_ERROR;
	
DONE:

	smsc_set_bits(dev,TX_CFG,TX_CFG_TX_ON_);
	
	SMSC_LEAVE_CRITICAL_REGION(bOldStatus);

	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);
	smsc_mac_write_reg(dev,MAC_CR, smsc_mac_read_reg(dev,MAC_CR)|MAC_CR_TXEN_|MAC_CR_HBDIS_);
	SMSC_UNLOCK(priv->MacPhyAccessLock);

		
	return result;
}*/

/********************************************************************
    FUNCTION:    smsc_Rx_init
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Initialize the relevant register for RX service.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_Rx_init(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwVal=0;	
	
	//  Because this is part of the single threaded initialization
	//  path there is no need to acquire the MacPhyAccessLock
	dwVal=smsc_mac_read_reg(dev,MAC_CR);
	dwVal|=MAC_CR_RXEN_|MAC_CR_PRMS_;//HPFILT_;
	smsc_mac_write_reg(dev,MAC_CR,dwVal);

	dwVal=SMSC_READ(dev,FIFO_INT)&0xFFFF0000;
	SMSC_WRITE(dev, FIFO_INT, dwVal |(SMSC_RSFL_LEVEL | SMSC_RDFL_LEVEL));

	return ETH_SUCCESS;

}


/********************************************************************
    FUNCTION:    smsc_Rx_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Deal with the Rx Interrupts.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_Rx_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn)
{
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	u_int32 dwRxSts;

	SMSC_ASSERT(priv!=NULL);

#if 1
	dwRxSts= *dwStatus &
		      (INT_STS_RDFL_
		      |INT_STS_RSFF_
		      |INT_STS_RSFL_
		      |INT_STS_RWT_
		      |INT_STS_RXDFH_INT_
		      |INT_STS_RXDF_
		      |INT_STS_RXD_INT_
		      |INT_STS_RXE_
		      |INT_STS_RXSTOP_INT_
		      );
	
#else
	dwRxSts=*dwStatus & INT_STS_RSFL_;
#endif
	
	if  (dwRxSts & INT_STS_RSFL_ & dwIntEn ) /* some packets come */
	{
		priv->rx_isr_count++;
		*dwStatus&=(~INT_STS_RSFL_);
		dwRxSts&=~(INT_STS_RSFL_);
			
		/* Disable the RSFL_INT and use polling in the smsc task*/
		smsc_clear_bits(dev,INT_EN,INT_EN_RSFL_EN_);		
		 
		
		if (RC_OK!=event_send(priv->smsc_task_id,priv->event_rsfl))
		{	
			SMSC_ISR_TRACE("RX_ISR: event_send error!!!\n",0,0);
		}
	}

	if (dwRxSts & INT_STS_RXE_)
	{
		SMSC_ISR_TRACE("Rx_ISR: RX Error !!!\n",0,0);
		*dwStatus&=(~INT_STS_RXE_);
		dwRxSts&=~(INT_STS_RXE_);
		//if (RC_OK!=event_send(priv->smsc_task_id,priv->event_reset))
		//{	
		//	SMSC_ISR_TRACE("RX_ISR: event_send error!!!\n",0,0);
		//}	
	}

	/* Rx DMA Finished */
	if (dwRxSts & INT_STS_RXD_INT_ & dwIntEn)
	{
		//SMSC_ISR_TRACE("Rx_ISR: RX DMA Finished !!!\n",0,0);
		*dwStatus&=(~INT_STS_RXD_INT_);
		dwRxSts&=~(INT_STS_RXD_INT_);			
		smsc_clear_bits(dev,INT_EN,INT_EN_RXD_INT_EN_);	
		SMSC_UNLOCK(priv->dma_rx_sem);
	}

	if (dwRxSts & dwIntEn)
	{
		SMSC_ISR_TRACE("Rx_ISR: Other INT 0x%x!!!!\n",dwRxSts,0);	
		SMSC_ISR_TRACE("INT_EN=0x%x INT_STS=0x%x!!!!\n",SMSC_READ(dev,INT_EN),SMSC_READ(dev,INT_STS));	
		smsc_set_bits(dev,INT_STS,dwRxSts);
		*dwStatus &=~dwRxSts;			
	}
		
	return ETH_SUCCESS;
}


/********************************************************************
    FUNCTION:    smsc_Rx_process_status
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  read status words from RX status FIFO and process them .
                  
    RETURNS:     
                  ETH_SUCCESS: get a valid Rx status word.
                  !ETH_SUCCESS: 
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_Rx_process_status(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwFrameLen)
{
	u_int32 dwRxStatus=0;
	u_int32 dwTmp;	 
	ETH_STATUS result=!ETH_SUCCESS;
	u_int32 dwRxStatusSize=0;
	
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);

	dwRxStatusSize=(SMSC_READ(dev,RX_FIFO_INF)>>16)&0xFF;
	
	/* read status fifo until get a successful status or the status fifo is empty. */
	if (dwRxStatusSize)
	{
		priv->rx_task_count++;
		smsc_read_fifo(dev,&dwRxStatus,RX_STATUS_FIFO_PORT,1);
		while((dwRxStatus & RX_STS_ES_) || (dwRxStatus & RX_STS_MII_ERR)){

			if (dwRxStatus & RX_STS_MII_ERR)	
			{
				SMSC_TRACE("rx error!\n");
				event_send(priv->smsc_task_id,priv->event_reset);
			}	

			priv->stats.rx_errors++;	
			if (dwRxStatus & RX_STS_RUNT_FRM_)
			{
				SMSC_TRACE("smsc_Rx_process_status: Runt Frame!\n");
			}

			if (dwRxStatus & RX_STS_FRM_TOO_LONG)
			{
				SMSC_TRACE("smsc_Rx_process_status: Frame too long! \n");
				priv->stats.rx_length_errors++;
			}

			if (dwRxStatus & RX_STS_COLLISION)
			{
				SMSC_TRACE("smsc_Rx_process_status: Collision occur!\n");
				priv->stats.collisions++;
			}

			if (dwRxStatus & RX_STS_CRC_ERR)
			{
				SMSC_TRACE("smsc_Rx_process_status: CRC error!\n");
				priv->stats.rx_crc_errors++;
			}
			
			*dwFrameLen=(dwRxStatus&0x3FFF0000)>>16;
			dwTmp+=*dwFrameLen;//???
			dwTmp=(dwTmp+3)>>2;//word count

			result=smsc_Rx_fast_forward(dev,dwTmp); 
			if (result!=ETH_SUCCESS)
				return ETH_ERROR;
			dwRxStatusSize=(SMSC_READ(dev,RX_FIFO_INF)>>16)&0xFF;
			
			if (dwRxStatusSize>0)
				smsc_read_fifo(dev,&dwRxStatus,RX_STATUS_FIFO_PORT,1);	
			else
			{
				dwRxStatus=0;
				break;
			}	

		};
	}
	else
	{
		dwRxStatus=0;
	}	
	
	if (dwRxStatus==0) /* The RX FIFO empty */
	{
		//SMSC_TRACE("smsc_Rx_process_status: dwRxStatus==0, All packet recv is invalid!\n");
		result=ETH_ERROR;
	}
	else
	{
		priv->stats.rx_packets++;
		
		*dwFrameLen=(dwRxStatus&0x3FFF0000)>>16;	 
		
		if (dwRxStatus & RX_STS_MLT_FRM_)
		{
			priv->stats.rx_multicast++;		
		}
		
		priv->stats.rx_bytes+=*dwFrameLen;
		result=ETH_SUCCESS;
	}

	return result;
}


/********************************************************************
    FUNCTION:    smsc_Rx_fast_forward
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Fast discard a packet in the RX data FIFO.
                  
    RETURNS:     
                  ETH_SUCCESS: 
                  !ETH_SUCCESS: 
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_Rx_fast_forward(struct _DV_DEVICE_ENTRY *dev,u_int32 nDwords)
{
	u_int32 dwTimeOut=100000;	
	u_int32 dwTmp;	  
	SMSC_TRACE("smsc_RX_fast_forward, nDwords=%d!!!\n",nDwords);
	
	if(nDwords>=4) 
	{
		SMSC_WRITE(dev,RX_DP_CTRL,RX_DP_CTRL_RX_FFWD_);
		while((dwTimeOut)&&(SMSC_READ(dev,RX_DP_CTRL)&RX_DP_CTRL_RX_FFWD_))
		{
			dwTimeOut--;
		}
		
		if(dwTimeOut==0) 
		{
			SMSC_TRACE("timed out waiting for RX FFWD to finish, RX_DP_CTRL=0x%08lX\n",
				SMSC_READ(dev,RX_DP_CTRL));
			return ETH_ERROR;
		}
	} 
	else 
	{
		while(nDwords) {
			smsc_read_fifo(dev,&dwTmp,RX_DATA_FIFO,1);
			nDwords--;
		}
	}	

		

	return ETH_SUCCESS;
}

/********************************************************************
    FUNCTION:    smsc_Rx_transfer_frame
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Transfer data from the RX data FIFO into the Nucleus Net buffer.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static STATUS smsc_Rx_transfer_frame(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwFrameLen;
	u_int32 bytes_left;
	u_int32 len;
	NET_BUFFER *work_buf,*buf_ptr;
	u_int32 *dest=NULL;		
	u_int32 i;

	
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);
	
	//pt=(u_int32*)mem_nc_malloc(1024);

	for (i=0;i<SMSC_RX_POLLING_TIMES;i++)
	{
		while (smsc_Rx_process_status(dev,&dwFrameLen)==ETH_SUCCESS)
		{		

            
			work_buf = buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, dwFrameLen );
			bytes_left=dwFrameLen;

			if ( buf_ptr != NU_NULL )
			{
				priv->rx_congested=FALSE;
				buf_ptr->mem_total_data_len = bytes_left;
				buf_ptr->mem_buf_device = dev;
				buf_ptr->mem_flags = 0;

				/*Parent Packets*/				
				len=(bytes_left>NET_PARENT_BUFFER_SIZE)?NET_PARENT_BUFFER_SIZE:bytes_left;

				//SMSC_TRACE("\t3\tREAD START\n");				
				do
				{
					work_buf->data_len = len; 
					work_buf->data_ptr = (u_int8*)(work_buf->mem_parent_packet); /* point to data storage */
					bytes_left-=len;

					dest = (u_int32*)( (u_int32)(work_buf->data_ptr) & 0xFFFFFFFC); 	
					
					len=(len+BURST_SIZE-1) & (~(BURST_SIZE-1));
					smsc_read_fifo(dev,dest,RX_DATA_FIFO,len>>2);
										
					len=(bytes_left>NET_MAX_BUFFER_SIZE)?NET_MAX_BUFFER_SIZE:bytes_left;
					work_buf = work_buf->next_buffer ;									
					
				}while( (work_buf != NU_NULL) && bytes_left );

                     
				MEM_Buffer_Enqueue (&MEM_Buffer_List, buf_ptr );	
				NU_Set_Events (&Buffers_Available, 2, NU_OR);	

			}
			else /* No buffers were available */
			{
				/* Discard the received frame */		
				priv->rx_congested=TRUE;
				len=(dwFrameLen+3)>>2;		
				smsc_Rx_fast_forward(dev,len); 
				priv->stats.rx_discarded++;
				priv->stats.rx_errors++;
				priv->stats.rx_serious_error++;
				return NU_NO_MEMORY ;
			}
				
		}

	}

	return NU_SUCCESS;

}


/********************************************************************
    FUNCTION:    smsc_task
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  A task to wait RSFL msg and then start the RX transfer service
                  or the SMSC_RESET msg to reset chip .
                  
    RETURNS:     
                  N/A
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static void smsc_task(struct _DV_DEVICE_ENTRY *dev) 
{
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	STATUS result;
	events_t event_received;
	INT32 rtn;
	u_int32 dwFIFOInt;

	SMSC_ASSERT(priv!=NULL);

	priv->event_rsfl=event_create();
	SMSC_ASSERT(priv->event_rsfl!=0);

	priv->event_reset=event_create();
	SMSC_ASSERT(priv->event_reset!=0);

	
	while (1)
	{		
		event_received=0;
		rtn=event_receive(priv->event_reset|priv->event_rsfl,
							&event_received, 6000, FALSE);
		//SMSC_TRACE("\t2\tGET EVENT \n" );

		if ( (event_received & priv->event_rsfl) ||
			rtn!=RC_OK)
		{		
		
				result=smsc_Rx_transfer_frame(dev);

				dwFIFOInt=SMSC_READ(dev,FIFO_INT);

				priv->stats.rx_dropped+=SMSC_READ(dev,RX_DROP);
				
				/* some execptions occur*/
				if (result!=NU_SUCCESS)
				{
					SMSC_TRACE("RX No Buffer Available!\n" );
				#if 0								
					if (priv->stats.rx_serious_error>5000000)
					{
						SMSC_TRACE("Rx FIFO Dump and Buffer CleanUP !!\n");
						priv->stats.rx_serious_error=0;					
						smsc_Rx_pause(dev);		
						smsc_Rx_FIFO_dump(dev);	
						MEM_Buffer_Cleanup(&MEM_Buffer_List);							
						smsc_Rx_resume(dev);		
					}
				#endif	
				}
				
				cnxt_gpio_int_disable(SMSC_HOST_INTERRUPT);
				/* clear RSFL status */		
				smsc_set_bits(dev,INT_STS,INT_STS_RSFL_);
				/* Resume RXFL_INT*/				
				smsc_set_bits(dev,INT_EN,INT_EN_RSFL_EN_);
				cnxt_gpio_int_enable(SMSC_HOST_INTERRUPT);

				if (rtn == RC_KAL_TIMEOUT)
					event_received=0;
									
			}
				
			if (event_received & priv->event_reset)
			{
				cnxt_gpio_int_disable(SMSC_HOST_INTERRUPT);
				smsc_fast_recover(dev);
				cnxt_gpio_int_enable(SMSC_HOST_INTERRUPT);		
				if (RC_OK!=event_send(priv->smsc_task_id,priv->event_rsfl))
				{	
					SMSC_TRACE("Task: 2 event_send error!!!\n");
				}		
			}

			
		}		
}


/********************************************************************
    FUNCTION:    smsc_Rx_pause
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Stop receiving the data.
                  
    RETURNS:     
    			  N/A.	
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
/*static void smsc_Rx_pause(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwTmp;
	dwTmp=smsc_mac_read_reg(dev,MAC_CR);
	dwTmp&=(~MAC_CR_RXEN_);
	smsc_mac_write_reg(dev,MAC_CR,dwTmp);	
}*/

/********************************************************************
    FUNCTION:    smsc_Rx_resume
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  resume receiving the data.
                  
    RETURNS:     
    			  N/A.	
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
/*static void smsc_Rx_resume(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 dwTmp;
	dwTmp=smsc_mac_read_reg(dev,MAC_CR);
	dwTmp|=MAC_CR_RXEN_;
	smsc_mac_write_reg(dev,MAC_CR,dwTmp);
}*/

/********************************************************************
    FUNCTION:    smsc_Tx_FIFO_dump
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Dump the Rx data and status fifo and reset the fifo pointer.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
/*static ETH_STATUS smsc_Rx_FIFO_dump(struct _DV_DEVICE_ENTRY *dev)
{
	int32 timeout=1000;
	ETH_STATUS result=ETH_SUCCESS;
	smsc_priv_t *priv;

	priv=(smsc_priv_t*)(dev->user_defined_1);
	
	smsc_Rx_pause(dev);
	
	smsc_set_bits(dev,RX_CFG,RX_CFG_RX_DUMP_);
	while ((SMSC_READ(dev,RX_CFG)&RX_CFG_RX_DUMP_) & timeout--);
	if (timeout<=0)
	{
		result=ETH_ERROR;
		goto DONE;
	}	
	
DONE:

	//SMSC_LEAVE_CRITICAL_REGION	(bOldStatus);
	SMSC_TRACE("smsc_Rx_FIFO_dump finish result=%d, RX_FIFO_INF=0x%x,MAC_CR=0x%x\n",
			result, 
			SMSC_READ(dev,RX_FIFO_INF),
			smsc_mac_read_reg(dev,MAC_CR));
	
#if 0	
	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);
	smsc_mac_write_reg(dev,MAC_CR, smsc_mac_read_reg(dev,MAC_CR)|MAC_CR_RXEN_);
	SMSC_UNLOCK(priv->MacPhyAccessLock);
#endif	
	return result;
}*/

/********************************************************************
    FUNCTION:    smsc_Phy_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Deal with the Phy Interrupts.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_phy_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn)
{
	u_int32 dwPhyISF;
	ETH_STATUS result;
	u_int32 dwPhySts;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);	

	dwPhySts=*dwStatus & INT_STS_PHY_INT_;
	
	if ( dwPhySts & dwIntEn)
	{
		dwPhyISF=smsc_phy_read_reg(dev,PHY_ISR);

		if (dwPhyISF & PHY_INT_SRC_LINK_DOWN_)
		{
			/*deal with the LINK DOWN*/
			result=ETH_LINKDOWN;
			SMSC_ISR_TRACE("smsc_phy_ISR: link down!!!\n",0,0);
		}
		if (dwPhyISF & PHY_INT_SRC_ANEG_COMP_)
		{
			result=ETH_SUCCESS;
		}
		if (dwPhyISF & PHY_INT_SRC_REMOTE_FAULT_)
		{
			result=ETH_ERROR;
		}
		smsc_set_bits(dev,INT_STS,INT_STS_PHY_INT_);
		*dwStatus&= ~INT_STS_PHY_INT_;
	}
	
	return ETH_SUCCESS;
}

/********************************************************************
    FUNCTION:    smsc_other_ISR
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Deal with the other Interrupts.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
static ETH_STATUS smsc_other_ISR(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwStatus,u_int32 dwIntEn)
{
	//SMSC_TRACE("smsc_other_ISR: other unserviced INT !\n");
	
	if (*dwStatus & INT_STS_SW_INT_ & dwIntEn)
	{
		smsc_clear_bits(dev,INT_EN,INT_EN_SW_INT_EN_);
		smsc_set_bits(dev,INT_STS,INT_STS_SW_INT_);

		*dwStatus &=~INT_STS_SW_INT_;
	}
	
	return ETH_SUCCESS;
}

/********************************************************************
    FUNCTION:    smsc_get_stats
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  get current Tx/Rx stats.
                  
    RETURNS:     
                  the pointer to the ETH_STATS structure.
    NOTES:    		   
                
    CONTEXT:
    
*********************************************************************/
/*static const ETH_STATS*  smsc_get_stats (struct _DV_DEVICE_ENTRY *dev)
{
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);
	SMSC_ASSERT(priv!=NULL);
	
	return (const ETH_STATS*)&priv->stats;
}*/

static u_int32 Hash(u_int8 addr[6])
{
	int i;
	u_int32 crc=0xFFFFFFFFUL;
	u_int32 poly=0xEDB88320UL;
	u_int32 result=0;
	for(i=0;i<6;i++) 
	{
		int bit;
		u_int32 data=((u_int32)addr[i]);
		for(bit=0;bit<8;bit++) 
		{
			u_int32 p = (crc^((u_int32)data))&1UL;
			crc >>= 1;
			if(p!=0) crc ^= poly;
			data >>=1;
		}
	}
	result=((crc&0x01UL)<<5)|
		((crc&0x02UL)<<3)|
		((crc&0x04UL)<<1)|
		((crc&0x08UL)>>1)|
		((crc&0x10UL)>>3)|
		((crc&0x20UL)>>5);
	return result;
}

/********************************************************************
    FUNCTION:    smsc_set_multicast
    PARAMETERS:  
                 addr:  
               
    DESCRIPTION: 
                 Set filtering for multicast address.
                  
    RETURNS:     
                 Always  NU_SUCESS:   
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/
STATUS smsc_set_multicast(struct _DV_DEVICE_ENTRY *dev)
{
	STATUS result=NU_SUCCESS;
	u_int32 dwHashH=0;
	u_int32 dwHashL=0;
	u_int32 dwMask=0x01UL;
	u_int32 dwBitNum;
	NET_MULTI *mc_list=dev->dev_ethermulti;
	u_int32 dwMacCr;
	smsc_priv_t *priv=(smsc_priv_t*)(dev->user_defined_1);

	SMSC_LOCK(priv->MacPhyAccessLock,KAL_WAIT_FOREVER);

	dwMacCr=smsc_mac_read_reg(dev,MAC_CR);
	dwMacCr&=(~MAC_CR_PRMS_);
	dwMacCr&=(~MAC_CR_MCPAS_);
	dwMacCr|=MAC_CR_HPFILT_;
	
	while (mc_list!=NULL)
	{
		dwBitNum=Hash(mc_list->nm_addr);
		dwMask<<=(dwBitNum&0x1FUL);
		if(dwBitNum&0x20UL) {
			dwHashH|=dwMask;
		} else {
			dwHashL|=dwMask;
		}
		mc_list=mc_list->nm_next;
	}				
	
	smsc_mac_write_reg(dev,HASHH,dwHashH);
	smsc_mac_write_reg(dev,HASHL,dwHashL);
	smsc_mac_write_reg(dev,MAC_CR,dwMacCr);

	SMSC_UNLOCK(priv->MacPhyAccessLock);

	return result;
}


#ifdef USE_DEBUG

/*static void smsc_print_reg(struct _DV_DEVICE_ENTRY *dev)
{
	u_int32 reg;
	
	for (reg=0x54;reg<0xB0;reg+=4)
		SMSC_ISR_TRACE("SYS REG 0x%08x : 0x%08x\n",reg,SMSC_READ(dev,reg));
	
	SMSC_ISR_TRACE("MAC REG 0x%08x :0x%08x\n",1,smsc_mac_read_reg(dev,1));
	SMSC_ISR_TRACE("MAC REG 0x%08x :0x%08x\n",8,smsc_mac_read_reg(dev,8));
	SMSC_ISR_TRACE("MAC REG 0x%08x :0x%08x\n",0xC,smsc_mac_read_reg(dev,0xC));
	
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",0,smsc_phy_read_reg(dev,0));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",1,smsc_phy_read_reg(dev,1));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",4,smsc_phy_read_reg(dev,4));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",5,smsc_phy_read_reg(dev,5));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",17,smsc_phy_read_reg(dev,17));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",18,smsc_phy_read_reg(dev,18));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",27,smsc_phy_read_reg(dev,27));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",29,smsc_phy_read_reg(dev,29));
	SMSC_ISR_TRACE("PHY REG 0x%08x :0x%08x\n",31,smsc_phy_read_reg(dev,31));

}

static void smsc_dump(u_int8 * buf, u_int32 len)
{
	u_int32 i;
	u_int8 *p=buf;
	//SMSC_TRACE("\n%08x len=%d \n",buf,len);

	for (i=0;i<len;i++)
	{
		SMSC_TRACE("%02x  ",*p);
		p++;
		if ((i+1)%16==0)
			SMSC_TRACE("\n");		
	}
	
}*/
#endif 

/********************************************************************
    FUNCTION:    smsc_wake_up
    PARAMETERS:  
                  
               
    DESCRIPTION: 
                  Wake up the device in sleeping mode.
                  
    RETURNS:     
                  ETH_SUCESS:   
                  !ETH_SUCCESS:
    NOTES:
    		   
                
    CONTEXT:
    
*********************************************************************/

STATUS smsc_wake_up(struct _DV_DEVICE_ENTRY *dev)
{
	STATUS result;
	u_int32 timeout=1000;
	u_int32 pmt;

	do{
		SMSC_WRITE(dev,BYTE_TEST,0xF);
		timeout--;
		pmt=SMSC_READ(dev,PMT_CTRL);
	}while (timeout>0 && 	(pmt & PMT_CTRL_READY_)==0);

	if (timeout==0)
		result=ETH_ERROR;
	else
		result=ETH_SUCCESS;
	
	return result;
}

/****************************************************************************
* Modifications:
* $Log:
*       
*  1    Ethernet      1.0         3/29/2005 16:00:00 PM  Sam Chen
* $
****************************************************************************/
