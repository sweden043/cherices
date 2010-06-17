/*

  dm9ks.c: Version 2.03 2005/10/17 
  
        A Davicom DM9000A/DM9010 ISA NIC fast Ethernet driver for nucleus.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.


  (C)Copyright 1997-2005 DAVICOM Semiconductor,Inc. All Rights Reserved.

	
V1.00	10/13/2004	Add new function Early transmit & IP/TCP/UDP Checksum
			offload enable & flow control is default
V1.1	12/29/2004	Add Two packet mode & modify RX function
V1.2	01/14/2005	Add Early transmit mode 
V1.3	03/02/2005	Support kernel 2.6
v1.33   06/08/2005	#define DM9KS_MDRAL		0xf4
			#define DM9KS_MDRAH		0xf5
			
V2.00 Spenser - 01/10/2005
			- Modification for PXA270 MAINSTONE.
			- Modified dmfe_tx_done().
			- Add dmfe_timeout().
V2.01	10/07/2005	Modified dmfe_timer()
			Dected network speed 10/100M
V2.02	10/12/2005	Use link change to chage db->Speed
			dmfe_open() wait for Link OK  
V2.03	11/22/2005	Power-off and Power-on PHY in dmfe_init()
			support IOL
			
V2.04	Anky - 8/23/2006 Lee FOR DG
*/



#include "string.h"

#include "..\nupnet\target.h"
#include "..\nupnet\dev.h"
#include "..\nupnet\net.h"
#include "..\nupnet\mem_defs.h"
#include "..\nupnet\externs.h"
#include "..\nupnet\netevent.h"

#include "MW.h"
#include "retcodes.h"

#include "basetype.h"
#include "kal.h"
#include "nucleus.h"
#include "eth.h"


#include "board.h"
#include "gpio.h"



/* Board/System/Debug information/definition ---------------- */

#define DM9KS_ID		0x90000A46
#define DM9010_ID		0x90100A46
/*-------register name-----------------------*/
#define DM9KS_NCR		0x00	/* Network control Reg.*/
#define DM9KS_NSR		0x01	/* Network Status Reg.*/
#define DM9KS_TCR		0x02	/* TX control Reg.*/
#define DM9KS_TSRI		0x03
#define DM9KS_TSRII		0x04
#define DM9KS_RXCR		0x05	/* RX control Reg.*/
#define DM9KS_RSR		0x06
#define DM9KS_ROCR		0x07
#define DM9KS_BPTR		0x08
#define DM9KS_FCTR		0x09
#define DM9KS_FCR		0x0a
#define DM9KS_EPCR		0x0b
#define DM9KS_EPAR		0x0c
#define DM9KS_EPDRL		0x0d
#define DM9KS_EPDRH		0x0e
#define DM9KS_WCR		0x0f
#define DM9KS_GPCR		0x1e
#define DM9KS_GPR		0x1f	/* General purpose register */
#define DM9KS_TRPAL		0x22
#define DM9KS_TRPAH		0x23
#define DM9KS_RWPAL		0x24
#define DM9KS_RWPAH		0x25
#define DM9KS_VID_L		0x28
#define DM9KS_VID_H		0x29
#define DM9KS_PID_L		0x2A
#define DM9KS_PID_H		0x2B
#define DM9KS_CHIPR		0x2C
#define DM9KS_TCR2		0x2d
#define DM9KS_OCR		0x2E
#define DM9KS_SMCR		0x2f 	/* Special Mode Control Reg.*/
#define DM9KS_ETXCSR	0x30	/* Early Transmit control/status Reg.*/
#define	DM9KS_TCCR		0x31	/* Checksum cntrol Reg. */
#define DM9KS_RCSR		0x32	/* Receive Checksum status Reg.*/
#define DM9KS_MPAR		0x33
#define DM9KS_LEDCR		0x34
#define DM9KS_BUSCR		0x38
#define DM9KS_INTCR		0x39
#define DM9KS_SCCR		0x50
#define DM9KS_RSCCR		0x51
#define DM9KS_MRCMDX	0xf0
#define DM9KS_MRCMDX1	0xf1
#define DM9KS_MRCMD		0xf2
#define DM9KS_MDRAL		0xf4
#define DM9KS_MDRAH		0xf5
#define DM9KS_MWCMDX	0xf6
#define DM9KS_MWCMD		0xf8
#define DM9KS_MWRL		0xfa
#define DM9KS_MWRH		0xfb
#define DM9KS_TXPLL		0xfc
#define DM9KS_TXPLH		0xfd
#define DM9KS_ISR		0xfe
#define DM9KS_IMR		0xff
/*---------------------------------------------*/
#define DM9KS_REG05		0x30	/* SKIP_CRC/SKIP_LONG */ 
#define DM9KS_REGFF		0xA3	/* IMR */
#define DM9KS_DISINTR	0x80

#define DM9KS_PHY		0x40	/* PHY address 0x01 */
#define DM9KS_PKT_RDY	0x01	/* Packet ready to receive */


#define DM9KS_VID_L		0x28
#define DM9KS_VID_H		0x29
#define DM9KS_PID_L		0x2A
#define DM9KS_PID_H		0x2B

#define DM9KS_RX_INTR		0x01
#define DM9KS_TX_INTR		0x02
#define DM9KS_LINK_INTR		0x20

#define DM9KS_DWORD_MODE	1
#define DM9KS_BYTE_MODE		2
#define DM9KS_WORD_MODE		0


//#define u_int8 uint8
//#define u_int16 uint16
//#define u_int32 uint32


#ifndef USE_DEBUG
#define	USE_DEBUG
#endif

#ifdef USE_DEBUG
//#define USE_WARNING
//#define USE_TRACE
//#define USE_ASSERT
#endif //USE_DEBUG

#define udelay task_time_sleep
//#define DMFE_DEBUG trace
#define DMFE_DEBUG isr_trace

#if 0
#ifdef USE_ASSERT
#define DM9KS_ASSERT(condition)		\
	do{									\
		if(!(condition)) {																\
			trace("DM9KS_ASSERTION_FAILURE: File=" __FILE__ ", Line=%d\n",__LINE__);	\
			while(1);																	\
		}	\
	}while(0);	

#define	DM9KS_TRACE trace
#define	DM9KS_ISR_TRACE(msg,arg1,arg2)	isr_trace(msg,arg1,arg2)
//#define	DM9KS_WARNING  trace
	
#else
//
#define DM9KS_ASSERT(condition)
#define	DM9KS_TRACE(args...)	 
#define	DM9KS_ISR_TRACE(msg,arg1,arg2)	
#define	DM9KS_WARNING(args...) 
//
#endif

#endif

/* Board/System/Debug information/definition ---------------- */

/* Added for PXA of MAINSTONE 
#ifdef CONFIG_ARCH_MAINSTONE
#include <asm/arch/mainstone.h>
#define DM9KS_MIN_IO		(MST_ETH_PHYS + 0x300)
#define DM9KS_MAX_IO            (MST_ETH_PHYS + 0x370)
#define DM9K_IRQ		MAINSTONE_IRQ(3)
#else
#define DM9KS_MIN_IO		0x300
#define DM9KS_MAX_IO		0x370
#define DM9K_IRQ		3
#endif


#define TRUE			1
#define FALSE			0
*/


/* Number of continuous Rx packets */
//#define CONT_RX_PKT_CNT	10 

//#define DMFE_TIMER_WUT  jiffies+(HZ*5)	/* timer wakeup time : 5 second */

/*
#if defined(DM9KS_DEBUG)
#define DMFE_DBUG(dbug_now, msg, vaule)\
if (dmfe_debug||dbug_now) printk(KERN_ERR "dmfe: %s %x\n", msg, vaule)
#else
#define DMFE_DBUG(dbug_now, msg, vaule)\
if (dbug_now) printk(KERN_ERR "dmfe: %s %x\n", msg, vaule)
#endif

#ifndef CONFIG_ARCH_MAINSTONE
#pragma pack(push, 1)
#endif
*/



//#ifndef CONFIG_ARCH_MAINSTONE
//#pragma pack(pop)
//#endif


