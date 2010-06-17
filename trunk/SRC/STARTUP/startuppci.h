/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       cnxtpci.h                                                */
/*                                                                          */
/* Description:    PCI definitions.                                         */
/*                                                                          */
/* Author:         Tim Ross                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _STARTUPPCI_H
#define _STARTUPPCI_H

/******************/
/* Include Files  */
/******************/

/***********************/
/* Function Prototypes */
/***********************/
void InitPCI( void );

/**********************/
/* Local Definitions  */
/**********************/
typedef struct _PCI_CONFIG
{
    u_int32 ID;
    u_int8  IOAccess;
    u_int8  MemoryAccess;
    u_int16 Latency_Cache;
    u_int32 SizeOnBar[6];
    u_int8  BusMastering;
} PCI_CONFIG;
typedef PCI_CONFIG *LPPCI_CONFIG;

#endif
