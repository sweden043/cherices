/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       romdata.c                                                 */
/*                                                                          */
/* Description:    Board hardware description tables.                       */
/*                                                                          */
/* Author:         Ray Mack                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/***************************************************************************
$Header: romdata.c, 29, 5/30/04 8:19:27 PM, Steven Shen$
****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "startup.h"


const u_int16 AMD16MegBootBlockSectors2[] =
{
    0x0004, 0x0002, 0x0002, 0x0008, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010
};
const u_int16 AMD16MegBootBlockSectors[] =
{
    0x0004, 0x0002, 0x0002, 0x0038, 0x0040, 0x0040, 0x0040, 0x0040,
    0x0040, 0x0040, 0x0040
};



#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

/***********************/
/* Intel Block Formats */
/***********************/

const u_int16 Intel16MegBootBlockFlexSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};

const u_int16 Intel32MegBootBlockFlexSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};

/***********************/
/* Atmel Block Formats */
/***********************/
const u_int16 Atmel16MegBootBlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0008, 0x0008, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};

/*S29JL032H */
const u_int16 FJ32MegBootBlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};


/************************************/
/* STMicroelectronics Block Formats */
/************************************/


#endif
const u_int16 SamSungBlockSectors[] =     /* K8D6316UBM */
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010 
};

const u_int16 MX29LV160BlockSectors[] =
{
    0x0004, 0x0002, 0x0002, 0x0008, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010
};

const u_int16 MX29LVBlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};

const u_int16 MX29LV640BlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010 
};

const uint16  SST39VF3201BlockSectors[] =
{
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
};

const u_int16 S29GV064BlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010 
};

const u_int16 SST39VF6401BlockSectors[] =
{
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
 };

const u_int16 EN29LV640BlockSectors[] =
{
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010 
};

const uint16  EN29LV320BlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
};
const uint16  EN29LV160BlockSectors[] =
{
    0x0004, 0x0002, 0x0002, 0x0008, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010
};

const u_int16 STM29W640BlockSectors[] =     /* M29W640FB */
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x00010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010 
};
const uint16  M29W160EBBlockSectors[] =
{
	0x0004,0x0002,0x0002,0x0008,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010
};


const uint16  M29W320EBBlockSectors[] =
{
	0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010



};

const uint16 ES29LV160EBBlockSectors[]=
{
	0x0004,0x0002,0x0002,0x0008,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010
};


const uint16 F49L160BABlockSectors[]=
{
	0x0004,0x0002,0x0002,0x0008,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0008,
	0x0002,0x0002,0x0004
};


const uint16 S29AL016DBlockSectors[]=
{
	0x0004,0x0002,0x0002,0x0008,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,0x0010,
	0x0010,0x0010,0x0010
};


const uint16  S29AL032ABlockSectors[] =
{
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
    0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010
};