#define	DM9KS_10MHD   	0
#define 	DM9KS_100MHD  1 
#define 	DM9KS_10MFD   	4
#define 	DM9KS_100MFD  	5 
#define 	DM9KS_AUTO    	8

//

void dm9ks_link_task(struct _DV_DEVICE_ENTRY *dev);

//

#define	DAVICOM_STACK_SIZE	0x800
#define	DAVICOM_TASK_PRIO		200


#define DM9000AE_INT_HI	1

#define	DM9KS_ISA_DESC					0x4							//ISA_DESC_Nmb
#define	DM9KS_ISA_DESC_PRM_REG		(PCI_ISA_DESC_BASE+DM9KS_ISA_DESC*4)     //30010000X + c
#define	DM9KS_ISA_DESC_EXT_REG		(PCI_ISA_DESC_BASE+0x80+DM9KS_ISA_DESC*4)//30010000X + 8c
#define	DM9KS_ISA_IO_BASE			(PCI_IO_BASE+0x100000*DM9KS_ISA_DESC)     /*31300000X*/

#define	DM9KS_WR_WAIT			(0x2<<24)//(0x5<<24)
#define	DM9KS_RD_WAIT				(0x2<<16)//(0x5<<16)

#define	DM9KS_CS_SEL			0x3

#define	DM9KS_CS_SETUP			(0x4<<17)
#define	DM9KS_CTL_SETUP		(0x4<<5)
#define	DM9KS_CS_HOLD			(0x0<<12)
#define	DM9KS_ADDR_HOLD		(0x4)	//0x01//(0x8)	//0x01

#define	DM9KS_REG_SETUP		(0x4<<28)//(0x0<<28)
#define	DM9KS_REG_ACCESS		(0x2<<24)



/* Defines for Electra GPIO */
#if (GPIO_CONFIG == GPIOM_BRAZOS)
   #define DM9KS_INT_GPIO_PORT	4	
   #define DM9KS_HOST_INTERRUPT   (DM9KS_INT_GPIO_PORT +GPIO_DEVICE_ID_INTERNAL+GPIO_PIN_IS_INPUT )
#endif   


typedef struct _RX_DESC
{
	u_int8 rxbyte;
	u_int8 status;
	u_int16 length;
}RX_DESC;

typedef union{
	u_int8 buf[4];
	RX_DESC desc;
} rx_t;




struct _DV_DEVICE_ENTRY *dm9ks_dev;
//board_info_t *dm9ks_priv;

u_int16 dm9ks_queue_pkt_len;

u_int32 dm9ks_Link_change;

tick_id_t dm9ks_polling_tick_id;

PFNISR	dm9ks_pFnChain=NULL;


u_int32 dm9ks_counts=0;

u_int32 dm9ks_reset_counter;		/* counter: RESET */ 
u_int32 dm9ks_reset_tx_timeout;		/* RESET caused by TX Timeout */ 

u_int32 dm9ks_io_addr;			/* Register I/O base address */
u_int32 dm9ks_io_data;			/* Data I/O address */
char dm9ks_tx_pkt_cnt;

u_int8 dm9ks_op_mode;			/* PHY operation mode */
u_int8 dm9ks_io_mode;			/* 0:word, 2:byte */
//u_int8 dm9ks_device_wait_reset;		/* device state */
u_int8 dm9ks_Speed;			/* current speed */

//int dm9ks_cont_rx_pkt_cnt;/* current number of continuos rx packets  */

u_int32 dm9ks_tx_packets;
u_int32 dm9ks_tx_bytes;

u_int32 dm9ks_rx_bytes;
u_int32 dm9ks_rx_packets;
u_int32 dm9ks_rx_fifo_errors;
u_int32 dm9ks_rx_crc_errors;
u_int32 dm9ks_rx_length_errors;
	//struct timer_list timer;
	//struct net_device_stats stats;
	//unsigned char srom[128];
	//spinlock_t lock;


//queue_id_t dm9ks_msg_id;


//sem_id_t	dm9ks_rx_sem;

//events_t dm9ks_event_rs;
//events_t dm9ks_event_link;
	
//sem_id_t dm9ks_MacPhyAccessLock;
//sem_id_t dm9ks_ResetLock;
//u_int16 dm9ks_PhyAddessId;   

//task_id_t   dm9ks_task_id;

//

//link task
task_id_t dm9ks_link_task_id;
events_t dm9ks_event_link;
//NU_HISR  DM_RX_HISR_CB;


/* Global variable declaration ----------------------------- */
/*int dmfe_debug = 0;*/
//struct net_device * dmfe_dev = NULL;

/* For module input parameter */
//int mode       = DM9KS_AUTO;
 int dm9ks_media_mode = DM9KS_AUTO;
//u_int8  irq        = DM9K_IRQ;
u_int32 iobase;//	= DM9KS_ISA_IO_BASE;
u_int32 iodata;//	= iobase + 0x02000;





extern u_int8 mac_addr[6];//={0x00,0x11,0x22,0x33,0x44,0x55};



/* function declaration ------------------------------------- 


#if defined(CHECKSUM)
u_int8 check_rx_ready(u_int8);
#endif
*/



STATUS dm9ks_init( struct _DV_DEVICE_ENTRY *dev );
void dm9ks_init_dm9000(struct _DV_DEVICE_ENTRY *dev);
STATUS dm9ks_open(struct _DV_DEVICE_ENTRY *dev);
STATUS dm9ks_start_xmit(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf );
STATUS dm9ks_ioctl( struct _DV_DEVICE_ENTRY *dev, int option, DV_REQ *request );
//void dmfe_task(struct _DV_DEVICE_ENTRY *dev);
void dm9ks_packet_receive(void);
int32 dm9ks_interrupt(u_int32 irq, bool FIQ, PFNISR *shared);

void dm9000_hash_table(struct _DV_DEVICE_ENTRY *dev);

void dm9ks_polling(tick_id_t hTick, void *pUserData);






//DECLARE_TASKLET(dmfe_tx_tasklet,dmfe_tx_done,0);




void dm9k_outb(u_int8 reg, u_int32 addr){

	*(volatile u_int8 *)addr = reg;
}

void dm9k_outw(u_int16 reg, u_int32 addr){ 

	*(volatile u_int16 *)addr = reg;
}



// ARK
u_int8 dm9k_inb(u_int32 addr){

	return (u_int8)(*(volatile u_int8 *)addr);
}
u_int16 dm9k_inw(u_int32 addr){

	return (u_int16)(*(volatile u_int16 *)addr);
}


/*
   Read a byte from I/O port
*/
u_int8 ior( int reg)
{
	dm9k_outb(reg, dm9ks_io_addr);
	return dm9k_inb(dm9ks_io_data);
}

/*
   Write a byte to I/O port
*/
void iow( int reg, u_int8 value)
{
	dm9k_outb(reg, dm9ks_io_addr);
	dm9k_outb(value, dm9ks_io_data);
}

