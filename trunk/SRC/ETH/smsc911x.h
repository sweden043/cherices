/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                 				*/
/*                       SOFTWARE FILE/MODULE HEADER                        					*/
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          				*/
/*                              Austin, TX                                  								*/
/*                         All Rights Reserved                              							*/
/****************************************************************************/
/*
 * Filename:        smsc911x.h
 *
 *
 * Description:     SMSC LAN911X Ethernet Driver  Header File
 *
 *
 * Author:          Sam Chen
 *
 ****************************************************************************/
/* $Id:smsc911x.h,v 1.0, 3/29/2005 16:00:00 PM  Sam Chen
 ****************************************************************************/
#ifndef _SMSC911X_H_
#define _SMSC911X_H_

/******************/
/* Include Files  */
/******************/

#include "basetype.h"
#include "kal.h"
#include "nucleus.h"
#include "eth.h"


/******************/
/* Marco defination */
/******************/
#define BIT0    0x00000001
#define BIT1    0x00000002
#define BIT2    0x00000004
#define BIT3    0x00000008
#define BIT4    0x00000010
#define BIT5    0x00000020
#define BIT6    0x00000040
#define BIT7    0x00000080
#define BIT8    0x00000100
#define BIT9    0x00000200
#define BIT10   0x00000400
#define BIT11   0x00000800
#define BIT12   0x00001000
#define BIT13   0x00002000
#define BIT14   0x00004000
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000


/* Register Bits */

/* TX DATA FIFO CMD 'A'*/
#define		TX_CMD_A_ON_COMP_				(BIT31)
#define		TX_CMD_A_BUF_END_ALGN_			(BIT26|BIT25)
#define		TX_CMD_A_DATA_OFFSET_			(0x1F<<16)
#define		TX_CMD_A_FIRST_SEG_				BIT13
#define		TX_CMD_A_LAST_SEG_				BIT12
#define		TX_CMD_A_BUF_SIZE_				(0x3FF)

#define		TX_CMD_A_BUG_END_ALGN_4B		0
#define		TX_CMD_A_BUG_END_ALGN_8B		1
#define		TX_CMD_A_BUG_END_ALGN_16B		2

/* TX DATA FIFO CMD 'B'*/
#define		TX_CMD_B_PKT_TAG_				(0xFFFF<<16)
#define		TX_CMD_B_ADD_CRC_DISABLE_		BIT13
#define		TX_CMD_B_DISABLE_PADDING_		BIT12
#define		TX_CMD_B_PKT_BYTE_LENGTH_		(0x3FF)

/* TX STATUS FIFO */
#define		TX_STS_PKT_TAG_					(0xFFFF<<16)
#define		TX_STS_ES_							BIT15
#define		TX_STS_LOC_						BIT11
#define		TX_STS_NOC_						BIT10
#define		TX_STS_LC_							BIT9
#define		TX_STS_EC_							BIT8	
#define		TX_STS_CCOUNT_					(0xF<<3)	
#define		TX_STS_ED_							BIT2	
#define		TX_STS_UE_							BIT1	
#define		TX_STS_DF_							BIT0	

/* RX STATUS FIFO */
#define		RX_STS_FLT_FAIL_					BIT30
#define		RX_STS_PKT_LEN_					(0x3FFF<<16)
#define		RX_STS_ES_							BIT15
#define		RX_STS_BRC_FRM_					BIT13
#define		RX_STS_LE_							BIT12
#define		RX_STS_RUNT_FRM_					BIT11
#define		RX_STS_MLT_FRM_					BIT10
#define		RX_STS_FRM_TOO_LONG				BIT7
#define		RX_STS_COLLISION					BIT6
#define		RX_STS_FRM_TYPE					BIT5
#define		RX_STS_RCM_TM						BIT4
#define		RX_STS_MII_ERR						BIT3
#define		RX_STS_DRB_BIT						BIT2
#define		RX_STS_CRC_ERR					BIT1