/**************************/
/* Flash Descriptor Array */
/**************************/
FLASH_DESC FlashDescriptors[] =
{

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

    {FLASH_ID_ENTER_INTEL,
     FLASH_ID_EXIT_INTEL,
     FLASH_CHIP_VENDOR_ST,
     0x0016,    //M58LW032D-90
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     65, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD         |
             FLASH_FLAGS_CONSTANT_WRITE_SPEED | 
             FLASH_FLAGS_UNIFORM_SECTOR_SIZE,
     0,         //Uniform Block Size of 128K
     (u_int16 *) 0x20},   //Uniform Block Size of 128K

    /*********/
    /* Intel */
    /*********/

    //NOTE: Intel parts that can be used in both x8 and x16 configurations
    //do NOT need to be listed as separate parts in this table. This is 
    //because they can use the same Enter/ExitIDMode routines.





    // Not tested with Rev_C
    {FLASH_ID_ENTER_INTEL,
     FLASH_ID_EXIT_INTEL,
     FLASH_CHIP_VENDOR_INTEL,
     0x88F4,    //28F160F3-120
     0x1000000, //16Mbit (1Mbit x 16)
     120, 0, 0, 0, 0,
     200, 0, 0, 0, 0,
     70,        //Burst Wait
     16,        //BytesPerPage (Burst Mode)
     FLASH_FLAGS_BURST_SUPPORTED | FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(Intel16MegBootBlockFlexSectors)/sizeof(u_int16), //# Sectors
     Intel16MegBootBlockFlexSectors},    //Sector Layout

    // Tested with Rev_C
    {FLASH_ID_ENTER_INTEL,
     FLASH_ID_EXIT_INTEL,
     FLASH_CHIP_VENDOR_INTEL,
     0x88C3,    //28F160C3-90
     0x1000000, //16Mbit (1Mbit x 16)
     75, 0, 0, 0, 0,
     65, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD | FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(Intel16MegBootBlockFlexSectors)/sizeof(u_int16), //# Sectors
     Intel16MegBootBlockFlexSectors},    //Sector Layout

    // Tested with Rev_C
    {FLASH_ID_ENTER_INTEL,
     FLASH_ID_EXIT_INTEL,
     FLASH_CHIP_VENDOR_INTEL,
     0x88C5,    //28F320C3-100
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     65, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_UNLOCK_LOCK_SEQ_REQD | FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(Intel32MegBootBlockFlexSectors)/sizeof(u_int16), //# Sectors
     Intel32MegBootBlockFlexSectors},    //Sector Layout

    /*******/
    /* AMD */
    /*******/

    //NOTE: AMD parts that can be used in both x8 and x16 configurations
    //must be listed as separate parts in this table. This is because they
    //require different EnterIDMode routines.




#endif

#if CUSTOMER != VENDOR_D

    // Tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_AMD,
     0x2249,    //29LV160DB-70
     0x1000000, //16Mbit (1Mbit x 16)
#if CUSTOMER == VENDOR_A
     90, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
#else
     70, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
#endif
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout

#endif

#if CUSTOMER != VENDOR_A && CUSTOMER != VENDOR_D

    // Tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_AMD,
     0x22C4,    //29LV160DT-70
     0x1000000, //16Mbit (1Mbit x 16)
     70, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT,       //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout



    // Not Yet Tested, but copied from 29LV160D, which is mostly the same
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_AMD,
     0x22d7,    //29LV641D-90
     0x4000000, //64Mbit (4Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_UNIFORM_SECTOR_SIZE,   //Flags
     0,         //Uniform Block Size of 64K
     (u_int16 *) 0x10},   //Uniform Block Size of 64K


    /***********/
    /* Fujitsu */
    /***********/

    // Not tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x2245,    //29PL160BD-75
     0x1000000, //16Mbit (1Mbit x 16)
     80, 0, 0, 0, 0,
     80, 0, 0, 0, 0,
     30,        //Burst Wait
     16,        //BytesPerPage (Burst Mode)
     FLASH_FLAGS_BURST_SUPPORTED,       //Flags
     sizeof(AMD16MegBootBlockSectors)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors}, //Sector Layout

    // Not tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x2227,    //29PL160TD-75
     0x1000000, //16Mbit (1Mbit x 16)
     80, 0, 0, 0, 0,
     80, 0, 0, 0, 0,
     30,        //Burst Wait
     16,        //BytesPerPage (Burst Mode)
     FLASH_FLAGS_BURST_SUPPORTED | FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT,       //Flags
     sizeof(AMD16MegBootBlockSectors)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors}, //Sector Layout

    // Not tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x2245,    //29LV160B-80
     0x1000000, //16Mbit (1Mbit x 16)
     80, 0, 0, 0, 0,
     80, 0, 0, 0, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout

    // Not tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x2227,    //29LV160T-80
     0x1000000, //16Mbit (1Mbit x 16)
     80, 0, 0, 0, 0,
     80, 0, 0, 0, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT,       //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout

    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x225F,    //29DL324BD-90
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(Intel32MegBootBlockFlexSectors)/sizeof(u_int16),  //# Sectors
     Intel32MegBootBlockFlexSectors}, //Sector Layout

    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x225C,    //29DL324TD-90
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 45, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT,       //Flags
     sizeof(Intel32MegBootBlockFlexSectors)/sizeof(u_int16),  //# Sectors
     Intel32MegBootBlockFlexSectors}, //Sector Layout





    /*******/
    /* SST */
    /*******/

    // Tested with Rev_C
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_SST,
     0x2782,    //39VF160-70
     0x1000000, //16Mbit (1Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 60, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_UNIFORM_SECTOR_SIZE,   //Flags
     0,         //Uniform Block Size of 64K
     (u_int16 *) 0x10},   //Uniform Block Size of 64K

    /*********/
    /* Atmel */
    /*********/

    // Not tested with Rev_C
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_ATMEL,
     0x00C8,    //AT49BV32X-90
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     200, 0, 0, 0, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_CONSTANT_WRITE_SPEED,  //Flags
     sizeof(Intel32MegBootBlockFlexSectors)/sizeof(u_int16),  //# Sectors
     Intel32MegBootBlockFlexSectors}, //Sector Layout

    // Not tested with Rev_C
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_ATMEL,
     0x00C9,    //AT49BV32XT-90
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     200, 0, 0, 0, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_TOP_BOOT_BLOCK_LAYOUT | FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(Intel32MegBootBlockFlexSectors)/sizeof(u_int16),  //# Sectors
     Intel32MegBootBlockFlexSectors}, //Sector Layout

    // Tested with Rev_C
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_ATMEL,
     0x00C0,    //AT49LV1614-90
     0x1000000, //16Mbit (1Mbit x 16)
     90, 0, 0, 0, 0,
     100, 0, 0, 60, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(Atmel16MegBootBlockSectors)/sizeof(u_int16),  //# Sectors
     Atmel16MegBootBlockSectors}, //Sector Layout
    
    /**********************/
    /* STMicroelectronics */
    /**********************/
    
    // Not tested with Rev_C only
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_ST,
     0x2249, 		//M29W160DB
     0x1000000, //16Mbit (1Mbit x 16)
     90, 0, 0, 0, 0,
     60, 10, 35, 35, 10,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout

#endif

#if CUSTOMER != VENDOR_A

    /***********/
    /* Toshiba */
    /***********/

    // Tested with Rev_C
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_TOSHIBA,
     0x0043,    //TC58FVB160FT-85
     0x1000000, //16Mbit (1Mbit x 16)
     85, 0, 0, 0, 0,
     65, 0, 0, 30, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout

#endif

    /************/
    /* MACRONIX */
    /************/

    

	 /************/
	 /* Fujitsu  */
	 /************/
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_FUJITSU,
     0x2249,    //29LV160BE
     0x1000000, //16Mbit (1Mbit x 16)
     80, 0, 0, 0, 0,
     80, 0, 0, 0, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(AMD16MegBootBlockSectors2)/sizeof(u_int16),  //# Sectors
     AMD16MegBootBlockSectors2}, //Sector Layout
    
    /**********************/
    /* STMicroelectronics */
    /**********************/
    

   {
     FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_ST,
     0x22FD,                     // M29W640FB-70
     0x4000000,                    // 64Mbit(4M x 16bit)
     70, 0, 0, 0, 0,             // Read Timings
     70, 0, 0, 0, 45,            // Write Timings
     0,
     0,
     0,
     sizeof(STM29W640BlockSectors)/sizeof(u_int16),
     STM29W640BlockSectors         // Sector Layout
    },
    /*add by txl ,2005-6-2
   {
     FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_AMD,
     0x227E,                     // M29W800DT-70
     0x2000000,                   // 32Mbit (4mbit x8)
     70, 0, 0, 0, 0,             //Read Timings
     50, 0, 0, 45, 0,            //Write Timings
     0,
     0,
     0,
     sizeof(FJ32MegBootBlockSectors)/sizeof(u_int16),
     FJ32MegBootBlockSectors         // Sector Layout
    },*/
    //ST  M29W160EB
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x0020,
     0x2249,      //M29W160EB
     0x1000000,                // 16Mbit(1M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(M29W160EBBlockSectors)/sizeof(u_int16),         //# Sectors
     M29W160EBBlockSectors       //Sector Layout
     } ,  

     //ST  M29W320EB 
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x0020,
     0x2257,      //M29W320EB
     0x2000000,                // 32Mbit(2M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     70, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(M29W320EBBlockSectors)/sizeof(u_int16),         //# Sectors
     M29W320EBBlockSectors       //Sector Layout
     } ,  
    
     /*SAMSUNG K8D6316UBM-8M*/
     {FLASH_ID_ENTER_SAMSUNG,
     FLASH_ID_EXIT_SAMSUNG,
     FLASH_CHIP_VENDOR_SAMSUNG,
     0x22E2,      //Empty			*_*
     0x4000000, //64Mbit (4M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     60, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(SamSungBlockSectors)/sizeof(u_int16),
     SamSungBlockSectors    //Sector Layout
     } , 

     /*mxic-2M*/
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_MACRONIX,
     0x2249,      //MX29LV160BTC
     0x1000000,                // 16Mbit(1M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(MX29LV160BlockSectors)/sizeof(u_int16),         //# Sectors
     MX29LV160BlockSectors       //Sector Layout
     } ,  


     /*mxic-4M*/
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_MACRONIX,
     0x22A8,      //MX29LV320BTC-70
     0x2000000,                // 32Mbit(2M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(MX29LVBlockSectors)/sizeof(u_int16),         //# Sectors
     MX29LVBlockSectors       //Sector Layout
     } ,  
     
     //mxic-8M---
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_MACRONIX,
     0x227E,      //MX29LV640MBTC-90
     0x4000000,                // 64Mbit(4M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(MX29LV640BlockSectors)/sizeof(u_int16),         //# Sectors
     MX29LV640BlockSectors       //Sector Layout
     } ,  
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_MACRONIX,
     0x22CB,      //MX29LV640MBTC-90
     0x4000000,                // 64Mbit(4M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(MX29LV640BlockSectors)/sizeof(u_int16),         //# Sectors
     MX29LV640BlockSectors       //Sector Layout
     } ,       
     /*SST-4M*/
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_SST,
     0x235B,    //SST39VF3201-70
     0x2000000, //32Mbit (2Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 60, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,   //Flags
     sizeof(SST39VF3201BlockSectors)/sizeof(u_int16),         //# Sectors
     SST39VF3201BlockSectors       //Sector Layout
     } ,  

     /*S29GL_064M90TAIR4*/
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_AMD,
     0x227E,      //MX29LV640MBTC-90
     0x4000000,                // 64Mbit(4M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(S29GV064BlockSectors)/sizeof(u_int16),         //# Sectors
     S29GV064BlockSectors       //Sector Layout
     } ,  


      /*SST-8M-SST39VF6401B*/
    {FLASH_ID_ENTER_ATMELX16,
     FLASH_ID_EXIT_AMD,
     FLASH_CHIP_VENDOR_SST,
     0x236D,    //SST39VF6401B-70
     0x4000000, //64Mbit (4Mbit x 16)
     90, 0, 0, 0, 0,
     50, 0, 0, 60, 0,
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,   //Flags
     sizeof(SST39VF6401BlockSectors)/sizeof(u_int16),         //# Sectors
     SST39VF6401BlockSectors       //Sector Layout
     } ,  

    //EN29LV640L-90TCP
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x007F,
     0x227E,      //
     0x4000000,                // 64Mbit(4M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     60, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(EN29LV640BlockSectors)/sizeof(u_int16),         //# Sectors
     EN29LV640BlockSectors       //Sector Layout
     } ,  

     //EN29LV320B-70TCP
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x007F,
     0x22F9,      //
     0x2000000,               //32Mbit(2M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(EN29LV320BlockSectors)/sizeof(u_int16),         //# Sectors
     EN29LV320BlockSectors       //Sector Layout
     } ,  
     //EN29LV160AB-70TCP
    {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x007F,
     0x2249,      //
     0x1000000,                // 16Mbit(1M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  *_*
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(EN29LV160BlockSectors)/sizeof(u_int16),         //# Sectors
     EN29LV160BlockSectors       //Sector Layout
     } ,  

    
	//ES29LV160EB
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x004a,
     0x2249,      //
     0x1000000,                // 16Mbit(1M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(ES29LV160EBBlockSectors)/sizeof(u_int16),         //# Sectors
     ES29LV160EBBlockSectors       //Sector Layout
     }, 
	
     //ESMT F49L160BA -70T
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x007f,
     0x0049,      //
     0x1000000,                // 16Mbit(1M x 16bit)
     70, 0, 0, 0, 0,  //Read Timings
     50, 0, 0, 45, 0,            //Write Timings  
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(F49L160BABlockSectors)/sizeof(u_int16),         //# Sectors
     F49L160BABlockSectors       //Sector Layout
     },  

	 //S29AL016D
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x0001,
     0x0049,      //
     0x1000000,                // 16Mbit(1M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(S29AL016DBlockSectors)/sizeof(u_int16),         //# Sectors
     S29AL016DBlockSectors       //Sector Layout
     },  

	//S29AL032A
     {FLASH_ID_ENTER_AMDX16,
     FLASH_ID_EXIT_AMD,
     0x0001,		//
     0x007E,      //
     0x2000000,                // 32Mbit(2M x 16bit)
     90, 0, 0, 0, 0,  //Read Timings
     90, 0, 0, 45, 0,            //Write Timings  
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     sizeof(S29AL032ABlockSectors)/sizeof(u_int16),         //# Sectors
     S29AL032ABlockSectors       //Sector Layout
     },  

	 
    /*************************/
    /*     Bank Empty        */
    /* This must be the last */
    /* entry in this table.  */
    /*************************/
     {0,
     0,
     0xff,      //Empty
     0xff,      //Empty
     0,         //Empty
     0, 0, 0, 0, 0,  //Read Timings
     0, 0, 0, 0, 0,  //Write Timings
     0,         //Burst Wait
     0,         //BytesPerPage (Burst Mode)
     0,         //Flags
     0,         //# Sectors
     0}         //Sector Layout

};