/*
   Read a word from phyxcer
*/
u_int16 phy_read(int reg)
{
	/* Fill the phyxcer register into REG_0C */
	iow( DM9KS_EPAR, DM9KS_PHY | reg);

	iow( DM9KS_EPCR, 0xc); 	/* Issue phyxcer read command */
	udelay(100);			/* Wait read complete */
	iow( DM9KS_EPCR, 0x0); 	/* Clear phyxcer read command */

	/* The read data keeps on REG_0D & REG_0E */
	return ( ior( DM9KS_EPDRH) << 8 ) | ior( DM9KS_EPDRL);
	
}
/*
   Write a word to phyxcer
*/
void phy_write( int reg, u_int16 value)
{
	/* Fill the phyxcer register into REG_0C */
	iow( DM9KS_EPAR, DM9KS_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	iow( DM9KS_EPDRL, (value & 0xff));
	iow( DM9KS_EPDRH, ( (value >> 8) & 0xff));

	iow( DM9KS_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);			/* Wait write complete */
	iow( DM9KS_EPCR, 0x0);	/* Clear phyxcer write command */
}



/* DM9000 network baord routine ---------------------------- */

/*
  Search DM9000 board, allocate space and register it
*/

STATUS dm9ks_init( struct _DV_DEVICE_ENTRY *dev )
{
//	DV_REQ	req;
	u_int32 id_val=0,id_val1=0;
	volatile u_int32 *dwAddr=0;

	iobase = DM9KS_ISA_IO_BASE;
	iodata = iobase + 0x02000;	
	
		/*Initialize the ISA Desc*/
		dwAddr=(volatile u_int32 *)DM9KS_ISA_DESC_PRM_REG;
/*		*dwAddr = 0x0d0e1020;*/

			*dwAddr=(PCI_ISA_DESC_TYPE_ISA|
			  	DM9KS_WR_WAIT |
			  	DM9KS_RD_WAIT |
			  	PCI_ROM_DESC_BURST_ENABLE |
                     PCI_ISA_DESC_NO_EXT_WAIT_STATES |
                     PCI_ISA_DESC_IO_TYPE_IORD_IOWR |
                     ( DM9KS_CS_SEL << PCI_ISA_DESC_CS_SHIFT)|
                     PCI_ROM_DESC_WIDTH_16  
				);

		
		dwAddr=(volatile u_int32 *)DM9KS_ISA_DESC_EXT_REG;
/*		*dwAddr = 0x00000000;*/
		*dwAddr=(DM9KS_REG_SETUP|
				DM9KS_REG_ACCESS|
				DM9KS_CS_SETUP |
				DM9KS_CS_HOLD |
				DM9KS_CTL_SETUP|
				DM9KS_ADDR_HOLD);

		/* end config ISA*/


	dm9k_outb(DM9KS_VID_L, iobase);
	id_val = dm9k_inb(iodata);
	dm9k_outb(DM9KS_VID_H, iobase);
	id_val |= dm9k_inb(iodata) << 8;
	dm9k_outb(DM9KS_PID_L, iobase);
	id_val |= dm9k_inb(iodata) << 16;
	dm9k_outb(DM9KS_PID_H, iobase);
	id_val |= dm9k_inb(iodata) << 24;
	dm9k_outb(DM9KS_CHIPR, iobase);
	id_val1 = dm9k_inb(iodata);

	DMFE_DEBUG("<DM9KS>: VID = 0x%x\n",id_val,0);

	if(id_val1!=0x18)
		return ETH_ERROR;
	
	if (id_val == DM9KS_ID || id_val == DM9010_ID) {
		
		dev->dev_open = dm9ks_open;
		dev->dev_start = dm9ks_start_xmit;
		dev->dev_receive = 0;
		dev->dev_output = NET_Ether_Send;
		dev->dev_input = NET_Ether_Input;
		dev->dev_ioctl =  dm9ks_ioctl;
		dev->dev_event =  0;

		dev->dev_type = DVT_ETHER;
		dev->dev_addrlen = DADDLEN;
		dev->dev_hdrlen = sizeof( DLAYER );
		dev->dev_mtu = ETHERNET_MTU;
		dev->dev_flags |= DV_BROADCAST;

		dev->dev_io_addr=DM9KS_ISA_IO_BASE;
		

		dev->dev_mac_addr[0] = mac_addr[0];
		dev->dev_mac_addr[1] = mac_addr[1];
		dev->dev_mac_addr[2] = mac_addr[2];
		dev->dev_mac_addr[3] = mac_addr[3];
		dev->dev_mac_addr[4] = mac_addr[4];
		dev->dev_mac_addr[5] = mac_addr[5];
/**/
		dm9ks_dev = dev;

//		req.dvr_dvru.dvru_data=dev->dev_mac_addr;
		
//		dm9ks_ioctl(dev,DEV_GET_MAC,&req);

	}else{
		return ETH_ERROR;
	}
	return ( (*(dev->dev_open)) (dev) );

}


void dm9ks_Get_ip(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo= dev->dev_addr.dev_ip_addr;
	ip[3]=lo&0xFF;
	ip[2]=(lo>>8)&0xFF;
	ip[1]=(lo>>16)&0xFF;
	ip[0]=(lo>>24)&0xFF;
	
}

void dm9ks_Set_ip( DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo=ip[3] | ip[2]<<8 | ip[1]<<16 | ip[0]<<24;

	dev->dev_addr.dev_ip_addr=lo;
}

void dm9ks_Get_mask(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo= dev->dev_addr.dev_netmask;
	ip[3]=lo&0xFF;
	ip[2]=(lo>>8)&0xFF;
	ip[1]=(lo>>16)&0xFF;
	ip[0]=(lo>>24)&0xFF;
	
}
void dm9ks_Set_mask(struct _DV_DEVICE_ENTRY *dev,u_int8 ip[4])
{
	u_int32 lo;
	lo=ip[3] | ip[2]<<8 | ip[1]<<16 | ip[0]<<24;

	dev->dev_addr.dev_netmask=lo;
}


void dm9ks_get_mac_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6])
{

	//lock;
	mac[0]=ior(0x10);
	mac[1]=ior(0x11);
	mac[2]=ior(0x12);
	mac[3]=ior(0x13);
	mac[4]=ior(0x14);
	mac[5]=ior(0x15);

	//unlock;
}

void dm9ks_set_mac_address(struct _DV_DEVICE_ENTRY *dev,u_int8 mac[6])
{
	
	dev->dev_mac_addr[0]=mac[0];
	dev->dev_mac_addr[1]=mac[1];
	dev->dev_mac_addr[2]=mac[2];
	dev->dev_mac_addr[3]=mac[3];
	dev->dev_mac_addr[4]=mac[4];
	dev->dev_mac_addr[5]=mac[5];
	
	iow(0x10, dev->dev_mac_addr[0]);
	iow(0x11, dev->dev_mac_addr[1]);
	iow(0x12, dev->dev_mac_addr[2]);
	iow(0x13, dev->dev_mac_addr[3]);
	iow(0x14, dev->dev_mac_addr[4]);
	iow(0x15, dev->dev_mac_addr[5]);


}