/* ID_REV*/
#define		ID_REV_CHIP_ID_					(0xFFFF<<16)	// RO	default 0x0118
#define		ID_REV_REV_ID_						(0xFFFF)			// RO

/* IRQ_CFG				(0x54)*/
#define		IRQ_CFG_INT_DEAS_					(0xFF<<24)	// R/W
#define     	IRQ_CFG_INT_DEAS_CLR_			BIT14  		// SC
#define     	IRQ_CFG_INT_DEAS_STS_			BIT13  		// SC
#define		IRQ_CFG_IRQ_INT_					BIT12		// RO
#define		IRQ_CFG_IRQ_EN_					BIT8			// R/W
#define		IRQ_CFG_IRQ_POL_					BIT4			// R/W Not Affected by SW Reset
#define		IRQ_CFG_IRQ_TYPE_					BIT0			// R/W Not Affected by SW Reset

/* INT_STS				(0x58)*/
#define		INT_STS_SW_INT_					BIT31	// R/WC
#define		INT_STS_TXSTOP_INT_				BIT25	// R/WC
#define		INT_STS_RXSTOP_INT_				BIT24	// R/WC
#define		INT_STS_RXDFH_INT_				BIT23	// R/WC
#define		INT_STS_TX_IOC_					BIT21	// R/WC
#define		INT_STS_RXD_INT_					BIT20	// R/WC
#define		INT_STS_GPT_INT_					BIT19	// R/WC
#define		INT_STS_PHY_INT_					BIT18	// RO
#define		INT_STS_PME_INT_					BIT17	// R/WC
#define		INT_STS_TXSO_						BIT16	// R/WC
#define		INT_STS_RWT_						BIT15	// R/WC
#define		INT_STS_RXE_						BIT14	// R/WC
#define		INT_STS_TXE_						BIT13	// R/WC
#define		INT_STS_TDFU_						BIT11	// R/WC
#define		INT_STS_TDFO_						BIT10	// R/WC
#define		INT_STS_TDFA_						BIT9		// R/WC
#define		INT_STS_TSFF_						BIT8		// R/WC
#define		INT_STS_TSFL_						BIT7		// R/WC
#define		INT_STS_RXDF_						BIT6		// R/WC
#define		INT_STS_RDFL_						BIT5		// R/WC
#define		INT_STS_RSFF_						BIT4		// R/WC
#define		INT_STS_RSFL_						BIT3		// R/WC
#define		INT_STS_GPIO2_INT_				BIT2		// R/WC
#define		INT_STS_GPIO1_INT_				BIT1		// R/WC
#define		INT_STS_GPIO0_INT_				BIT0		// R/WC

/* INT_EN				(0x5C)*/
#define		INT_EN_SW_INT_EN_				BIT31	// R/W
#define		INT_EN_TXSTOP_INT_EN_			BIT25	// R/W
#define		INT_EN_RXSTOP_INT_EN_			BIT24	// R/W
#define		INT_EN_RXDFH_INT_EN_				BIT23	// R/W
#define		INT_EN_TIOC_INT_EN_				BIT21	// R/W
#define		INT_EN_RXD_INT_EN_				BIT20	// R/W
#define		INT_EN_GPT_INT_EN_				BIT19	// R/W
#define		INT_EN_PHY_INT_EN_				BIT18	// R/W
#define		INT_EN_PME_INT_EN_				BIT17	// R/W
#define		INT_EN_TXSO_EN_					BIT16	// R/W
#define		INT_EN_RWT_EN_					BIT15	// R/W
#define		INT_EN_RXE_EN_					BIT14	// R/W
#define		INT_EN_TXE_EN_					BIT13	// R/W
#define		INT_EN_TDFU_EN_					BIT11	// R/W
#define		INT_EN_TDFO_EN_					BIT10	// R/W
#define		INT_EN_TDFA_EN_					BIT9		// R/W
#define		INT_EN_TSFF_EN_					BIT8		// R/W
#define		INT_EN_TSFL_EN_					BIT7		// R/W
#define		INT_EN_RXDF_EN_					BIT6		// R/W
#define		INT_EN_RDFL_EN_					BIT5		// R/W
#define		INT_EN_RSFF_EN_					BIT4		// R/W
#define		INT_EN_RSFL_EN_					BIT3		// R/W
#define		INT_EN_GPIO2_INT_					BIT2		// R/W
#define		INT_EN_GPIO1_INT_					BIT1		// R/W
#define		INT_EN_GPIO0_INT_					BIT0		// R/W