int NumFlashTypes = sizeof(FlashDescriptors) / sizeof(FLASH_DESC) - 1;

/**************************/
/* Nand Flash Descriptor Array */
/**************************/
NAND_FLASH_DESC NandFlashDescriptors[] =
{
    /**********************/
    /* Samsung Nand Flash */
    /**********************/

    {FLASH_ID_ENTER_SAMSUNG,
     FLASH_ID_EXIT_SAMSUNG,
     FLASH_CHIP_VENDOR_SAMSUNG,
     0x00E6,    //K9F6408U0A( same as the old part KM29U64000AT)
     0x4000000, //Chip size in bits: 8MB (8M x 8Bit) Nand Flash
     300,       //Read Wait(nano sec)
     700,       //Write Wait(nano sec)
     208, 24, 224, 112,
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     8, //Flash Org (x8)
     512,  // Page size
     16,  //Pages per block
     1024, //Number of blocks
     16    //Spare area size bytes
     }, 


    /**********************/
    /* Samsung Nand Flash */
    /**********************/

    {FLASH_ID_ENTER_SAMSUNG,
     FLASH_ID_EXIT_SAMSUNG,
     FLASH_CHIP_VENDOR_SAMSUNG,
     0x0073,    //K9F2808U0X(X=B or C)
     0x8000000, //Chip size in bits: 16MB (16M x 8Bit) Nand Flash
     300,
     700, 
     208, 24, 224, 112,
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     8, //Flash Org (x8)
     512,  // Page size
     32,  //Pages per block
     1024, //Number of blocks
     16    //Spare area size bytes
     }, 

    /**********************/
    /* Samsung Nand Flash */
    /**********************/

    {FLASH_ID_ENTER_SAMSUNG,
     FLASH_ID_EXIT_SAMSUNG,
     FLASH_CHIP_VENDOR_SAMSUNG,
     0x0053,    //K9F2816U0C
     0x8000000, //Chip size in bits: 16MB (8M x 16Bit) Nand Flash
     300,
     700, 
     208, 24, 224, 112,
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     16, //Flash Org (x8)
     256,  // Page size
     32,  //Pages per block
     1024, //Number of blocks
     16    //Spare area size bytes
     }, 

     {FLASH_ID_ENTER_SAMSUNG,
     FLASH_ID_EXIT_SAMSUNG,
     FLASH_CHIP_VENDOR_SAMSUNG,
     0x0053,    //K9F2816U0C
     0x8000000, //Chip size in bits: 16MB (8M x 16Bit) Nand Flash
     300,
     700, 
     208, 24, 224, 112,
     FLASH_FLAGS_CONSTANT_WRITE_SPEED, //Flags
     16, //Flash Org (x8)
     256,  // Page size
     32,  //Pages per block
     1024, //Number of blocks
     16    //Spare area size bytes
     }, 
     
    /*************************/
    /*     Bank Empty        */
    /* This must be the last */
    /* entry in this table.  */
    /*************************/
    {0,
     0,
     0xFF,
     0xFF,
     0,
     0,
     0, 
     0, 0, 0, 0,
     0, //Flags
     0, //Flash Org (x8)
     0,  // Page size
     0,  //Pages per block
     0, //Number of blocks
     0    //Spare area size bytes
     }
};
int NumNandFlashTypes = sizeof(NandFlashDescriptors) / sizeof(NAND_FLASH_DESC) - 1;