STATUS dm9ks_ioctl( DV_DEVICE_ENTRY *dev, int option, DV_REQ *request )
{
	STATUS result;
//	board_info_t *db=(board_info_t*)(dev->user_defined_1);
	
	switch (option)
	{
		case DEV_ADDMULTI:
			NET_Add_Multi(dev, request);
//			result=dm9000_hash_table(dev);
			break;
		case DEV_DELMULTI:
			NET_Del_Multi(dev, request);
//			result=dm9000_hash_table(dev);
			break;
		case DEV_GET_MAC:
//			dm9ks_get_mac_address(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_MAC:	
//			dm9ks_set_mac_address(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_GET_IP:
			dm9ks_Get_ip(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_IP:
			dm9ks_Set_ip(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_GET_MASK:
			dm9ks_Get_mask(dev, request->dvr_dvru.dvru_data);
			break;
		case DEV_SET_MASK:
			dm9ks_Set_mask(dev, request->dvr_dvru.dvru_data);
			break;
						
		case DEV_GET_LINK_STATE:
			request->dvr_dvru.drvu_flags = dm9ks_Speed;
			break;
			
		default:
			result=ETH_SUCCESS;
			break;
	}
   return result;
}



STATUS dm9ks_open(struct _DV_DEVICE_ENTRY *dev)
{
	ETH_STATUS result=ETH_SUCCESS;
	CNXT_GPIO_STATUS tmp;
//	int tick;
	
	dm9ks_io_addr = iobase;
	dm9ks_io_data = iodata;


#if 1//TASK_LINK_DETECT

	dm9ks_link_task_id=task_create(
						(PFNTASK)dm9ks_link_task ,dev,NULL,
						DAVICOM_STACK_SIZE, DAVICOM_TASK_PRIO,
						"DMFE_LINK_TASK" );

	if(dm9ks_link_task_id==0)
	{
		DMFE_DEBUG("<DM9KS>:Error task_create .\n",0,0);
		return ETH_ERROR;
	}else{

		DMFE_DEBUG("<DM9KS>:task_created .\n",0,0);

	}
#else

	/* Create timer to polling the DM9000AE status */
	dm9ks_polling_tick_id=tick_create(dm9ks_polling, 0, "DMFE");
	if(dm9ks_polling_tick_id==0)
	{
		DMFE_DEBUG("<dm9ks>:DMTICK NONE!\n",0,0);
		return ETH_ERROR;
	}

	tick = tick_set(dm9ks_polling_tick_id, 3000, FALSE);
       if(tick != RC_OK)
        {
            DMFE_DEBUG("<dm9ks>:DMTICK NOT SET!\n",0,0);
            return ETH_ERROR; /* Should never be reached */
        }  

#endif
	


	dm9ks_init_dm9000(dev);

//	udelay(1000);
	
	/* Init driver variable */
	dm9ks_reset_counter 	= 0;
	dm9ks_reset_tx_timeout 	= 0;
//	dm9ks_cont_rx_pkt_cnt	= 0;

	cnxt_gpio_set_input(DM9KS_HOST_INTERRUPT);
	cnxt_gpio_set_int_edge(DM9KS_HOST_INTERRUPT, POS_EDGE);//NEG_EDGE);//POS_EDGE);
	cnxt_gpio_clear_pic_interrupt(DM9KS_HOST_INTERRUPT);

	tmp=cnxt_gpio_int_register_isr(DM9KS_HOST_INTERRUPT, 
						(PFNISR)dm9ks_interrupt, FALSE, FALSE, &dm9ks_pFnChain );
	if( CNXT_GPIO_STATUS_OK != tmp )
	{
		DMFE_DEBUG("<DM9KS>:Error registering ISR handler .\n",0,0); 
		return ETH_ERROR;
	}
		
	/*enable GPIO INT*/
	if (CNXT_GPIO_STATUS_OK != cnxt_gpio_int_enable(DM9KS_HOST_INTERRUPT))
	{
		DMFE_DEBUG("<DM9KS>:Error enable GPIO INT .\n",0,0); 
		result= ETH_ERROR ;
	}   

//	cnxt_gpio_clear_pic_interrupt(DM9KS_HOST_INTERRUPT);	
	
	iow( DM9KS_RXCR, DM9KS_REG05 | 1);	/* RX enable */
	iow( DM9KS_IMR, DM9KS_REGFF); 	// Enable TX/RX interrupt mask
	

#if 0
	if(tick_start(dm9ks_polling_tick_id)!=RC_OK)
	{
			DMFE_DEBUG("<DM9KS>:FAIL .\n",0,0); 
			tick_destroy(dm9ks_polling_tick_id);
			return ETH_ERROR;
	}
	DMFE_DEBUG("<DM9KS>:tick_start --- dm9ks_polling_tick_id .\n",0,0); 
#endif
	return result;


}







/* Set PHY operationg mode
*/
void set_PHY_mode(void)
{
	u_int16 phy_reg0 = 0x1200;		/* Auto-negotiation & Restart Auto-negotiation */
	u_int16 phy_reg4 = 0x01e1;		/* Default flow control disable*/

	if ( !(dm9ks_op_mode & DM9KS_AUTO) ) // op_mode didn't auto sense */
	{ 
		switch(dm9ks_op_mode) {
			case DM9KS_10MHD:  phy_reg4 = 0x21; 
                        	           phy_reg0 = 0x1000;
					   break;
			case DM9KS_10MFD:  phy_reg4 = 0x41; 
					   phy_reg0 = 0x1100;
                                	   break;
			case DM9KS_100MHD: phy_reg4 = 0x81; 
					   phy_reg0 = 0x3000;
				    	   break;
			case DM9KS_100MFD: phy_reg4 = 0x101; 
					   phy_reg0 = 0x3100;
				   	   break;
			default: 
					   break;
		} // end of switch
	} // end of if
	phy_write( 0, phy_reg0);
	phy_write( 4, phy_reg4);
}


/* 
	Initilize dm9000 board
*/
void dm9ks_init_dm9000(struct _DV_DEVICE_ENTRY *dev)
{
	u_int8 tmp_PBCR=0;
	
	DMFE_DEBUG("<DM9KS>:dmfe_init_dm9000()\n",0,0);

	/* set the internal PHY power-on, GPIOs normal, and wait 2ms */
	iow( DM9KS_GPR, 1); 	/* Power-Down PHY */
	udelay(500);
	iow( DM9KS_GPR, 0);	/* GPR (reg_1Fh)bit GPIO0=0 pre-activate PHY */
	udelay(20);		/* wait 2ms for PHY power-on ready */

	/* do a software reset and wait 20us */
	iow( DM9KS_NCR, 3);
	udelay(20);		/* wait 20us at least for software reset ok */
	iow( DM9KS_NCR, 3);	/* NCR (reg_00h) bit[0] RST=1 & Loopback=1, reset on */
	udelay(20);		/* wait 20us at least for software reset ok */

	/* I/O mode */
	dm9ks_io_mode = ior( DM9KS_ISR) >> 6; /* ISR bit7:6 keeps I/O mode */
	DMFE_DEBUG("<DM9KS>:io_mode = 0x%x\n",dm9ks_io_mode,0);

	/* Set PHY */
	dm9ks_op_mode = dm9ks_media_mode;
	set_PHY_mode();

	tmp_PBCR = ior(DM9KS_BUSCR);
	tmp_PBCR |= 0xe0;
	DMFE_DEBUG("<DM9KS>:DM9KS_BUSCR = 0x%x\n",tmp_PBCR,0);
	iow(DM9KS_BUSCR, tmp_PBCR );

	/* Program operating register */
	iow( DM9KS_NCR, 0);
	iow( DM9KS_TCR, 0);		/* TX Polling clear */
	
	iow( DM9KS_TCR2, 0x80);	/* LED mode */
	
	iow( DM9KS_BPTR, 0x3f);	/* Less 3kb, 600us */
	iow( DM9KS_SMCR, 0);		/* Special Mode */
	iow( DM9KS_NSR, 0x2c);	/* clear TX status */

#if defined(DM9000AE_INT_HI)			/* interrupt high active */
	iow( DM9KS_INTCR, 0x00);
#else
	iow( DM9KS_INTCR, 0x01);
#endif
	
	iow( DM9KS_ISR, 0x0f); 	/* Clear interrupt status */

	/* Added by jackal at 03/29/2004 */
#if defined(CHECKSUM)
	DMFE_DEBUG("<DM9KS>:Open TCP/IP/UDP checksum\n",0,0);
	iow( DM9KS_TCCR, 0x07);	/* TX UDP/TCP/IP checksum enable */
	iow( DM9KS_RCSR, 0x02);	/*Receive checksum enable */
#endif

#if defined(ETRANS)
	DMFE_DEBUG("<DM9KS>:Open Early Transmit Function\n",0,0);
	iow( DM9KS_ETXCSR, 0x83);
#endif
 
	/* Set address filter table */
	dm9000_hash_table(dev);

	/* Activate DM9000A/DM9010 */
//	iow( DM9KS_RXCR, DM9KS_REG05 | 1);	/* RX enable */
//	iow( DM9KS_IMR, DM9KS_REGFF); 	// Enable TX/RX interrupt mask
 
	/* Init Driver variable */
	dm9ks_Speed = 0;
	dm9ks_tx_pkt_cnt = 0;
	dm9ks_Link_change = 0;
	dm9ks_queue_pkt_len = 0;

	dm9ks_tx_packets = 0;

}

/*
  Hardware start transmission.
  Send a packet to media from the upper layer.
*/
STATUS dm9ks_start_xmit(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf )
{
	char * data_ptr;
	UINT32 i=0, tmplen_t=0;
	UINT16 data_len=0,segment_len=0;
	NET_BUFFER *work_buf = buf;
	u_int16 segments=0,j=0;
	bool section_t;


//	DMFE_DEBUG("<dmfe>:Ethernet Tansmite---->>\n",0,0);

	section_t = critical_section_begin();

	cnxt_gpio_int_disable(DM9KS_HOST_INTERRUPT);

	/* Disable all interrupt */
	iow( DM9KS_IMR, DM9KS_DISINTR);

	if(dm9ks_Speed == 0)
	{
		iow( DM9KS_IMR, DM9KS_REGFF);
		cnxt_gpio_int_enable(DM9KS_HOST_INTERRUPT);
		critical_section_end( section_t);

		DEV_Recover_TX_Buffers (dev);	//debug 0828
		
		DMFE_DEBUG("<dmfe>:ERROR!!! Ethernet Tansmite(NO LINK)\n",0,0);

		
		return NU_SUCCESS;// ETH_LINKDOWN;	//NU_SUCCESS;	
	}

	if(dm9ks_tx_pkt_cnt>1)
	{
		iow( DM9KS_IMR, DM9KS_REGFF);		
		cnxt_gpio_int_enable(DM9KS_HOST_INTERRUPT);
		critical_section_end( section_t);

//		DEV_Recover_TX_Buffers (dev);	//debug 0828
		/*
		if(dm9ks_tx_pkt_cnt!=1 && dm9ks_tx_pkt_cnt!=0)
		{
			DMFE_DEBUG("<dmfe>:ERROR!!! dm9ks_tx_pkt_cnt = %d\n",dm9ks_tx_pkt_cnt,0);
			dm9ks_tx_pkt_cnt = 0;
			
		}*/


		DMFE_DEBUG("<dmfe>:ERROR!!! dm9ks_tx_pkt_cnt > 1( = %d)\n",dm9ks_tx_pkt_cnt,0);

		
		return ETH_NOFIFO;//NU_INVALID_MEMORY;	//NU_SUCCESS;
	}

	segments = 0;
	
	while(work_buf)
	{
		segments++;
		data_len += work_buf->data_len;
		work_buf=work_buf->next_buffer;
	}

	work_buf=buf;

	/* Set TX length to reg. 0xfc & 0xfd */
//	iow( DM9KS_TXPLL, (data_len & 0xff));
//	iow( DM9KS_TXPLH, (data_len >> 8) & 0xff);

	dm9k_outb(DM9KS_MWCMD, dm9ks_io_addr); // Write data into SRAM trigger

	for(j=0;j<segments;j++)
	{

		segment_len = work_buf->data_len;
		
		/* Move data to TX SRAM */
		data_ptr = (char *)work_buf->data_ptr;
		
		switch(dm9ks_io_mode)
		{
			case DM9KS_BYTE_MODE:
				for (i = 0; i < segment_len; i++)
					dm9k_outb((data_ptr[i] & 0xff), dm9ks_io_data);
				break;
			case DM9KS_WORD_MODE:
				tmplen_t = (segment_len + 1) / 2;
				for (i = 0; i < tmplen_t; i++)
				{
	         			dm9k_outw(((u_int16 *)data_ptr)[i], dm9ks_io_data);
				}
	         		break;
		}

		work_buf=work_buf->next_buffer;

	}

	if(dm9ks_tx_pkt_cnt==0)
	{
	
		/* Set TX length to reg. 0xfc & 0xfd */
		iow( DM9KS_TXPLL, (data_len & 0xff));
		iow( DM9KS_TXPLH, (data_len >> 8) & 0xff);
	
#if !defined(ETRANS)
		/* Issue TX polling command */
		iow( DM9KS_TCR, 0x1); /* Cleared after TX complete*/
#endif
		i = 7500;
		while(i--);
//		DMFE_DEBUG("<dmfe>:debug xmit data ---dm9ks_tx_pkt_cnt = 0---\n",0,0);

//		dm9ks_tx_packets++;
		dm9ks_tx_bytes+=data_len;

	}else{
		/* Second packet */
		dm9ks_queue_pkt_len = data_len;
	}

	dm9ks_tx_pkt_cnt++;		//dm9ks max 2 packets

	/* packet counting */

	/* Saved the time stamp */
//	dev->trans_start = jiffies;
//	dm9ks_cont_rx_pkt_cnt =0;

	dm9ks_tx_packets++;
//	DMFE_DEBUG("<dmfe>:debug xmit data counts = %d\n",dm9ks_tx_packets,0);

	/* Re-enable interrupt */
	iow( DM9KS_IMR, DM9KS_REGFF);
	cnxt_gpio_int_enable(DM9KS_HOST_INTERRUPT);
	critical_section_end( section_t);

	/* Free this SKB */
	DEV_Recover_TX_Buffers (dev);

//	DMFE_DEBUG("<DM9KS>:dm9ks_tx_pkt_cnt++ (0x%x)\n",dm9ks_tx_pkt_cnt,0);
	
	
	return ETH_SUCCESS;

}

#if 0
STATUS dm9ks_start_xmit(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf )
{

	/* Disable all interrupt */
	iow( DM9KS_IMR, DM9KS_DISINTR);
	
	while(1)
	{
		if(dm9ks_tx_xmit(dev,buf)==1)
		{
			/* Free this SKB */
			DEV_Recover_TX_Buffers (dev);
			return NU_SUCCESS;
		}else
		{
			
			/* Free this SKB */
			DEV_Recover_TX_Buffers (dev);
			return NU_SUCCESS;
		}
	}
	
	/* Re-enable interrupt */
	iow( DM9KS_IMR, DM9KS_REGFF);
	
	return NU_SUCCESS;
}

#endif

#if 0
/*
  Stop the interface.
  The interface is stopped when it is brought.
*/
int dmfe_stop(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_stop", 0);

	/* deleted timer */
	del_timer(&db->timer);

	netif_stop_queue(dev); 

	/* free interrupt */
	free_irq(dev->irq, dev);

	/* RESET devie */
	phy_write( 0x00, 0x8000);	/* PHY RESET */
	iow( DM9KS_GPR, 0x01); 	/* Power-Down PHY */
	iow( DM9KS_IMR, DM9KS_DISINTR);	/* Disable all interrupt */
	iow( DM9KS_RXCR, 0x00);	/* Disable RX */

	/* Dump Statistic counter */
#if FALSE
	printk("\nRX FIFO OVERFLOW %lx\n", db->stats.rx_fifo_errors);
	printk("RX CRC %lx\n", db->stats.rx_crc_errors);
	printk("RX LEN Err %lx\n", db->stats.rx_length_errors);
	printk("RESET %x\n", db->reset_counter);
	printk("RESET: TX Timeout %x\n", db->reset_tx_timeout);
	printk("g_TX_nsr %x\n", g_TX_nsr);
#endif

	return 0;
}
#endif

void dmfe_tx_done(unsigned long unused)
{
	u_int8 tx_status,tx_status_1;

	u_int32 i;

//	DEBUG_OUT("dmfe_tx_done()");
	
	tx_status = ior( DM9KS_NSR);

	tx_status_1 = ior( DM9KS_NSR);
//	DMFE_DEBUG("<dm9ks>:tx_status = 0x%x  RE_read = 0x%x\n",tx_status,tx_status_1);

//	tx_status_1 = ior( DM9KS_NSR);
//	DMFE_DEBUG("<dm9ks>:tx_status_1 = 0x%x\n",tx_status_1,0);
	
	if (tx_status & 0xc) 
	{
		/* One packet sent complete */
		dm9ks_tx_pkt_cnt--;
//		DMFE_DEBUG("<DM9KS>:dm9ks_tx_pkt_cnt-- (0x%x)\n",dm9ks_tx_pkt_cnt,0);
			/*
			if (dm9ks_tx_pkt_cnt != 0 && dm9ks_tx_pkt_cnt != 1)
			{
				//printk("[dmfe_tx_done] dm9ks_tx_pkt_cnt ERROR!!\n");
				dm9ks_tx_pkt_cnt =0;
			}
			*/
			/* Queue packet check & send */
			if (dm9ks_tx_pkt_cnt > 0)
			{
				/* Set TX length to DM9000 */
				iow( DM9KS_TXPLL, (dm9ks_queue_pkt_len & 0xff));
				iow( DM9KS_TXPLH, (dm9ks_queue_pkt_len >> 8) & 0xff);
		
				/* Issue TX polling command */
				iow( DM9KS_TCR, 0x1);    /* Cleared after TX complete */
				
				i = 7500;
				while(i--);
				
				dm9ks_tx_packets++;
				dm9ks_tx_bytes+=dm9ks_queue_pkt_len;
			} 
				
	}

}

/*
  DM9000 insterrupt handler
  receive the packet to upper layer, free the transmitted packet
*/
int32 dm9ks_interrupt(u_int32 irq, bool FIQ, PFNISR *pfnChain)
{

	u_int8 reg_save;
	u_int8 int_status=0,int_status_1=0;
	bool section_t;

	*pfnChain = NULL; /* ISA interrupts not shared.. */

	section_t = critical_section_begin();
//	int_disable(irq);	
	cnxt_gpio_int_disable(DM9KS_HOST_INTERRUPT);

	
	/* Save previous register address */
	reg_save = dm9k_inb(dm9ks_io_addr);

	/* Disable all interrupt */
	iow( DM9KS_IMR, DM9KS_DISINTR); 
	

	/* Got DM9000A/DM9010 interrupt status */
	int_status = ior( DM9KS_ISR);		/* Got ISR */	
	iow( DM9KS_ISR, int_status);		/* Clear ISR status */ 

	int_status_1 = ior( DM9KS_ISR);		/* Got ISR */	
//	DMFE_DEBUG("<dm9ks>:int_status = 0x%x RE_read = 0x%x\n",int_status,int_status_1);
	
//	int_status_1 = ior( DM9KS_ISR);		/* Got ISR */	
//	iow( DM9KS_ISR, int_status_1);		/* Clear ISR status */ 
//	DMFE_DEBUG("<dm9ks>:int_status_1 = 0x%x\n",int_status_1,0);

	/* Link status change */
	if (int_status & DM9KS_LINK_INTR) 
	{
		dm9ks_Speed = 0;
		dm9ks_Link_change = 1;
	#if 0
	#if 0
		if (RC_OK!=event_send(db->dm9ks_task_id,db->event_link))
		{	
			DMFE_DEBUG("<dm9ks>:Interrupt Link Change: event_send error!!!\n",0,0);
		}
	
	#else
			phy_read(0x1);
			if(phy_read(0x1) & 0x4) /*Link OK*/
			{
				/* set media speed */
				if(phy_read(0)&0x2000) db->Speed =100;
				else db->Speed =10;
			}
			else
				db->Speed =0;
		//printk("[INTR]i=%d speed=%d\n",i, (int)(db->Speed));	
	#endif
	#endif
	}


	/* Trnasmit Interrupt check */
	if (int_status & DM9KS_TX_INTR)
		dmfe_tx_done(0);


#if 1
	/* Received the coming packet */
	if (int_status & DM9KS_RX_INTR) 
	{
#if 0
//		sem_put(db->rx_sem);
		if (RC_OK!=event_send(db->dm9ks_task_id,db->event_rs))
		{	
			DMFE_DEBUG("<dm9ks>:Interrupt RX: event_send error!!!\n",0,0);
		}
#else
		dm9ks_packet_receive();
#endif
	}
#endif	


	critical_section_end( section_t);
//	int_enable(irq);
	cnxt_gpio_int_enable(DM9KS_HOST_INTERRUPT);


	/* Re-enable interrupt mask */ 
	iow( DM9KS_IMR, DM9KS_REGFF);
	

	/* Restore previous register address */
	dm9k_outb(reg_save, dm9ks_io_addr); 

	return RC_ISR_HANDLED;
}

#if 0
/*
  Get statistics from driver.
*/
struct net_device_stats * dmfe_get_stats(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_get_stats", 0);
	return &db->stats;
}

/*
  Process the upper socket ioctl command
*/
int dmfe_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	DMFE_DBUG(0, "dmfe_do_ioctl()", 0);
	return 0;
}

/* Our watchdog timed out. Called by the networking layer */
void
dmfe_timeout(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)dev->priv;

	DMFE_DBUG(0, "dmfe_TX_timeout()", 0);
	printk("TX time-out -- dmfe_timeout().\n");
	db->reset_tx_timeout++;
	db->stats.tx_errors++;
#if FALSE
	printk("TX packet count = %d\n", db->tx_pkt_cnt);	
	printk("TX timeout = %d\n", db->reset_tx_timeout);	
	printk("22H=0x%02x  23H=0x%02x\n",ior(0x22),ior(0x23));
	printk("faH=0x%02x  fbH=0x%02x\n",ior(0xfa),ior(0xfb));
#endif
	dmfe_reset(dev);
	
}