/* BYTE_TEST				(0x64)*/

/* FIFO_INT				(0x68)*/
#define		FIFO_INT_TX_AVAIL_LEVEL_		(0xFF<<24)	// R/W
#define		FIFO_INT_TX_STS_LEVEL_		(0xFF<<16)	// R/W
#define		FIFO_INT_RX_AVAIL_LEVEL_		(0xFF<<8)	// R/W
#define		FIFO_INT_RX_STS_LEVEL_		(0xFF)		// R/W

/* RX_CFG					(0x6C)*/
#define		RX_CFG_RX_END_ALGN_			(BIT30|BIT31)		// R/W
#define		RX_CFG_RX_DMA_CNT_			(0x0FFF<<16)	// R/W
#define		RX_CFG_RX_DUMP_				BIT15			// R/W
#define		RX_CFG_RXDOFF_				(0x1F<<8)	// R/W

#define		RX_CFG_RX_END_ALGN_4B		0
#define		RX_CFG_RX_END_ALGN_16B		1
#define		RX_CFG_RX_END_ALGN_32B		2


/* TX_CFG					(0x70)*/
#define		TX_CFG_TXS_DUMP_				BIT15	// Self Clearing
#define		TX_CFG_TXD_DUMP_				BIT14	// Self Clearing
#define		TX_CFG_TXSAO_					BIT2		// R/W
#define		TX_CFG_TX_ON_					BIT1		// R/W
#define		TX_CFG_STOP_TX_				BIT0		// Self Clearing

/* HW_CFG					(0x74)*/
#define		HW_CFG_TTM_					BIT21		// R/W
#define		HW_CFG_SF_					BIT20		// R/W
#define		HW_CFG_TX_FIF_SZ_			(0xF<<16)	// R/W
#define		HW_CFG_TR_					(BIT12|BIT13)	// R/W

#define          HW_CFG_EXT_PHY_EN_		(BIT2)  // R/W
#define          HW_CFG_SMI_SEL_			      (BIT4)  // R/W
#define          HW_CFG_EXT_PHY_DET_		(BIT3)  // RO  //only available on 115/117
#define		HW_CFG_32_16_BIT_MODE_		(BIT2)			// RO  //only available on 116/118
#define     	HW_CFG_SRST_TO_				(BIT1)  		// RO  //only available on 115/117
#define		HW_CFG_SRST_					(BIT0)			// Self Clearing
#define          HW_CFG_PHY_CLK_SEL_        (BIT5 | BIT6)
#define          HW_CFG_PHY_CLK_SEL_INT_PHY_	(0x0UL) // R/W
#define          HW_CFG_PHY_CLK_SEL_EXT_PHY_	(0x20UL) // R/W
#define          HW_CFG_PHY_CLK_SEL_CLK_DIS_ (0x40UL) // R/W





/* RX_DP_CTRL				(0x78)*/
#define		RX_DP_CTRL_RX_FFWD_			BIT31		// RO

/* RX_FIFO_INF				(0x7C)*/
#define		RX_FIFO_INF_RXSUSED_			(0xFF<<16)	// RO
#define		RX_FIFO_INF_RXDUSED_			(0xFFFF)		// RO

/* TX_FIFO_INF				(0x80)*/
#define		TX_FIFO_INF_TSUSED_			(0xFF<<16)  // RO
#define		TX_FIFO_INF_TDFREE_			(0xFFFF)		// RO

