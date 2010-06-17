/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        PCI_SEND.H
 *
 *
 * Description:     Header file for pci driver - exported sender declarations.
 *
 *
 * Author:          Carroll Vance
 *
 ****************************************************************************/
/* $Header: pci_send.h, 4, 9/3/02 7:40:02 PM, Matt Korte$
 ****************************************************************************/
#ifndef __PCI_SEND_H__
#define __PCI_SEND_H__

#include "basetype.h"

/* Used to disable CM code load */
#define DISABLE_CM_CODE_LOAD                  0
/* Used to disable Cm boot load */
#define DISABLE_CM_BOOT_LOAD                  0
/* PCI_send return values */
#define PCI_SEND_OK                           0
#define PCI_ERR_TRANSFER                    (-1) 
#define PCI_ERR_BAD_ARG                     (-2)
#define PCI_ERR_RECEIVER_FULL               (-3)  
#define PCI_ERR_SEMTO                       (-4)
#define PCI_ERR_NOT_INITIALIZED             (-5)
#define PCI_ERR_INTERNAL                    (-6)
#define PCI_ERR_RECEIVER_BUFF_OVERFLOW      (-7)
#define PCI_ERR_CM_RESET                    (-8)

typedef enum { PCI_OK = 0, PCI_ERROR = 1} PCI_STATUS;

extern int        PCI_send(u_int8 *pData, u_int32 usLength, u_int16 usType);
extern PCI_STATUS PCI_driverInit(u_int8 *CmCode, u_int32 CmCodeLen, u_int8 *CmBootData, u_int32 CmBootDataLen);


#endif /* __PCI_SEND_H__ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         9/3/02 7:40:02 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  3    mpeg      1.2         8/22/02 6:43:38 PM     Larry Wang      SCR(s) 
 *        4460 :
 *        Remove DISABLE_CM_CA_CERT_LOAD flag.
 *        
 *  2    mpeg      1.1         4/22/02 3:58:12 PM     Dave Moore      SCR(s) 
 *        3587 :
 *        added support for new message types.
 *        
 *        
 *  1    mpeg      1.0         4/3/02 1:00:24 AM      Carroll Vance   
 * $
 * 
 *    Rev 1.3   03 Sep 2002 18:40:02   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 ****************************************************************************/