void dmfe_reset(struct net_device * dev)
{
	board_info_t *db = (board_info_t *)dev->priv;
	u_int8 reg_save;
	int i;
	/* Save previous register address */
	reg_save = dm9k_inb(db->io_addr);

	netif_stop_queue(dev); 
	db->reset_counter++;
	dmfe_init_dm9000(dev);
	
	db->Speed =10;
	for(i=0; i<1000; i++) /*wait link OK, waiting time=1 second */
	{
		if(phy_read(0x1) & 0x4) /*Link OK*/
		{
			if(phy_read(0)&0x2000) db->Speed =100;
			else db->Speed =10;
			break;
		}
		udelay(1000);
	}
	
	netif_wake_queue(dev);
	
	/* Restore previous register address */
	dm9k_outb(reg_save, db->io_addr);

}
/*
  A periodic timer routine
*/
void dmfe_timer(unsigned long data)
{
	struct net_device * dev = (struct net_device *)data;
	board_info_t *db = (board_info_t *)dev->priv;
	DMFE_DBUG(0, "dmfe_timer()", 0);
	
	if (db->cont_rx_pkt_cnt>=CONT_RX_PKT_CNT)
	{
		db->cont_rx_pkt_cnt=0;
		iow( DM9KS_IMR, DM9KS_REGFF);
	}
	/* Set timer again */
	db->timer.expires = DMFE_TIMER_WUT;
	add_timer(&db->timer);
	
	return;
}