/***************************************************************************
                     PVCS Version Control Information
$Log: 
 29   mpeg      1.28        5/30/04 8:19:27 PM     Steven Shen     CR(s) 9330 
       9331 : Fix the sector organization of Fujistu 29LV160BE.
 28   mpeg      1.27        4/22/04 4:22:05 PM     Sunil Cheruvu   CR(s) 8870 
       8871 : Added the Nand Flash support for Wabash(Milano rev 5 and above) 
       and Brazos(Bronco).
 27   mpeg      1.26        3/19/04 12:14:06 AM    Xiao Guang Yan  CR(s) 8595 :
        Added flash descriptor entry for Macronix 2Mx16 and Fujitsu 1Mx16.
 26   mpeg      1.25        3/5/03 5:18:28 PM      Tim White       SCR(s) 5681 
       5682 :
       Add support for ST M58LW032D flash type.
       
       
 25   mpeg      1.24        2/13/03 12:21:14 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 24   mpeg      1.23        12/12/02 5:27:52 PM    Tim White       SCR(s) 5157 
       :
       Removed support for Wabash Rev_A (WABASH_FLASH_TIMING_BUG).
       
       
 23   mpeg      1.22        8/15/02 6:11:12 PM     Tim White       SCR(s) 4408 
       :
       Add support for Fujitsu 29DL324TD/BD 16Mb flash parts.  Removed the 
       bogus
       AMD16MegBootBlockSectors4[] array.  It was a uniform size array and not 
       used.
       
       
 22   mpeg      1.21        4/29/02 9:48:44 AM     Bobby Bradford  SCR(s) 3580 
       :
       Modified the FLASH_DESC table, to use a function ID value
       rather than a function pointer for the enter/exit ID mode
       functions.  Now, in SETUPROM.C, the functions are embedded
       into the main function (RealSetupROMs) rather than called
       via a function pointer in the table.
       
 21   mpeg      1.20        1/23/02 1:34:10 PM     Tim White       SCR(s) 3059 
       :
       Added WABASH_FLASH_TIMING_BUG to the Intel320 and the 16MB flash option.
       
       
 20   mpeg      1.19        1/14/02 1:01:24 PM     Dave Moore      SCR(s) 3006 
       :
       Changed timings of 29LV160D.
       
       
 19   mpeg      1.18        11/15/01 4:24:02 PM    Bobby Bradford  SCR(s) 2878 
       :
       Modified the table entry for AMD 29LV641 to accomodate Uniform Sectors
       
 18   mpeg      1.17        11/15/01 9:04:46 AM    Bobby Bradford  SCR(s) 2878 
       :
       Added entry for AM29LV641-90.  Hasn't been tested yet.
       
 17   mpeg      1.16        2/21/01 1:51:46 PM     Tim White       DCS#1220: 
       Set Vendor_A flash ROM speed correctly for Am29LV160DB-90 parts.
       
 16   mpeg      1.15        2/15/01 5:29:08 PM     Tim White       DCS#1215: 
       Merge Vendor_D changes back into mainline codebase.
       
 15   mpeg      1.14        1/31/01 2:30:28 PM     Tim White       DCS#0988: 
       Reclaim footprint space.
       
 14   mpeg      1.13        1/25/01 5:45:44 PM     Angela          integrated 
       Vendor_C changes into our codebase (see DCS#1049)
       
 13   mpeg      1.12        9/26/00 6:09:16 PM     Tim White       Added 
       switchable secondary descriptor capability to low-level flash function.
       
 12   mpeg      1.11        9/20/00 11:23:34 AM    Tim White       Added 
       Toshiba flash ROM capability.
       
 11   mpeg      1.10        6/9/00 11:56:22 AM     Tim White       Added 
       Echostar flash ROM parts.
       
 10   mpeg      1.9         6/2/00 2:39:14 PM      Tim White       Added 
       support for the AMD 29LV200B flash ROM parts.
       
 9    mpeg      1.8         5/17/00 12:21:10 PM    Tim White       Fixed 
       problem with SST ROM's.
       
 8    mpeg      1.7         5/1/00 11:00:16 PM     Tim White       Added 
       bringup #if's for Vendor_B.
       
 7    mpeg      1.6         4/10/00 6:21:54 PM     Tim White       Added the 
       correct Write (program) values, added several new Flash ROM types.
       
 6    mpeg      1.5         3/9/00 2:20:56 PM      Tim White       
       Changed speed of the TE28F160C3BA90 to 80ns from 60ns.
       
 5    mpeg      1.4         3/8/00 5:20:10 PM      Tim White       Restructured
        the BANK_INFO array and added support for new Intel Flash ROM.
       
 4    mpeg      1.3         1/31/00 2:21:00 PM     Tim White       Added 
       support for the AMD 29PL160BB-70 ROM's.
       
 3    mpeg      1.2         1/26/00 5:33:02 PM     Tim White       The Intel 
       28F160F3-120 ROM parts page mode accesses are actually 70ns
       not the advertised 35ns.  Changed the setting to 70ns from 40ns.
       
 2    mpeg      1.1         12/8/99 5:12:52 PM     Tim Ross        Added 
       changes for Colorado.
       
 1    mpeg      1.0         11/11/99 12:52:28 PM   Dave Wilson     
$
 * 
 *    Rev 1.25   05 Mar 2003 17:18:28   whiteth
 * SCR(s) 5681 5682 :
 * Add support for ST M58LW032D flash type.
 * 
 * 
 *    Rev 1.24   13 Feb 2003 12:21:14   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
****************************************************************************/