/* PMT_CTRL				(0x84)*/
#define		PMT_CTRL_PM_MODE_		(BIT12|BIT13)	// Self Clearing
#define		PMT_CTRL_PHY_RST_			BIT10		// Self Clearing
#define		PMT_CTRL_WOL_EN_			BIT9			// R/W
#define		PMT_CTRL_ED_EN_			BIT8			// R/W
#define		PMT_CTRL_PME_TYPE_		BIT6			// R/W Not Affected by SW Reset
#define		PMT_CTRL_WUPS_			(BIT5|BIT4)	// R/WC
#define		PMT_CTRL_PME_IND_			BIT3			// R/W
#define		PMT_CTRL_PME_POL_			BIT2			// R/W
#define		PMT_CTRL_PME_EN_			BIT1			// R/W Not Affected by SW Reset
#define		PMT_CTRL_READY_			BIT0			// RO

#define	       PMT_CTRL_PM_MODE_D0_	(0x0)  		// Self Clearing
#define         	PMT_CTRL_PM_MODE_D1_	(0x1<<12)	// Self Clearing
#define         	PMT_CTRL_PM_MODE_D2_	(0x2<<12)  	// Self Clearing

#define		PMT_CTRL_WUPS_NOWAKE_	(0x0)		// R/WC
#define		PMT_CTRL_WUPS_ED_		BIT4			// R/WC
#define		PMT_CTRL_WUPS_WOL_		BIT5			// R/WC
#define		PMT_CTRL_WUPS_MULTI_		(BIT4|BIT5)	// R/WC


/* GPIO_CFG				(0x88)*/
#define		GPIO_CFG_LED3_EN_			BIT30		// R/W
#define		GPIO_CFG_LED2_EN_			BIT29		// R/W
#define		GPIO_CFG_LED1_EN_			BIT28		// R/W
#define		GPIO_CFG_GPIO2_INT_POL_	BIT26		// R/W
#define		GPIO_CFG_GPIO1_INT_POL_	BIT25		// R/W
#define		GPIO_CFG_GPIO0_INT_POL_	BIT24		// R/W
#define		GPIO_CFG_EEPR_EN_			(0x00700000)	// R/W
#define		GPIO_CFG_GPIOBUF2_		BIT22		// R/W
#define		GPIO_CFG_GPIOBUF1_		BIT21		// R/W
#define		GPIO_CFG_GPIOBUF0_		BIT20		// R/W
#define		GPIO_CFG_GPIODIR2_		BIT10		// R/W
#define		GPIO_CFG_GPIODIR1_		BIT9			// R/W
#define		GPIO_CFG_GPIODIR0_		BIT8			// R/W
#define		GPIO_CFG_GPIOD4_			BIT5			// R/W
#define		GPIO_CFG_GPIOD3_			BIT4			// R/W
#define		GPIO_CFG_GPIOD2_			BIT2			// R/W
#define		GPIO_CFG_GPIOD1_			BIT1			// R/W
#define		GPIO_CFG_GPIOD0_			BIT0			// R/W

/* GPT_CFG					(0x8C)*/
#define		GPT_CFG_TIMER_EN_			BIT29		// R/W
#define		GPT_CFG_GPT_LOAD_		(0xFFFF)		// R/W

/* GPT_CNT					(0x90)*/
#define		GPT_CNT_GPT_CNT_			(0xFFFF)		// RO

/* MAC_CSR_CMD				(0xA4)*/
#define		MAC_CSR_CMD_CSR_BUSY_	BIT31		// Self Clearing
#define		MAC_CSR_CMD_R_NOT_W_	BIT30		// R/W
#define		MAC_CSR_CMD_CSR_ADDR_	(0xFF)		// R/W