#endif


#if !defined(CHECKSUM)
#define check_rx_ready(a)	((a) == 0x01)
#else
inline u_int8 check_rx_ready(u_int8 rxbyte)
{
	if (!(rxbyte & 0x01))		
		return 0;	
	return ((rxbyte >> 4) | 0x01);
}
#endif



//TASK
void dm9ks_link_task(struct _DV_DEVICE_ENTRY *dev)
{
	events_t link_changed;
	INT32 rtn;
//	u_int32 dm9ks_events=0;
//	u_int32 link_change_events=0; //debug
	
	dm9ks_event_link=event_create();


	while(1)
	{
		link_changed = 0;
		
		rtn=event_receive(dm9ks_event_link,&link_changed, 6000,  FALSE);
//		DMFE_DEBUG("\n\n<DM9KS>: EVENTS = %d\n", dm9ks_events++,0);

		if(dm9ks_Link_change == 1)
		{
//			DMFE_DEBUG("<dm9ks>:Link status change!\n",0,0);
			phy_read(0x1);
			if(phy_read(0x1) & 0x4) /*Link OK*/
			{
				/* set media speed */
				if(phy_read(0)&0x2000)
				{
					dm9ks_Speed =100;
					DMFE_DEBUG("<dm9ks>:Linked = 100Mbps!\n",0,0);
				}
				else 
				{
					dm9ks_Speed =10;
					DMFE_DEBUG("<dm9ks>:Linked = 10Mbps!\n",0,0);
				}
			}
			else
			{
				dm9ks_Speed =0;
				DMFE_DEBUG("<dm9ks>:Link down!!!\n",0,0);
			}

			dm9ks_Link_change = 0;
		}
				
		if ( link_changed & dm9ks_event_link);// ||rtn!=RC_OK)
		{

//			DMFE_DEBUG("\n\n<DM9KS>: link_changed events = %d\n", link_change_events++,0);

		}

		
		if (rtn == RC_KAL_TIMEOUT){
			link_changed=0;
		}

	}

}


//




