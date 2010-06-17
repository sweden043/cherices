/*****************************************************************************
*
*         Copyright (c) 1993 - 2001 Accelerated Technology, Inc.
*
*  PROPRIETARY RIGHTS of Accelerated Technology are involved in the
*  subject matter of this material.  All manufacturing, reproduction,
*  use, and sales rights pertaining to this subject matter are governed
*  by the license agreement.  The recipient of this software implicitly
*  accepts the terms of the license.
*
******************************************************************************
*
*   FILE NAME                                           VERSION 
*        
*       mii.h                                             4.4   
*        
*   COMPONENT
*
*       MII - Media Independent Interface control
*
*   DESCRIPTION
*
*       This file contains the symbols that define the generic Media Independent
*       Interface (MII) management interface registers.
*
*   DATA STRUCTURES
*
*       None
*
*   DEPENDENCIES
*
*       None
*
*****************************************************************************/

#ifndef MII
#define MII

#ifdef __cplusplus
extern "C" {
#endif


#define MII_CONTROL 0

#define MII_CTRL_RESET ((unsigned short)( 0x8000 ))
#define MII_CTRL_LOOPBACK ((unsigned short)( 0x4000 ))
#define MII_CTRL_100MBPS ((unsigned short)( 0x2000 ))
#define MII_CTRL_AUTO_NEG ((unsigned short)( 0x1000 ))
#define MII_CTRL_POWER_DOWN ((unsigned short)( 0x0800 ))
#define MII_CTRL_ISOLATE ((unsigned short)( 0x0400 ))
#define MII_CTRL_RESTART ((unsigned short)( 0x0200 ))
#define MII_CTRL_FULL_DUPLEX ((unsigned short)( 0x0100 ))
#define MII_CTRL_COLL_TEST ((unsigned short)( 0x0080 ))
#define MII_CTRL_TEST_MODE_MASK ((unsigned short)( 0x0070 ))
#define MII_CTRL_MASTER_SLAVE_EN ((unsigned short)( 0x0008 ))
#define MII_CTRL_MASTER_SLAVE_VAL ((unsigned short)( 0x0004 ))
#define MII_CTRL_RESERVED1_0 ((unsigned short)( 0x0003 ))


#define MII_STATUS 1

#define MII_STAT_T4 ((unsigned short)( 0x8000 ))
#define MII_STAT_TX_FULL_DUPLEX ((unsigned short)( 0x4000 ))
#define MII_STAT_TX ((unsigned short)( 0x2000 ))
#define MII_STAT_10_FULL_DUPLEX ((unsigned short)( 0x1000 ))
#define MII_STAT_10 ((unsigned short)( 0x0800 ))
#define MII_STAT_T2_FULL_DUPLEX ((unsigned short)( 0x0400 ))
#define MII_STAT_T2 ((unsigned short)( 0x0200 ))
#define MII_STAT_RESERVED8 ((unsigned short)( 0x0100 ))
#define MII_STAT_MASTER_SLAVE_FAULT ((unsigned short)( 0x0080 ))
#define MII_STAT_PREAMB_SUPPRESS ((unsigned short)( 0x0040 ))
#define MII_STAT_AUTO_NEG_DONE ((unsigned short)( 0x0020 ))
#define MII_STAT_REMOTE_FAULT ((unsigned short)( 0x0010 ))
#define MII_STAT_AUTO_NEG ((unsigned short)( 0x0008 ))
#define MII_STAT_LINK_UP ((unsigned short)( 0x0004 ))
#define MII_STAT_JABBER_DETECT ((unsigned short)( 0x0002 ))
#define MII_STAT_EXTENDED_CAP ((unsigned short)( 0x0001 ))


#define MII_PHY_ID_1 2

#define MII_PHY_ID_1_MASK ((unsigned short)( 0xFFFF ))


#define MII_PHY_ID_2 3

#define MII_PHY_ID_2_MASK ((unsigned short)( 0xFC00 ))
#define MII_PHY_MODEL_MASK ((unsigned short)( 0x03F0 ))
#define MII_PHY_REV_MASK ((unsigned short)( 0x000F ))

#define MII_OUI(id1, id2) \
  ((((unsigned long)(id1)) << 6) | \
   ((((unsigned long)(id2)) & (MII_PHY_ID_2_MASK)) >> 10))


#define MII_ADVERTISEMENT 4

#define MII_ADVR_NEXT_PAGE ((unsigned short)( 0x8000 ))
#define MII_ADVR_RESERVED14 ((unsigned short)( 0x4000 ))
#define MII_ADVR_REMOTE_FAULT ((unsigned short)( 0x2000 ))
#define MII_ADVR_RESERVED12_11 ((unsigned short)( 0x1800 ))
#define MII_ADVR_PAUSE ((unsigned short)( 0x0400 ))
#define MII_ADVR_T4 ((unsigned short)( 0x0200 ))
#define MII_ADVR_TX_FULL_DUPLEX ((unsigned short)( 0x0100 ))
#define MII_ADVR_TX ((unsigned short)( 0x0080 ))
#define MII_ADVR_10_FULL_DUPLEX ((unsigned short)( 0x0040 ))
#define MII_ADVR_10 ((unsigned short)( 0x0020 ))
#define MII_ADVR_SELECTOR_MASK ((unsigned short)( 0x001F ))
#define MII_ADVR_802_3 ((unsigned short)( 0x0001 ))
#define MII_ADVR_802_3_ISLAN_16T ((unsigned short)( 0x0002 ))


#define MII_LINK_PARTNER 5

#define MII_LINK_NEXT_PAGE ((unsigned short)( 0x8000 ))
#define MII_LINK_ACK ((unsigned short)( 0x4000 ))
#define MII_LINK_REMOTE_FAULT ((unsigned short)( 0x2000 ))
#define MII_LINK_RESERVED12_11 ((unsigned short)( 0x1800 ))
#define MII_LINK_PAUSE ((unsigned short)( 0x0400 ))
#define MII_LINK_T4 ((unsigned short)( 0x0200 ))
#define MII_LINK_TX_FULL_DUPLEX ((unsigned short)( 0x0100 ))
#define MII_LINK_TX ((unsigned short)( 0x0080 ))
#define MII_LINK_10_FULL_DUPLEX ((unsigned short)( 0x0040 ))
#define MII_LINK_10 ((unsigned short)( 0x0020 ))
#define MII_LINK_SELECTOR_MASK ((unsigned short)( 0x001F ))
#define MII_LINK_802_3 ((unsigned short)( 0x0001 ))
#define MII_LINK_802_3_ISLAN_16T ((unsigned short)( 0x0002 ))


#define MII_EXPANSION 6

#define MII_EXP_RESERVED15_5 ((unsigned short)( 0xFFE0 ))
#define MII_EXP_PARALLEL_FAULT ((unsigned short)( 0x0010 ))
#define MII_EXP_LINK_NEXT_PAGE ((unsigned short)( 0x0008 ))
#define MII_EXP_NEXT_PAGE_ABLE ((unsigned short)( 0x0004 ))
#define MII_EXP_PAGE_RECEIVED ((unsigned short)( 0x0002 ))
#define MII_EXP_LINK_AUTO_NEG ((unsigned short)( 0x0001 ))


#define MII_NEXT_PAGE 7


#define MII_MAX_PHY 32
#define MII_MAX_REGS 32

typedef STATUS (*mii_ReadMII)(DV_DEVICE_ENTRY* deviceP, int phyAddr,
                              int regAddr, unsigned short* inP);

typedef STATUS (*mii_WriteMII)(DV_DEVICE_ENTRY* deviceP, int phyAddr,
                               int regAddr, unsigned short out);


STATUS MII_AutoNeg(DV_DEVICE_ENTRY* deviceP, int phyAddr,
                   unsigned long retries, int* isFullDuplexP, int* is100MbpsP,
                   mii_ReadMII miiRead, mii_WriteMII miiWrite);

  
#ifdef __cplusplus
}
#endif

#endif /* MII */

