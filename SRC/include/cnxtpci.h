/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       cnxtpci.h                                                */
/*                                                                          */
/* Description:    PCI API declarations                                     */
/*                                                                          */
/* Author:         Miles Bintz                                              */
/*                                                                          */
/* Copyright Conexant Systems, 2001                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _CNXTPCI_H
#define _CNXTPCI_H

typedef struct _PCI_CONFIG_BLOCK {
    u_int32 ID;
    u_int16 status;
    u_int16 command;
    u_int32 class; u_int8  revision;
    u_int8  bist, header, latency_timer, cache_line_size;
    u_int32 mem_base;
    u_int32 io_base;
    u_int32 mem_mapped_io;
    u_int16 subsys_id, subsys_vendor_id;
    u_int8  max_at, min_gnt, int_pin, int_line;
    u_int8  base1_enable, base2_base0_size;
    u_int8  base2_remap;
    u_int8  enable_TRDY;
} PCI_CONFIG_BLOCK;
typedef PCI_CONFIG_BLOCK* LPPCI_CONFIG_BLOCK;

/***********************/
/* Function Prototypes */
/***********************/
typedef enum {CNXT_PCI_STS_SUCCESS, CNXT_PCI_STS_FAILURE, CNXT_PCI_STS_NOT_FOUND} CNXT_PCI_STATUS;

/*********************************************/
/*  User provides the vendor ID and device   */
/*  ID and the HW will scan the PCI bus      */
/*  and return the bus and device number.    */
/*********************************************/
CNXT_PCI_STATUS cnxt_pci_get_devnum_from_id(u_int16 vendor_id, u_int16 dev_id, u_int8* busnumber, u_int8* devnumber);

/*********************************************/
/* After the user knows the bus and device   */
/* numbers they can query the device for     */
/* its configuration data.                   */
/*********************************************/
CNXT_PCI_STATUS cnxt_pci_get_cfg(u_int8 bus, u_int8 dev, LPPCI_CONFIG_BLOCK config);

/*********************************************/
/*   After the user knows the bus and device */
/*   numbers they can set the device's       */
/*   configuration data.                     */
/*********************************************/
CNXT_PCI_STATUS cnxt_pci_set_cfg(u_int8 bus, u_int8 dev, LPPCI_CONFIG_BLOCK config);

#endif