#if 0
/*
  Received a packet and pass to upper layer
*/
void dmfe_task(struct _DV_DEVICE_ENTRY *dev)
{
	board_info_t *db;
	events_t event_received,link_changed;
	INT32 rtn;
//	u_int8 rxbyte, val;
//	u_int16 i, GoodPacket, MDRAH, MDRAL, tmplen = 0;
//	rx_t rx;
//	u_int8* rdptr;	
//	u_int16 * ptr = (u_int16*)&rx;
//	NET_BUFFER *work_buf,*buf_ptr;
//	u_int32 bytes_left,templen;

	db = (board_info_t *)dev->user_defined_1;

	db->event_rs=event_create();

	db->event_link=event_create();

	while(1)
	{
		event_received = 0;
		link_changed = 0;
		
//		rtn=event_receive(db->event_rs,&event_received, KAL_WAIT_FOREVER, TRUE);//6000,  FALSE);
		rtn=event_receive(db->event_rs,&event_received, 6000,  FALSE);

		
#if 0


Link_change = 0;


		DMFE_DEBUG("\n\n<DM9KS>: EVENTS = %d\n", dm9ks_counts++,0);




		if ( (event_received & db->event_rs));// ||rtn!=RC_OK)
		{
			DMFE_DEBUG("<DM9KS>: dmfe_packet_receive\n\n",0,0);

			do {
				/*store the value of Memory Data Read address register*/
				MDRAH=ior( DM9KS_MDRAH);
				MDRAL=ior( DM9KS_MDRAL);
				
				ior( DM9KS_MRCMDX);		/* Dummy read */
				rxbyte = dm9k_inb(db->io_data);	/* Got most updated data */

				/* packet ready to receive check */
				if(!(val = check_rx_ready(rxbyte))) break;

				/* A packet ready now  & Get status/length */
				GoodPacket = TRUE;
				dm9k_outb(DM9KS_MRCMD, db->io_addr);

				/* Read packet status & length */
				switch (db->io_mode) 
					{
					  case DM9KS_BYTE_MODE: 
		 				    *ptr = dm9k_inb(db->io_data) + 
						               (dm9k_inb(db->io_data) << 8);
						    *(ptr+1) = dm9k_inb(db->io_data) + 
							    (dm9k_inb(db->io_data) << 8);
						    break;
					  case DM9KS_WORD_MODE:
						    *ptr = dm9k_inw(db->io_data);
						    *(ptr+1)    = dm9k_inw(db->io_data);
						    break;
							
					  default:
						    break;
					}

				/* Packet status check */
				if (rx.desc.status & 0xbf)
				{
					GoodPacket = FALSE;
					if (rx.desc.status & 0x01) 
					{
						db->rx_fifo_errors++;
						DMFE_DEBUG("<RX FIFO error>\n",0,0);
					}
					if (rx.desc.status & 0x02) 
					{
						db->rx_crc_errors++;
						DMFE_DEBUG("<RX CRC error>\n",0,0);
					}
					if (rx.desc.status & 0x80) 
					{
						db->rx_length_errors++;
						DMFE_DEBUG("<RX Length error>\n",0,0);
					}
					if (rx.desc.status & 0x08)
						DMFE_DEBUG("<Physical Layer error>\n",0,0);
				}

				if (!GoodPacket)
				{
					// drop this packet!!!
					switch (db->io_mode)
					{
						case DM9KS_BYTE_MODE:
					 		for (i=0; i<rx.desc.length; i++)
								dm9k_inb(db->io_data);
							break;
						case DM9KS_WORD_MODE:
							tmplen = (rx.desc.length + 1) / 2;
							for (i = 0; i < tmplen; i++)
								dm9k_inw(db->io_data);
							break;
							
					}
					continue;/*next the packet*/
				}
				
//				skb = dev_alloc_skb(rx.desc.length+4);
				work_buf = buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, rx.desc.length );
				bytes_left=rx.desc.length;

				DMFE_DEBUG("<DM9KS>:debug:rx_data_len = 0x%x(%d)\n",rx.desc.length,rx.desc.length);
				
				if (work_buf == NULL )
				{	
					DMFE_DEBUG("<dm9ks>:%s: Memory squeeze.\n", dev->dev_net_if_name,0);
					/*re-load the value into Memory data read address register*/
					iow(DM9KS_MDRAH,MDRAH);
					iow(DM9KS_MDRAL,MDRAL);
					return;
				}else
				{
					
					buf_ptr->mem_total_data_len = bytes_left;
					buf_ptr->mem_buf_device = dev;
					buf_ptr->mem_flags = 0;
					
					/*Parent Packets*/				
					templen=(bytes_left>NET_PARENT_BUFFER_SIZE)?NET_PARENT_BUFFER_SIZE:bytes_left;
					do
					{
						
						work_buf->data_len = templen; 
						work_buf->data_ptr = (u_int8*)(&work_buf->me_data.packet[0]); /* point to data storage */
//						work_buf->data_ptr = (u_int8*)(&work_buf->mem_parent_packet); /* point to data storage */
						bytes_left-=templen;

						/* Move data from DM9000 */
						rdptr = (u_int8*)work_buf->data_ptr;

						/* Read received packet from RX SARM */
						switch (db->io_mode)
						{
							case DM9KS_BYTE_MODE:
						 		for (i=0; i<templen; i++)
									rdptr[i]=dm9k_inb(db->io_data);
								break;
							case DM9KS_WORD_MODE:
								tmplen = (templen + 1) / 2;
								for (i = 0; i < tmplen; i++)
								{
									((u_int16 *)rdptr)[i] = dm9k_inw(db->io_data);
//									DMFE_DEBUG("0x%04x",((u_int16 *)rdptr)[i],0);
								}
								break;
								
						}				
						templen=(bytes_left>NET_MAX_BUFFER_SIZE)?NET_MAX_BUFFER_SIZE:bytes_left;
						work_buf = work_buf->next_buffer ;	
					}while( (work_buf != NU_NULL) && bytes_left );
					
					/* Pass to upper layer */

						
					MEM_Buffer_Enqueue (&MEM_Buffer_List, buf_ptr );	
					NU_Set_Events (&Buffers_Available, 2, NU_OR);	
						
//					dev->last_rx=jiffies;
					db->rx_packets++;
					db->rx_bytes += rx.desc.length;
					db->cont_rx_pkt_cnt++;
						
				}
					
			}while((rxbyte & 0x01) == DM9KS_PKT_RDY);

		}

		if ( (link_changed & db->event_link));// ||rtn!=RC_OK)
		{
			DMFE_DEBUG("<DM9KS>: dmfe_link_change\n\n",0,0);
			
			if(phy_read(0x1) & 0x4) /*Link OK*/
			{

				db->Speed = 10;
			}else
				db->Speed = 0;

		}



#if 0		
			phy_read(0x1);
			if(phy_read(0x1) & 0x4) /*Link OK*/
			{
				/* wait for detected Speed */
				for(i=0; i<200;i++)
					udelay(1000);
				/* set media speed */
				if(phy_read(0)&0x2000) db->Speed =100;
				else db->Speed =10;
				break;
			}
			udelay(1000);
#endif
			

		if (rtn == RC_KAL_TIMEOUT){
			event_received=0;
		}
#endif
	}
}
#endif