/* AFC_CFG					(0xAC)*/
#define		AFC_CFG_AFC_HI_			(0xFF0000)	// R/W
#define		AFC_CFG_AFC_LO_			(0x00FF00)	// R/W
#define		AFC_CFG_BACK_DUR_		(0x0000F0)	// R/W
#define		AFC_CFG_FCMULT_			(0x000008)	// R/W
#define		AFC_CFG_FCBRD_			BIT2			// R/W
#define		AFC_CFG_FCADD_			BIT1			// R/W
#define		AFC_CFG_FCANY_			BIT0			// R/W

/* E2P_CMD					(0xB0)*/
#define		E2P_CMD_EPC_BUSY_		BIT31	// Self Clearing
#define		E2P_CMD_EPC_CMD_			(BIT30|BIT29|BIT28)	// R/W
#define		E2P_CMD_EPC_CMD_READ_	(0x00)	// R/W
#define		E2P_CMD_EPC_CMD_EWDS_	BIT28		// R/W
#define		E2P_CMD_EPC_CMD_EWEN_	BIT29		// R/W
#define		E2P_CMD_EPC_CMD_WRITE_	(BIT29|BIT28)// R/W
#define		E2P_CMD_EPC_CMD_WRAL_	BIT30		// R/W
#define		E2P_CMD_EPC_CMD_ERASE_	(BIT30|BIT28)// R/W
#define		E2P_CMD_EPC_CMD_ERAL_	(BIT30|BIT29)// R/W
#define		E2P_CMD_EPC_CMD_RELOAD_	(0x70000000) // R/W
#define		E2P_CMD_EPC_TIMEOUT_		BIT9			// R
#define		E2P_CMD_MAC_ADDR_LOADED_	BIT8		// RO
#define		E2P_CMD_EPC_ADDR_		(0x000000FF)	// R/W

/* E2P_DATA				(0xB4)*/
#define		E2P_DATA_EEPROM_DATA_	(0x000000FF)	// R/W


/* MAC Register Bit defination */

/* MAC_CR - MAC Control Register */
#define MAC_CR_RXALL_					BIT31
#define MAC_CR_HBDIS_					BIT28
#define MAC_CR_RCVOWN_				BIT23
#define MAC_CR_LOOPBK_					BIT21
#define MAC_CR_FDPX_					BIT20
#define MAC_CR_MCPAS_					BIT19
#define MAC_CR_PRMS_					BIT18
#define MAC_CR_INVFILT_					BIT17
#define MAC_CR_PASSBAD_				BIT16
#define MAC_CR_HFILT_					BIT13
#define MAC_CR_HPFILT_					BIT13
#define MAC_CR_LCOLL_					BIT12
#define MAC_CR_BCAST_					BIT11
#define MAC_CR_DISRTY_					BIT10
#define MAC_CR_PADSTR_					BIT8
#define MAC_CR_BOLMT_MASK_			(BIT6 | BIT7)
#define MAC_CR_DFCHK_					BIT5
#define MAC_CR_TXEN_					BIT3
#define MAC_CR_RXEN_					BIT2



#define MII_ACC_PHY_ADDR_				(0x1F<<11)
#define MII_ACC_MIIRINDA_				(0x1F<<6)
#define MII_ACC_MII_WRITE_				BIT1
#define MII_ACC_MII_BUSY_				BIT0


#define FLOW_FCPT_						0xFFFF0000
#define FLOW_FCPASS_					BIT2
#define FLOW_FCEN_						BIT1
#define FLOW_FCBSY_					BIT0

#define WUCSR_GUE_						BIT9
#define WUCSR_WUFR_					BIT6
#define WUCSR_MPR_						BIT5
#define WUCSR_WAKE_EN_				BIT2
#define WUCSR_MPEN_					BIT1

/*PHY Register Bits defination */
#define PHY_BCR_RESET_					BIT15
#define PHY_BCR_SPEED_SELECT_			BIT13
#define PHY_BCR_AUTO_NEG_ENABLE_		BIT12
#define PHY_BCR_RESTART_AUTO_NEG_	BIT9
#define PHY_BCR_DUPLEX_MODE_			BIT8

