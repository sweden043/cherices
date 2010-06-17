/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       pcidata.c                                                 */
/*                                                                          */
/* Description:    Board hardware description tables.                       */
/*                                                                          */
/* Author:         Ray Mack                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "startuppci.h"

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

/*****************************************************************************/
/* PCI Devices                                                               */
/* The list below contains all of the POSSIBLE PCI devices that can be found */
/* when the PCI bus is scanned. Since the device will not be configured if   */
/* it is not found on the bus, these do NOT need to be specified for each    */
/* board.                                                                    */
/*****************************************************************************/
PCI_CONFIG PCIDevices[] =
{
   {
      0xC8611045,         // usb/PCI CARD
      0,                  // No I/O Accesses
      1,                  // Does Memory Accesses
      0x2004,             // Latency 32 clocks, Cache = 4 DWORDS
      {
         0x1000,          // BAR0 - required for 4Kb Register Address space.
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing
         0                // BAR5 - nothing
      },
      1                   // Bus Mastering
   },                         

   {
      0x201514F1,         // Conexant 2015 PCI SoftK56 Modem (RS56/SP-PCI, R6793-11, 12767.2)
      0,                  // Does not do I/O Accesses
      1,                  // Does Memory Accesses.
      0x2000,             // Latency 32 clocks, Cache = None
      {
         0x10000,         // BAR0 - Needs 64k bytes MEM space for BASIC 2.10
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },                  // Bus Mastering
      1
   },

   {
      0x103314F1,         // Conexant 1033 HCF PCI Modem (RLDL56PDF, R6785-82P w/ 11229-12)
      0,                  // Does not do I/O Accesses
      1,                  // Does Memory Accesses.
      0x2000,             // Latency 32 clocks, Cache = None
      {
         0x10000,         // BAR0 - Needs 64k bytes MEM space for BASIC 2.10
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },
      0                   // Bus Mastering
   },                          

   {
      0x1002127a,         // Generic Rockwell RS56-PCI Modem
      0,                  // No I/O Accesses.
      1,                  // Does Memory Accesses.
      0x2004,             // Latency 32 clocks, Cache = 4 DWORDS
      {
         0x10000,         // BAR0 - 64K Bus Memory
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },
      0                   // Bus Mastering
   },                          

   {
      0x1003127a,         // Generic Rockwell RS56-PCI Modem
      0,                  // No I/O Accesses.
      1,                  // Does Memory Accesses.
      0x2004,             // Latency 32 clocks, Cache = 4 DWORDS
      {
         0x10000,         // BAR0 - 64K Bus Memory
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },
      0                   // Bus Mastering
   },                          

   {
      0x1005127a,         // Generic Rockwell RS56-PCI Modem
      0,                  // No I/O Accesses.
      1,                  // Does Memory Accesses.
      0x2004,             // Latency 32 clocks, Cache = 4 DWORDS
      {
         0x10000,         // BAR0 - 64K Bus Memory
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },
      0                   // Bus Mastering
   },                          

   {
      0x5F6e14F1,         // CX24430/CX24431 IB Modem
      0,                  // No I/O Accesses.
      1,                  // Does Memory Accesses.
      0x6004,             // Latency ?, no cache
      {
         0x2000000,       // BAR0 - Needs 32MB of memory space
         0,               // BAR1 - nothing.
         0,               // BAR2 - nothing.
         0,               // BAR3 - nothing.
         0,               // BAR4 - nothing.
         0                // BAR5 - nothing.
      },
      1                   // Bus Mastering
   },                          
};


/********************************************************************/
/* NOTE:                                                            */
/* In addition to the above devices, the PCI-PCCard bridge uses the */
/* following resources:                                             */
/*                                                                  */
/* Memory Space:                                                    */
/*       4KB @ 0x40000000 for CardBus/ExCA control registers        */
/*                                                                  */
/* I/O Space:                                                       */
/*       4 bytes @ 0xfeec for PCCard ExCA control index/data regs.  */
/********************************************************************/
int iNumPCIDevices = sizeof(PCIDevices)/sizeof(PCI_CONFIG);

#endif