void dm9ks_packet_receive(void)
{

	u_int8 rxbyte, val;
	u_int16 i, GoodPacket,  MDRAH, MDRAL;
	rx_t rx;
	u_int8* rdptr;	
	u_int16 * ptr = (u_int16*)&rx;
	NET_BUFFER *work_buf,*buf_ptr;
	u_int32 bytes_left,len,tmplen_t,dwFrameLen;

//	db = (board_info_t *)dm9ks_dev->user_defined_1;
	
			do {
				/*store the value of Memory Data Read address register*/
				MDRAH=ior( DM9KS_MDRAH);
				MDRAL=ior( DM9KS_MDRAL);
				
				ior( DM9KS_MRCMDX);		/* Dummy read */
				rxbyte = dm9k_inb(dm9ks_io_data);	/* Got most updated data */

				/* packet ready to receive check */
				if(!(val = check_rx_ready(rxbyte))) break;

				/* A packet ready now  & Get status/length */
				GoodPacket = TRUE;
				dm9k_outb(DM9KS_MRCMD, dm9ks_io_addr);

				/* Read packet status & length */
				switch (dm9ks_io_mode) 
					{
					  case DM9KS_BYTE_MODE: 
		 				    *ptr = dm9k_inb(dm9ks_io_data) + 
						               (dm9k_inb(dm9ks_io_data) << 8);
						    *(ptr+1) = dm9k_inb(dm9ks_io_data) + 
							    (dm9k_inb(dm9ks_io_data) << 8);
						    break;
					  case DM9KS_WORD_MODE:
						    *ptr = dm9k_inw(dm9ks_io_data);
						    *(ptr+1)    = dm9k_inw(dm9ks_io_data);
						    break;
							
					  default:
						    break;
					}

				/* Packet status check */
				if (rx.desc.status & 0xbf)
				{
					GoodPacket = FALSE;
					if (rx.desc.status & 0x01) 
					{
						dm9ks_rx_fifo_errors++;
						DMFE_DEBUG("<RX FIFO error>\n",0,0);
					}
					if (rx.desc.status & 0x02) 
					{
						dm9ks_rx_crc_errors++;
						DMFE_DEBUG("<RX CRC error>\n",0,0);
					}
					if (rx.desc.status & 0x80) 
					{
						dm9ks_rx_length_errors++;
						DMFE_DEBUG("<RX Length error>\n",0,0);
					}
					if (rx.desc.status & 0x08)
						DMFE_DEBUG("<Physical Layer error>\n",0,0);
				}

				if (!GoodPacket)
				{
					// drop this packet!!!
					switch (dm9ks_io_mode)
					{
						case DM9KS_BYTE_MODE:
					 		for (i=0; i<rx.desc.length; i++)
								dm9k_inb(dm9ks_io_data);
							break;
						case DM9KS_WORD_MODE:
							tmplen_t = (rx.desc.length + 1) / 2;
							for (i = 0; i < tmplen_t; i++)
								dm9k_inw(dm9ks_io_data);
							break;
							
					}
					continue;/*next the packet*/
				}
				
				dwFrameLen = rx.desc.length;
				
				work_buf = buf_ptr = MEM_Buffer_Chain_Dequeue(&MEM_Buffer_Freelist, dwFrameLen);
				bytes_left=dwFrameLen;

//				DMFE_DEBUG("<DM9KS>:debug:rx_data_len = 0x%x(%d)\n",rx.desc.length,rx.desc.length);
				
				if (buf_ptr == NULL )
				{	
					DMFE_DEBUG("<dm9ks>: Memory squeeze.\n",0,0);
					/*re-load the value into Memory data read address register*/
					iow(DM9KS_MDRAH,MDRAH);
					iow(DM9KS_MDRAL,MDRAL);
					return;
				}else
				{
					
					buf_ptr->mem_total_data_len = bytes_left;
					buf_ptr->mem_buf_device = dm9ks_dev;
					buf_ptr->mem_flags = 0;
					
					/*Parent Packets*/				
					len=(bytes_left>NET_PARENT_BUFFER_SIZE)?NET_PARENT_BUFFER_SIZE:bytes_left;
					do
					{
						
						work_buf->data_len = len; 
						work_buf->data_ptr = (u_int8*)(&work_buf->mem_parent_packet); /* point to data storage */
						bytes_left-=len;

						/* Move data from DM9000 */
						rdptr = (u_int8*)work_buf->data_ptr;

						/* Read received packet from RX SARM */
						switch (dm9ks_io_mode)
						{
							case DM9KS_BYTE_MODE:
						 		for (i=0; i<len; i++)
									rdptr[i]=dm9k_inb(dm9ks_io_data);
								break;
							case DM9KS_WORD_MODE:
								tmplen_t = (len + 1) / 2;
								for (i = 0; i < tmplen_t; i++)
								{
									((u_int16 *)rdptr)[i] = dm9k_inw(dm9ks_io_data);
//									DMFE_DEBUG("0x%04x",((u_int16 *)rdptr)[i],0);
								}
								break;
								
						}				
						len=(bytes_left>NET_MAX_BUFFER_SIZE)?NET_MAX_BUFFER_SIZE:bytes_left;
						work_buf = work_buf->next_buffer ;	
					}while( (work_buf != NU_NULL) && bytes_left );
					
					/* Pass to upper layer */

						
					MEM_Buffer_Enqueue (&MEM_Buffer_List, buf_ptr );	
					NU_Set_Events (&Buffers_Available, 2, NU_OR);	
						
//					dev->last_rx=jiffies;
					dm9ks_rx_packets++;
					dm9ks_rx_bytes += rx.desc.length;
//					dm9ks_cont_rx_pkt_cnt++;
						
				}
					
			}while((rxbyte & 0x01) == DM9KS_PKT_RDY);


}


#if 0

/*
  Read a word data from SROM
*/
u_int16 read_srom_word( int offset)
{
	iow( DM9KS_EPAR, offset);
	iow( DM9KS_EPCR, 0x4);
	udelay(200);
	iow( DM9KS_EPCR, 0x0);
	return (ior( DM9KS_EPDRL) + (ior( DM9KS_EPDRH) << 8) );
}
#endif

/*
  Set DM9000A/DM9010 multicast address
*/
void dm9000_hash_table(struct _DV_DEVICE_ENTRY *dev)
{
//	board_info_t *db = (board_info_t *)dev->user_defined_1;
	u_int16 i, oft, hash_table[4];

	//DEBUG_OUT("dm9000_hash_table() --->");

	/* Set Node address */
#if 1
	iow(0x10, dev->dev_mac_addr[0]);
	iow(0x11, dev->dev_mac_addr[1]);
	iow(0x12, dev->dev_mac_addr[2]);
	iow(0x13, dev->dev_mac_addr[3]);
	iow(0x14, dev->dev_mac_addr[4]);
	iow(0x15, dev->dev_mac_addr[5]);
#else

	iow(0x10, 0x00);
	iow(0x11, 0x11);
	iow(0x12, 0x22);
	iow(0x13, 0x33);
	iow(0x14, 0x44);
	iow(0x15, 0x55);

#endif
	/* Clear Hash Table */
	for (i = 0; i < 4; i++)
		hash_table[i] = 0x0;

	/* broadcast address */
	hash_table[3] = 0x8000;
#if 0
	/* the multicast address in Hash Table : 64 bits */
	for (i = 0; i < mc_cnt; i++, mcptr = mcptr->next) {
		hash_val = cal_CRC((char *)mcptr->dmi_addr, 6, 0) & 0x3f; 
		hash_table[hash_val / 16] |= (u_int16) 1 << (hash_val % 16);
	}
#endif
	/* Write the hash table to MAC MD table */
	for (i = 0, oft = 0x16; i < 4; i++) {
		iow( oft++, hash_table[i] & 0xff);
		iow( oft++, (hash_table[i] >> 8) & 0xff);
	}
	DMFE_DEBUG("<DM9KS>:MAC addr: 0x%x 0x%x\n",ior( 0x10),ior( 0x11));
	DMFE_DEBUG("<DM9KS>:MAC addr: 0x%x 0x%x\n",ior( 0x12),ior( 0x13));
	DMFE_DEBUG("<DM9KS>:MAC addr: 0x%x 0x%x\n",ior( 0x14),ior( 0x15));

}

#if 0
/*
  Calculate the CRC valude of the Rx packet
  flag = 1 : return the reverse CRC (for the received packet CRC)
         0 : return the normal CRC (for Hash Table index)
*/
unsigned long cal_CRC(unsigned char * Data, unsigned int Len, u_int8 flag)
{
	
	u_int32 crc = ether_crc_le(Len, Data);

	if (flag) 
		return ~crc;
		
	return crc;
	 
}
#endif



void dm9ks_polling(tick_id_t hTick, void *pUserData)
{
//	board_info_t *db;
//	db = (board_info_t *)dm9ks_dev->user_defined_1;	

//DMFE_DEBUG("<dm9ks>:tick running!\n",0,0);
//return ;
#if 1
	if(dm9ks_Link_change == 1)
	{
//		DMFE_DEBUG("<dm9ks>:Link status change!\n",0,0);
		phy_read(0x1);
		if(phy_read(0x1) & 0x4) /*Link OK*/
		{
			/* set media speed */
			if(phy_read(0)&0x2000)
			{
				dm9ks_Speed =100;
//				DMFE_DEBUG("<dm9ks>:Linked = 100Mbps!\n",0,0);
			}
			else 
			{
				dm9ks_Speed =10;
//				DMFE_DEBUG("<dm9ks>:Linked = 10Mbps!\n",0,0);
			}
		}
		else
		{
			dm9ks_Speed =0;
//			DMFE_DEBUG("<dm9ks>:Link down!!!\n",0,0);
		}

		dm9ks_Link_change = 0;
	}

#endif

}