#define PHY_BSR_LINK_STATUS_			BIT2
#define PHY_BSR_REMOTE_FAULT_			BIT4
#define PHY_BSR_AUTO_NEG_COMP_		BIT5


#define PHY_ANEG_ADV_PAUSE_ 			(BIT11|BIT10)

#define PHY_ANEG_ADV_ASYMP_			(0x2<<10)
#define PHY_ANEG_ADV_SYMP_			(0x1<<10)
#define PHY_ANEG_ADV_NOP				(0x0<<10)

#define PHY_ANEG_ADV_10H_				BIT5
#define PHY_ANEG_ADV_10F_				BIT6
#define PHY_ANEG_ADV_100H_			BIT7
#define PHY_ANEG_ADV_100F_			BIT8

#define PHY_ANEG_ADV_SPEED_			(0x1E<<4)


#define PHY_ANEG_LPA_ASYMP_			0x0800
#define PHY_ANEG_LPA_SYMP_			0x0400
#define PHY_ANEG_LPA_100FDX_			0x0100
#define PHY_ANEG_LPA_100HDX_			0x0080
#define PHY_ANEG_LPA_10FDX_			0x0040
#define PHY_ANEG_LPA_10HDX_			0x0020

#define MODE_CTRL_STS_EDPWRDOWN_	0x2000
#define MODE_CTRL_STS_ENERGYON_		0x0002

#define PHY_INT_SRC_ENERGY_ON_		0x0080
#define PHY_INT_SRC_ANEG_COMP_		0x0040
#define PHY_INT_SRC_REMOTE_FAULT_	0x0020
#define PHY_INT_SRC_LINK_DOWN_		0x0010


#define PHY_INT_MASK_ENERGY_ON_		0x0080
#define PHY_INT_MASK_ANEG_COMP_		0x0040
#define PHY_INT_MASK_REMOTE_FAULT_	0x0020
#define PHY_INT_MASK_LINK_DOWN_		0x0010


#define PHY_SPECIAL_SPD_				0x001C
#define PHY_SPECIAL_SPD_10HALF_		0x0004
#define PHY_SPECIAL_SPD_10FULL_		0x0014
#define PHY_SPECIAL_SPD_100HALF_		0x0008
#define PHY_SPECIAL_SPD_100FULL_		0x0018


//end of lan register bit definitions

#define  MAX_EXT_PHY                      0x5


/******************/
/* Types Defination */
/******************/


typedef enum _smsc_port
{
	RX_DATA_FIFO=0,
	TX_DATA_FIFO=0x20,
	RX_STATUS_FIFO_PORT=0x40,
	RX_STATUS_FIFO_PEEK=0x44,
	TX_STATUS_FIFO_PORT=0x48,
	TX_STATUS_FIFO_PEEK=0x4c	
}SMSC_PORT;

typedef enum _smsc_reg
{    	
	RX_DFIFO=0,
	TX_DFIFO=0x20,
	RX_SFIFO_PORT=0x40,
	RX_SFIFO_PEEK=0x44,
	TX_SFIFO_PORT=0x48,
	TX_SFIFO_PEEK=0x4c,		
   	ID_REV=0x50,
	IRQ_CFG=0x54,
	INT_STS=0x58,
	INT_EN=0x5c,
	BYTE_TEST=0x64,
	FIFO_INT=0x68,
	RX_CFG=0x6c,
	TX_CFG=0x70,
	HW_CFG=0x74,
	RX_DP_CTRL=0x78,
	RX_FIFO_INF=0x7c,
	TX_FIFO_INF=0x80,
	PMT_CTRL=0x84,
	GPIO_CFG=0x88,
	GPT_CNT=0x90,
	ENDIAN=0x98,
	FREE_RUN=0x9c,
	RX_DROP=0xa0,
	MAC_CSR_CMD=0xa4,
	MAC_CSR_DATA=0xa8,
	AFC_CFG=0xac,
	E2P_CMD=0xb0,
	E2P_DATA=0xb4,
	IO_ZERO=0x0,
	IO_FFFF=0xFE
}SMSC_REG;

typedef enum _mac_reg
{
    MAC_CR=1,
    ADDRH,
    ADDRL,
    HASHH,
    HASHL,
    MII_ACC,
    MII_DATA,
    FLOW,
    VLAN1,
    VLAN2,
    WUFF,
    WUCSR
}MAC_REG;


typedef enum _phy_reg
{
    PHY_BCR=0,
    PHY_BSR,
    PHY_ID1,
    PHY_ID2,
    PHY_ANEG_ADV,
    PHY_ANEG_LPA,
    PHY_ANER_ANE,
    PHY_MCSR=17,
    PHY_SMR,
    PHY_SCSI=27,
    PHY_ISR=29,
    PHY_IMR,
    PHY_SCSR    
}PHY_REG;

typedef enum _link_status
{
    LINK_OFF=0x0,
    LINK_SPEED_10HD=0x1,
    LINK_SPEED_10FD=0x2,
    LINK_SPEED_100HD=0x4,
    LINK_SPEED_100FD=0x8,
    LINK_SYMMETRIC_PAUSE=0x10,
    LINK_ASYMMETRIC_PAUSE=0x20,
    LINK_AUTO_NEGOTIATE=0x40
}LINK_STATUS;



typedef struct _SMSC_PRIV {
	
	u_int32 dwInstanceIndex;

	ETH_STATS stats;

	sem_id_t MacPhyAccessLock;
	sem_id_t ResetLock;
     u_int16 PhyAddessId;   

	u_int32 dwLinkSpeed;			/* the current link status */	
	u_int32 dwLinkSettings;		/* the link setting by manual or automatic. */

	u_int32 dma_tx_channel;
	u_int32 dma_rx_channel;
	sem_id_t dma_rx_sem;
	sem_id_t dma_tx_sem;
	
	queue_id_t msg_id;
	task_id_t   smsc_task_id;	
	tick_id_t 	  polling_tick_id;	

	events_t	event_rsfl;
	events_t	event_reset;

	u_int32 rx_isr_count;
	u_int32 rx_task_count;

	bool rx_congested;
	bool tx_congested;

}smsc_priv_t;	



/***********************/
/* Function prototypes */
/***********************/
u_int32 smsc_read(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg);
void smsc_write(struct _DV_DEVICE_ENTRY *dev,SMSC_REG reg,u_int32 data);
STATUS smsc_open(struct _DV_DEVICE_ENTRY *dev);
STATUS smsc_Tx_hard_xmit(struct _DV_DEVICE_ENTRY *dev,NET_BUFFER *buf );
STATUS smsc_init( struct _DV_DEVICE_ENTRY *dev );
STATUS smsc_ioctl( DV_DEVICE_ENTRY *device, int option, DV_REQ *request );
u_int32 smsc_phy_read_reg(struct _DV_DEVICE_ENTRY *dev, PHY_REG reg);
void smsc_phy_write_reg(struct _DV_DEVICE_ENTRY *dev, PHY_REG reg,u_int32 dwVal);
u_int32 smsc_mac_read_reg(struct _DV_DEVICE_ENTRY *dev, MAC_REG reg);
void smsc_mac_write_reg(struct _DV_DEVICE_ENTRY *dev, MAC_REG reg,u_int32 dwVal);
STATUS smsc_Rx_process_status(struct _DV_DEVICE_ENTRY *dev,u_int32 *dwFrameLen);
u_int8 smsc_e2prom_ops(struct _DV_DEVICE_ENTRY *dev,u_int32 type,u_int8 addr, u_int8 data);
#endif  




/****************************************************************************
 * Modifications:
 * $Log:
 *       
 *  1    Ethernet      1.0         3/29/2005 16:00:00 PM  Sam Chen
 * $
 ****************************************************************************/
 
