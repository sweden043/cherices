/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      STV0297.C
 *
 * Description:   The STV0297 QAM Demodulator IC Driver  is one part of cable
 *                front-end driver. Some cable front-ends use the demodulator
 *                IC to demodulate IF signals and do the FEC processing.
 *
 *                The file contains the functions to read/write registers and
 *                some operation functions,  such as set/get the symbol rate,
 *                set/get two AGC values, etc.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: STV0297.C, 3, 5/26/04 2:54:14 AM, Steven Shen$
 * $Id: STV0297.C,v 1.2, 2004-05-26 07:54:14Z, Steven Shen$
 ****************************************************************************/

/***************************/
/*       Header Files      */
/***************************/
#include "stbcfg.h"
#include "STV0297.h"
#include "retcodes.h"
#include "iic.h"


#ifndef I2C_BUS_CABLE_FE
#define I2C_BUS_CABLE_FE      (I2C_BUS_0)
#endif


/***************************/
/*    Global Variables     */
/***************************/
extern DEM_REGISTER  gDemReg[DEM_REG_NUM];

/* Variables for C/N estimation */
#ifndef STV0297_CNS
int32    gST0CN[5][40];
#else
int32    gST0CN[5][108];
#endif

/***************************/
/* Static Global Variables */
/***************************/


/***************************/
/* Static Local Functions  */
/***************************/
#if (defined m88dc2000)
#define  NULL			0
//static DEM_RETURN dc2_set_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 ui8RegValue);
//static DEM_RETURN dc2_get_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 *pRegValue);
#endif
static DEM_RETURN st0_set_a_reg (u_int8 ui8Addr, u_int8 ui8Index);
static DEM_RETURN st0_get_a_reg (u_int8 ui8Addr, u_int8 ui8Index);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if (0)

static DEM_RETURN st0_set_regs (u_int8 ui8Addr, u_int8 ui8Index, u_int8 ui8Num);
static DEM_RETURN st0_get_regs (u_int8 ui8Addr, u_int8 ui8Index, u_int8 ui8Num);

/*****************************************************************************/
/*  FUNCTION:    st0_set_regs                                                */
/*                                                                           */
/*  PARAMETERS:  ui8Addr  - the i2c slave address for writing.               */
/*               ui8Index - the index of the first register to write in map. */
/*               ui8Num   - the number of data to write.                     */
/*                                                                           */
/*  DESCRIPTION: This function sets the STV0297 internal registers in burst  */
/*               through the I2C bus.                                        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  NOTES:       For burst writing, the index of registers must be continue. */
/*               The data to write are stored in the gobal register map.     */
/*               Please use this function carefully.                         */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
static DEM_RETURN st0_set_regs (u_int8 ui8Addr, u_int8 ui8Index, u_int8 ui8Num)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   u_int8      i;
   
   /* check the number of data to burst access. */
   if ( ui8Num > ST0_I2CBT_LEN )
   {
      return (DEM_ERROR);
   }
   
   /* setup data and commands for i2c transaction */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = gDemReg[ui8Index].addr;   /* base register address */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   for (i=0; i<ui8Num; i++)
   {
      ui8Data[i+2] = gDemReg[ui8Index+i].value;
      ui8Cmd[i+2]  = IIC_DATA;
   }
   ui8Cmd[i+2] = IIC_STOP;
   
   iicTransBuf.dwCount = ui8Num + 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {
      return (DEM_OK);
   }
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_regs                                                */
/*                                                                           */
/*  PARAMETERS:  ui8Addr  - the i2c slave address for reading.               */
/*               ui8Index - the index of the first register to read in map.  */
/*               ui8Num   - the number of data to read.                      */
/*                                                                           */
/*  DESCRIPTION: This function gets the STV0297 internal registers in burst  */
/*               through the I2C bus.                                        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  NOTES:       For burst reading, the index of registers must be continue. */
/*               The read data are stored in the gobal register map.         */
/*               Please use this function carefully.                         */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
static DEM_RETURN st0_get_regs (u_int8 ui8Addr, u_int8 ui8Index, u_int8 ui8Num)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   u_int8      i;
   
   /* check the number of data to burst access. */
   if ( ui8Num > ST0_I2CBT_LEN )
   {
      return (DEM_ERROR);
   }
   
   /* phase 1 */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = gDemReg[ui8Index].addr;   /* base register address */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_STOP;
   iicTransBuf.dwCount = 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   
   /* phase 2 */
   ui8Data[0] = ui8Addr + 1;  /* i2c slave address - RD */
   ui8Cmd[0]  = IIC_START;
   for (i=0; i<ui8Num; i++)
   {
      ui8Cmd[i+1]  = IIC_DATA | IIC_ACK;
   }
   ui8Cmd[i]   = IIC_DATA;    /* clear IIC_ACK flag for the last byte */
   ui8Cmd[i+1] = IIC_STOP;
   
   iicTransBuf.dwCount = ui8Num + 2;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {  /* if the i2c transaction is successful, copy data into register map */
      for (i=0; i<ui8Num; i++)
      {
         gDemReg[ui8Index+i].value = ui8Data[1+i];
      }
      return (DEM_OK);
   }
}

#endif
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*****************************************************************************/
/*  FUNCTION:    st0_set_a_reg                                               */
/*                                                                           */
/*  PARAMETERS:  ui8Addr  - the i2c slave address for writing.               */
/*               ui8Index - the index of the register to write in map.       */
/*                                                                           */
/*  DESCRIPTION: This function sets one STV0297 internal register            */
/*               through the I2C bus.                                        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  NOTES:       The datum to write is stored in the gobal register map.     */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_set_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 ui8RegValue)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   
   /* setup data and commands for i2c transaction */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = ui8RegAddr;		  /* the register address */
   ui8Data[2] = ui8RegValue;		  /* the data to write */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_DATA;
   ui8Cmd[3]  = IIC_STOP;
   
   iicTransBuf.dwCount = 4;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {
      return (DEM_OK);
   }
}
#endif
static DEM_RETURN st0_set_a_reg (u_int8 ui8Addr, u_int8 ui8Index)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   
   /* setup data and commands for i2c transaction */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = gDemReg[ui8Index].addr;   /* the register address */
   ui8Data[2] = gDemReg[ui8Index].value;  /* the data to write */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_DATA;
   ui8Cmd[3]  = IIC_STOP;
   
   iicTransBuf.dwCount = 4;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {
      return (DEM_OK);
   }
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_a_reg                                               */
/*                                                                           */
/*  PARAMETERS:  ui8Addr  - the i2c slave address for reading.               */
/*               ui8Index - the index of the register to read in map.        */
/*                                                                           */
/*  DESCRIPTION: This function gets one STV0297 internal register            */
/*               through the I2C bus.                                        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  NOTES:       The read datum is stored in the gobal register map.         */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 *pRegValue)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   
   /* phase 1 */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = ui8RegAddr;		  /* base register address */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_STOP;
   iicTransBuf.dwCount = 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   
   /* phase 2 */
   ui8Data[0] = ui8Addr + 1;  /* i2c slave address - RD */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_STOP;
   iicTransBuf.dwCount = 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {  /* if the i2c transaction is successful, copy data into register map */
      *pRegValue = ui8Data[1];
      return (DEM_OK);
   }
}
#endif
static DEM_RETURN st0_get_a_reg (u_int8 ui8Addr, u_int8 ui8Index)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[ST0_I2CBF_LEN];
   u_int8      ui8Cmd[ST0_I2CBF_LEN];
   
   /* phase 1 */
   ui8Data[0] = ui8Addr;                  /* i2c slave address - WR */
   ui8Data[1] = gDemReg[ui8Index].addr;   /* base register address */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_STOP;
   iicTransBuf.dwCount = 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   
   /* phase 2 */
   ui8Data[0] = ui8Addr + 1;  /* i2c slave address - RD */
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_STOP;
   iicTransBuf.dwCount = 3;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_FE) != TRUE )
   {  /* if the i2c transaction is failed, tell me the reason. */
      /* error code = iicTransBuf.dwError */
      return (DEM_ERROR);
   }
   else
   {  /* if the i2c transaction is successful, copy data into register map */
      gDemReg[ui8Index].value = ui8Data[1];
      return (DEM_OK);
   }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*****************************************************************************/
/*  FUNCTION:    st0_init_regs                                               */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function sets all of STV0297 internal registers        */
/*               with the application default values.                        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_init_regs (u_int8 i2cAddr)
{
   u_int8 regE3H,regE4H;
   
   dc2_set_a_reg(i2cAddr, 0x00, 0x48);
   dc2_set_a_reg(i2cAddr, 0x01, 0x09);
   dc2_set_a_reg(i2cAddr, 0xFB, 0x0A);
   dc2_set_a_reg(i2cAddr, 0xFC, 0x0B);
   dc2_set_a_reg(i2cAddr, 0x02, 0x0B);
   dc2_set_a_reg(i2cAddr, 0x03, 0x18);
   dc2_set_a_reg(i2cAddr, 0x05, 0x0D);
   #if (defined TDCH_G101F)
   dc2_set_a_reg(i2cAddr, 0x30, 0xFF);
   dc2_set_a_reg(i2cAddr, 0x31, 0x00);
   dc2_set_a_reg(i2cAddr, 0x32, 0xD8);
   dc2_set_a_reg(i2cAddr, 0x33, 0x38);
   dc2_set_a_reg(i2cAddr, 0x35, 0x58);
   #else
   dc2_set_a_reg(i2cAddr, 0x30, 0xFF);
   dc2_set_a_reg(i2cAddr, 0x31, 0x00);
   dc2_set_a_reg(i2cAddr, 0x32, 0xd0);
   dc2_set_a_reg(i2cAddr, 0x33, 0x00);
   dc2_set_a_reg(i2cAddr, 0x35, 0x48);
   #endif
   dc2_set_a_reg(i2cAddr, 0x36, 0x80);
   dc2_set_a_reg(i2cAddr, 0x44, 0xAA);	//0xAB
   dc2_set_a_reg(i2cAddr, 0x40, 0x00);
   dc2_set_a_reg(i2cAddr, 0x41, 0x10);
   dc2_set_a_reg(i2cAddr, 0x42, 0x14);
   dc2_set_a_reg(i2cAddr, 0x43, 0x40);
   dc2_set_a_reg(i2cAddr, 0x83, 0x00);
   dc2_set_a_reg(i2cAddr, 0x55, 0x7A);
   dc2_set_a_reg(i2cAddr, 0x56, 0xD9);
   dc2_set_a_reg(i2cAddr, 0x57, 0xDF);
   dc2_set_a_reg(i2cAddr, 0x58, 0x39);
   dc2_set_a_reg(i2cAddr, 0x5A, 0x00);
   dc2_set_a_reg(i2cAddr, 0x5C, 0x71);
   dc2_set_a_reg(i2cAddr, 0x5D, 0x23);
   dc2_set_a_reg(i2cAddr, 0x86, 0x40);
   dc2_set_a_reg(i2cAddr, 0xF9, 0x08);
   dc2_set_a_reg(i2cAddr, 0x61, 0x40);
   dc2_set_a_reg(i2cAddr, 0x62, 0x0A);
   dc2_set_a_reg(i2cAddr, 0x6C, 0x16);	//0x2C
   dc2_set_a_reg(i2cAddr, 0x6D, 0x10);	//0x16
   dc2_set_a_reg(i2cAddr, 0x6E, 0x18);
   dc2_set_a_reg(i2cAddr, 0x4A, 0xCD);
   dc2_set_a_reg(i2cAddr, 0x4B, 0xFF);
   dc2_set_a_reg(i2cAddr, 0x90, 0x06);
   dc2_set_a_reg(i2cAddr, 0xDE, 0x00);
   dc2_set_a_reg(i2cAddr, 0xA0, 0x03);
   dc2_set_a_reg(i2cAddr, 0xDF, 0x81);
   dc2_set_a_reg(i2cAddr, 0xFA, 0x40);
   dc2_set_a_reg(i2cAddr, 0x37, 0x10);
   dc2_set_a_reg(i2cAddr, 0xF0, 0x40);
   dc2_set_a_reg(i2cAddr, 0xF1, 0x00);
   dc2_set_a_reg(i2cAddr, 0xF2, 0x9C);
   dc2_set_a_reg(i2cAddr, 0xF3, 0x40);

   dc2_get_a_reg( i2cAddr, 0xE3, &regE3H);
   dc2_get_a_reg( i2cAddr, 0xE4, &regE4H);
   if (((regE3H & 0xC0) == 0x00) && ((regE4H & 0xC0) == 0x00))
   {
	   dc2_set_a_reg(i2cAddr, 0xF1, 0x02);
	   dc2_set_a_reg(i2cAddr, 0xF4, 0x04);
	   dc2_set_a_reg(i2cAddr, 0xF5, 0x00);
   }
   else
   {
	   dc2_set_a_reg(i2cAddr, 0xF1, 0x00);
	   dc2_set_a_reg(i2cAddr, 0xF4, 0x00);
	   dc2_set_a_reg(i2cAddr, 0xF5, 0x40);
	   #if (defined TDCH_G101F)
	   dc2_set_a_reg(i2cAddr, 0x39, 0x38);
	   dc2_set_a_reg(i2cAddr, 0x3A, 0x00);
	   #else
	   dc2_set_a_reg(i2cAddr, 0x39, 0x00);
	   dc2_set_a_reg(i2cAddr, 0x3A, 0x00);
	   #endif
   }

   dc2_set_a_reg(i2cAddr, 0xF6, 0x4E);
   dc2_set_a_reg(i2cAddr, 0xF7, 0x20);
   dc2_set_a_reg(i2cAddr, 0x89, 0x02);
   dc2_set_a_reg(i2cAddr, 0x14, 0x08);
   dc2_set_a_reg(i2cAddr, 0x6F, 0x0D);	//0x14
   dc2_set_a_reg(i2cAddr, 0x10, 0xFF);	//0x0A
   dc2_set_a_reg(i2cAddr, 0x11, 0x00);
   dc2_set_a_reg(i2cAddr, 0x12, 0x30);	//0x6C
   dc2_set_a_reg(i2cAddr, 0x13, 0x23);	//0x3C
   dc2_set_a_reg(i2cAddr, 0x60, 0x00);
   dc2_set_a_reg(i2cAddr, 0x69, 0x00);
   dc2_set_a_reg(i2cAddr, 0x6A, 0x03);
   dc2_set_a_reg(i2cAddr, 0x4C, 0x0C);
   dc2_set_a_reg(i2cAddr, 0x4D, 0x77);  //should be 0x77  (2006-11-22)
   dc2_set_a_reg(i2cAddr, 0x74, 0x0E);  // added 0x74 Reg,(2006-11-22)
   dc2_set_a_reg(i2cAddr, 0xE0, 0xD5);
   dc2_set_a_reg(i2cAddr, 0xE1, 0x25);	//0x26
   dc2_set_a_reg(i2cAddr, 0xE2, 0x06);
   dc2_set_a_reg(i2cAddr, 0x8D, 0x29);
   dc2_set_a_reg(i2cAddr, 0x8B, 0x5B);
   dc2_set_a_reg(i2cAddr, 0x4E, 0x77);
   dc2_set_a_reg(i2cAddr, 0x8E, 0x9D);
   dc2_set_a_reg(i2cAddr, 0x84, 0x6C);
   dc2_set_a_reg(i2cAddr, 0xC0, 0x43);
   dc2_set_a_reg(i2cAddr, 0x88, 0x80);

   dc2_set_a_reg(i2cAddr, 0x52,0x51);
   dc2_set_a_reg(i2cAddr, 0x53,0x03);
   dc2_set_a_reg(i2cAddr, 0x59,0x30);
   dc2_set_a_reg(i2cAddr, 0x5E,0x02);
   dc2_set_a_reg(i2cAddr, 0x71,0x03);
   dc2_set_a_reg(i2cAddr, 0x72,0x12);
   dc2_set_a_reg(i2cAddr, 0x73,0x12);

   return (DEM_OK);
}
#endif
DEM_RETURN st0_init_regs (u_int8 i2cAddr)
{
   u_int8      ui8Index;
   
   for (ui8Index=0; ui8Index<(DEM_REG_NUM); ui8Index++)
   {
      /* check if the register is read-only */
      if ( gDemReg[ui8Index].wflag )
      {  /* if the register is writable, set it with default value */
         gDemReg[ui8Index].value = gDemReg[ui8Index].start;
         st0_set_a_reg( i2cAddr, ui8Index);
      }
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_catch_regs                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function gets all values of STV0297 internal registers */
/*               and stores them into the register map. (ONLY FOR DEBUG)     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_catch_regs (u_int8 i2cAddr)
{
   u_int8      ui8Index;
   
   for (ui8Index=0; ui8Index<(DEM_REG_NUM); ui8Index++)
   {
      st0_get_a_reg( i2cAddr, ui8Index);
      gDemReg[ui8Index].resv3 = gDemReg[ui8Index].value;
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_store_regs                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function stores all current values of STV0297 internal */
/*               registers into the register map.                            */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_store_regs (u_int8 i2cAddr)
{
   u_int8      ui8Index;
   
   for (ui8Index=0; ui8Index<(DEM_REG_NUM); ui8Index++)
   {
      gDemReg[ui8Index].store = gDemReg[ui8Index].value;
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_reload_regs                                             */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function reloads all of STV0297 internal registers     */
/*               with the stored values.                                     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_reload_regs (u_int8 i2cAddr)
{
   u_int8      ui8Index;
   
   for (ui8Index=0; ui8Index<(DEM_REG_NUM); ui8Index++)
   {
      if ( gDemReg[ui8Index].wflag )
      {
         gDemReg[ui8Index].value = gDemReg[ui8Index].store;
         st0_set_a_reg( i2cAddr, ui8Index);
      }
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_swreset                                                 */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function resets most of STV0297 QAM demodulator        */
/*               by programming the SOFT_RESET bit of CTRL_0 register.       */
/*               However, blocks that can be individually reset under        */
/*               software control are not affected, such as EQ, RS, DI.      */
/*               The S/W reset is same as grounding pin NRESET of STV0297.   */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_global_soft_reset (u_int8 i2cAddr)
{
   u_int8 RegValue;
   
   dc2_get_a_reg(i2cAddr, 0x84, &RegValue);
   RegValue = RegValue & 0xBF;
   dc2_set_a_reg(i2cAddr, 0x84, RegValue);
   RegValue = RegValue | 0x40;
   dc2_set_a_reg(i2cAddr, 0x84, RegValue);
   return (DEM_OK);
}
DEM_RETURN dc2_soft_reset (u_int8 i2cAddr)
{
   dc2_set_a_reg(i2cAddr, 0x80, 0x01);
   dc2_set_a_reg(i2cAddr, 0x82, 0x00);
   dc2_set_a_reg(i2cAddr, 0x80, 0x00);
   return (DEM_OK);
}
#endif
DEM_RETURN st0_swreset (u_int8 i2cAddr)
{
   /* forces most of the chip to reset by setting SOFT_RESET bit. */
   gDemReg[ST0_RID_CTRL_0].value = gDemReg[ST0_RID_CTRL_0].value | 0x01;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_0);
   /* goes out of reset by clearing SOFT_RESET bit. */
   gDemReg[ST0_RID_CTRL_0].value = gDemReg[ST0_RID_CTRL_0].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_0);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_rs_swreset                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function resets R-S block of STV0297 QAM demodulator   */
/*               by programming the RESET_RS bit of CTRL_3 register.         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_rs_swreset (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value | 0x10;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_3);
   gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value & 0xEF;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_3);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_di_swreset                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function reset deinterleaver, descrambler sync detector */
/*               and deinterleaver sync detector of STV0297 QAM demodulator  */
/*               by programming the RESET_DI bit of CTRL_1 register.         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_di_swreset (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_CTRL_1].value = gDemReg[ST0_RID_CTRL_1].value | 0x01;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_1);
   gDemReg[ST0_RID_CTRL_1].value = gDemReg[ST0_RID_CTRL_1].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_1);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_ver                                                 */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData    - the pointer to the data of version information.  */
/*                                                                           */
/*  DESCRIPTION: The function get the STV0297 chip version information       */
/*               by reading the VERSION bits of CTRL_0 register.             */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_ver (u_int8 i2cAddr, u_int8 *pData)
{
   /* read the internal register */
   dc2_get_a_reg( i2cAddr, 0x80, pData);
   /* get the chip version information */
   *pData = (*pData) >> 4;
   return (DEM_OK);
}
#else
DEM_RETURN st0_get_ver (u_int8 i2cAddr, u_int8 *pData)
{
   /* read the internal register */
   st0_get_a_reg( i2cAddr, ST0_RID_CTRL_0);
   /* get the chip version information */
   *pData = (gDemReg[ST0_RID_CTRL_0].value >> 4) & 0x07;
   return (DEM_OK);
}
#endif
/*****************************************************************************/
/*  FUNCTION:    st0_set_spec_inv                                            */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               ui8SI - option of spectrum inversion.                       */
/*                                                                           */
/*  DESCRIPTION: The function sets the spectrum inversion option             */
/*               by programming the SPEC_INV bit of CTRL_3 register.         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_set_spec_inv (u_int8 i2cAddr, u_int8 ui8SI)
{
   u_int8 RegValue;
   
   dc2_get_a_reg(i2cAddr, 0x83, &RegValue);
   if ( ui8SI == ST0_SPECTRUM_INVERSION )
   {
      RegValue = RegValue | 0x08;
   }
   else
   {
      RegValue = RegValue & 0xF7;
   }
   dc2_set_a_reg(i2cAddr, 0x83, RegValue);
   return (DEM_OK);
}
#else
DEM_RETURN st0_set_spec_inv (u_int8 i2cAddr, u_int8 ui8SI)
{
   if ( ui8SI == ST0_SPECTRUM_INVERSION )
   {
      gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value | 0x08;
   }
   else
   {
      gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value & 0xF7;
   }
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_3);
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_spec_inv                                            */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data. 1 / 0.              */
/*                                                                           */
/*  DESCRIPTION: The function gets the spectrum inversion option             */
/*               by reading the SPEC_INV bit of CTRL_3 register.             */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_spec_inv (u_int8 i2cAddr, u_int8 *pData)
{
   st0_get_a_reg( i2cAddr, ST0_RID_CTRL_3);
   if ( gDemReg[ST0_RID_CTRL_3].value & 0x08 )
   {
      *pData = (u_int8)ST0_SPECTRUM_INVERSION;
   }
   else
   {
      *pData = (u_int8)ST0_SPECTRUM_NORMAL;
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_set_itu_j83_mode                                        */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               ui8Mode  - ITU-J83 mode (ST0_ITU_J83A / ST0_ITU_J83C).      */
/*                                                                           */
/*  DESCRIPTION: The function sets the ITU-J83 mode                          */
/*               by programming the J83C bit of CTRL_3 register.             */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_set_itu_j83_mode (u_int8 i2cAddr, u_int8 ui8Mode)
{
   if ( ui8Mode == ST0_ITU_J83C )
   {
      gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value | 0x01;
   }
   else
   {
      gDemReg[ST0_RID_CTRL_3].value = gDemReg[ST0_RID_CTRL_3].value & 0xFE;
   }
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_3);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_itu_j83_mode                                        */
/*                                                                           */
/*  PARAMETERS:  i2cAddr - the i2c address for accessing STV0297 registers.  */
/*               pData - the pointer to the return data.                     */
/*                       (ST0_ITU_J83A / ST0_ITU_J83C).                      */
/*                                                                           */
/*  DESCRIPTION: The function gets the ITU-J83 mode                          */
/*               by reading the J83C bit of CTRL_3 register.                 */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_itu_j83_mode (u_int8 i2cAddr, u_int8 *pData)
{
   st0_get_a_reg( i2cAddr, ST0_RID_CTRL_3);
   if ( gDemReg[ST0_RID_CTRL_3].value & 0x01 )
   {
      *pData = (u_int8)ST0_ITU_J83C;
   }
   else
   {
      *pData = (u_int8)ST0_ITU_J83A;
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_start_i2c_repeater                                      */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function enables the i2c repeater for one i2c access    */
/*               by setting the I2CT_EN bit of CTRL_6 register.              */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_start_i2c_repeater (u_int8 i2cAddr)
{
//   u_int8 *pRegValue=NULL;// 语法问题,加NULL可以
   u_int8 RegValue;
   dc2_get_a_reg(i2cAddr, 0x86, &RegValue);
   RegValue = RegValue | 0x80;
   dc2_set_a_reg( i2cAddr, 0x86, RegValue);
   return (DEM_OK);
}
#else
DEM_RETURN st0_start_i2c_repeater (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_CTRL_6].value = gDemReg[ST0_RID_CTRL_6].value | 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_6);
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_equ_swreset                                             */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: This function resets equalizer of STV0297 QAM demodulator   */
/*               by programming the RESET_EQL bit of CTRL_4 register.  The   */
/*               registers EQU_0 - EQU_4 are also reinitialized.             */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_equ_swreset (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_CTRL_4].value = gDemReg[ST0_RID_CTRL_4].value | 0x01;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_4);
   gDemReg[ST0_RID_CTRL_4].value = gDemReg[ST0_RID_CTRL_4].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_4);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_equ_init                                                */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the Equalizer module                */
/*               by programming the EQU_n registers.                         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_equ_init (u_int8 i2cAddr)
{
   /* 1: software reset the equalizer */
   st0_equ_swreset (i2cAddr);
   /* 2: set the u_threshold, initial_u & blind_u for the equalizer */
   gDemReg[ST0_RID_EQU_0].value = gDemReg[ST0_RID_EQU_0].start;
   gDemReg[ST0_RID_EQU_1].value = gDemReg[ST0_RID_EQU_1].start;
   st0_set_a_reg( i2cAddr, ST0_RID_EQU_0);
   st0_set_a_reg( i2cAddr, ST0_RID_EQU_1);
   /* 3: clear the LMS1 & LMS2 interrupt bits */
   gDemReg[ST0_RID_CTRL_2].value = gDemReg[ST0_RID_CTRL_2].value | 0x0C;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_2);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_set_qam_size                                            */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               ui8QAM - the QAM mode ( 16 / 32 / 64 / 128 / 256 )          */
/*                        or the auto-detect QAM mode                        */
/*                                                                           */
/*  DESCRIPTION: The function sets the QAM mode                              */
/*               by programming the MODE_SELECT bits of EQU_0 register or    */
/*               the bits of CTRL_9 register.                                */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_set_qam_size (u_int8 i2cAddr, u_int8 ui8QAM)
{
	u_int8 reg00H, reg4AH,regC2H,regC0H,regE2H,regC1H,reg44H,reg4CH;
	u_int8 RegValue;

	switch(ui8QAM){
	case ST0_QAM16:	// 16 QAM
		reg00H = 0x08;
		reg4AH = 0x0F;
		regC2H=0x02;
		regC0H=0x43;
		regE2H=0x06;
		reg44H=0xAA;  //added 2006-11-23
		reg4CH=0x0C;  //added 2006-11-23
		break;
	case ST0_QAM32:	// 32 QAM
		reg00H = 0x18;
		reg4AH = 0xFB;
		regC2H=0x02;
		regC0H=0x43;
		regE2H=0x06;
	  reg44H=0xAA;  //added 2006-11-23
		reg4CH=0x0C;  //added 2006-11-23
		break;
	case ST0_QAM128:	// 128 QAM
		reg00H = 0x28;
		reg4AH = 0xF9;
		regC2H=0x02;
		regC0H=0x43;
		regE2H=0x06;
		reg44H=0xAA;  //added 2006-11-23
		reg4CH=0x0C;  //added 2006-11-23
		break;
	case ST0_QAM256:	// 256 QAM
		reg00H = 0x38;
		reg4AH = 0xCD;
		regC2H=0x01;
		regC0H=0x43;
		regE2H=0x06;
		reg44H=0xAA;  //added 2006-11-23
		reg4CH=0x0E;  //added 2006-11-23
		
		break;
	case ST0_QAM64:	// 64 QAM
		
	default:
/*		reg00H = 0x38;//256
		reg4AH = 0xCD;
		regC2H=0x01;
		regC0H=0x43;
		regE2H=0x06;
*/		
 //       regC1H=0x40;
		reg00H = 0x48; //qam64
		reg4AH = 0xCD;
		regC2H=0x02;
		regC0H=0x43;
		regE2H=0x06;
		reg44H=0xAA;  //added 2006-11-23
		reg4CH=0x0C;  //added 2006-11-23

	}
	dc2_set_a_reg(i2cAddr, 0x00, reg00H);

	dc2_get_a_reg(i2cAddr, 0x88, &RegValue);
	RegValue = RegValue | 0x08;
	dc2_set_a_reg(i2cAddr, 0x88, RegValue);
	dc2_set_a_reg(i2cAddr, 0x4B, 0xFF);
	dc2_set_a_reg(i2cAddr, 0x4A, reg4AH);
	RegValue = RegValue & 0xF7;
	dc2_set_a_reg(i2cAddr, 0x88, RegValue);
	
	dc2_set_a_reg(i2cAddr, 0xC2, regC2H);
	dc2_set_a_reg(i2cAddr, 0xC0, regC0H);
  //  dc2_set_a_reg(i2cAddr, 0xC1, regC1H);
	dc2_set_a_reg(i2cAddr, 0xE2, regE2H);
	dc2_set_a_reg(i2cAddr, 0x44, reg44H);
	dc2_set_a_reg(i2cAddr, 0x4C, reg4CH);
   return (DEM_OK);
}
#else
DEM_RETURN st0_set_qam_size (u_int8 i2cAddr, u_int8 ui8QAM)
{
   if (ui8QAM == ST0_QAM_AUTO)
   {
      /* set the MODE_SELECT bits of EQU_0 register to 000b */
      gDemReg[ST0_RID_EQU_0].value = gDemReg[ST0_RID_EQU_0].value & 0x8F;
      st0_set_a_reg( i2cAddr, ST0_RID_EQU_0);
      /* set the CTRL_9 register for the auto-detect QAM mode
       *
       * the AUTO_QAMMODE_SEL bit  = 1b
       * the AUTOCONSTEL_TIMER bit = 0111b
       * the AUTOSTOP_CONSTEL bit  = 1b
       * the AUTOCONSTEL_ON bit    = 1b
       */
      gDemReg[ST0_RID_CTRL_9].value = 0xBE;//0X86;//0x3E; 
      st0_set_a_reg( i2cAddr, ST0_RID_CTRL_9);
   }
   else
   {
      ui8QAM = (ui8QAM << 4) & 0x70;
      gDemReg[ST0_RID_EQU_0].value = gDemReg[ST0_RID_EQU_0].value & 0x8F;
      gDemReg[ST0_RID_EQU_0].value = gDemReg[ST0_RID_EQU_0].value | ui8QAM;
      st0_set_a_reg( i2cAddr, ST0_RID_EQU_0);
   }
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_qam_size                                            */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the QAM mode                              */
/*               by reading the MODE_SELECT bits of EQU_0 register.          */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_qam_size (u_int8 i2cAddr, u_int8 *pData)
{
   u_int8   reg00H;
   dc2_get_a_reg(i2cAddr, 0x00, &reg00H);
   *pData = ((reg00H & 0x70) >> 4);
   return (DEM_OK);
}
#else
DEM_RETURN st0_get_qam_size (u_int8 i2cAddr, u_int8 *pData)
{
   st0_get_a_reg( i2cAddr, ST0_RID_EQU_0);
   *pData = ((gDemReg[ST0_RID_EQU_0].value & 0x70) >> 4);
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_noise_accumulator                                   */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the 16-bit noise estimation accumulator   */
/*               by reading the EQU_7 & EQU_8 registers.                     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_noise_accumulator (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   u_int8	     reg07H, reg08H;
   
   dc2_get_a_reg(i2cAddr, 0x07, &reg07H);
   dc2_get_a_reg(i2cAddr, 0x08, &reg08H);
   
   ui16Temp  = (u_int16)(reg08H);
   ui16Temp = (ui16Temp << 8) + (u_int16)(reg07H);
   *pData = ui16Temp;
   return (DEM_OK);
}
#else
DEM_RETURN st0_get_noise_accumulator (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_EQU_7);
   st0_get_a_reg( i2cAddr, ST0_RID_EQU_8);
   
   ui16Temp  = ((u_int16)(gDemReg[ST0_RID_EQU_8].value) << 8);
   ui16Temp += (u_int16)(gDemReg[ST0_RID_EQU_7].value);
   *pData = ui16Temp;
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_disable_initdem                                         */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function disables the initial quadrature demodulator    */
/*               by programming the IN_DEMOD_ON bit of INITDEM_5 register.   */
/*                                                                           */
/*               It is performed using a simpler sequencer which always      */
/*               requires F(CLKEXT) = 0.8 * F(IF).                           */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_disable_initdem (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_INITDEM_5].value = 0x08;
   st0_set_a_reg( i2cAddr, ST0_RID_INITDEM_5);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_wbagc_init                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the WBAGC module                    */
/*               by programming the WBAGC_n registers.                       */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_wbagc_init (u_int8 i2cAddr)
{
   /* 1: freeze the WBAGC loop */
   gDemReg[ST0_RID_WBAGC_3].value = gDemReg[ST0_RID_WBAGC_3].value & 0xEF;
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_3);
   /* 2: reset the WBAGC function, the bit is self-clearing. */
   gDemReg[ST0_RID_WBAGC_3].value = gDemReg[ST0_RID_WBAGC_3].value | 0x40;
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_3);
   /* 3: clear the WBAGC lock status */
   gDemReg[ST0_RID_WBAGC_3].value = gDemReg[ST0_RID_WBAGC_3].value & 0xF7;
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_3);
   /* 4: initialize the WBAGC agc2sd, agc2sd = 256. */
   
   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   gDemReg[ST0_RID_WBAGC_1].value = 0x00;
   gDemReg[ST0_RID_WBAGC_2].value = 0x29;
   #else
   gDemReg[ST0_RID_WBAGC_1].value = gDemReg[ST0_RID_WBAGC_1].start;
   gDemReg[ST0_RID_WBAGC_2].value = gDemReg[ST0_RID_WBAGC_2].start;
   #endif
   
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_1);
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_2);
   /* 5: unfreeze the AGC2 & the AGC1 */
   gDemReg[ST0_RID_DELAGC_6].value = 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_DELAGC_6);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_wbagc_get_acq                                           */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the WBAGC lock status                     */
/*               by reading the bit wagc_acq of the WBAGC_3 register.        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_agc_lock (u_int8 i2cAddr, u_int8 *pData)
{
   u_int8      reg43H;
   
   dc2_get_a_reg( i2cAddr, 0x43, &reg43H);
   if ((reg43H & 0x08) == 0x08)
   {
      *pData = (u_int8)(ST0_WBAGC_LOCKED);
   }
   else
   {
      *pData = (u_int8)(ST0_WBAGC_UNLOCKED);
   }
   return (DEM_OK);
}
#endif

DEM_RETURN st0_wbagc_get_acq (u_int8 i2cAddr, u_int8 *pData)
{
   u_int8      uTemp;
   
   /* get the WBAGC wagc_acq bit. */
   st0_get_a_reg( i2cAddr, ST0_RID_WBAGC_3);
   uTemp = (gDemReg[ST0_RID_WBAGC_3].value) & 0x10;
   if (0x10 == uTemp)
   {
      *pData = (u_int8)(ST0_WBAGC_LOCKED);
   }
   else
   {
      *pData = (u_int8)(ST0_WBAGC_UNLOCKED);
   }
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_wbagc_get_agc2sd                                        */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the 10-bit WBAGC agc2sd value             */
/*               by reading the WBAGC_1 and WBAGC_2 register.                */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_agc2sd (u_int8 i2cAddr, u_int16 *pData)
{
   u_int8 regE3H,regE4H,RFAGC,IFAGC;

   dc2_get_a_reg( i2cAddr, 0xE3, &regE3H);
   dc2_get_a_reg( i2cAddr, 0xE4, &regE4H);
   if (((regE3H & 0xC0) == 0x00) && ((regE4H & 0xC0) == 0x00))
   {
	   dc2_get_a_reg( i2cAddr, 0x55, &RFAGC);
	   dc2_get_a_reg( i2cAddr, 0x56, &IFAGC);
   }
   else
   {
	   dc2_get_a_reg( i2cAddr, 0x3B, &RFAGC);
	   dc2_get_a_reg( i2cAddr, 0x3C, &IFAGC);
   }
   *pData = RFAGC + IFAGC;

/*   u_int8     reg30HInit, reg31HInit, reg32HInit, reg33HInit, reg35HInit;
   u_int8	   reg30H, reg37H, reg38H, reg88H, i, j;
   u_int16 sum;
   
   dc2_get_a_reg(i2cAddr, 0x30, &reg30HInit);
   dc2_get_a_reg(i2cAddr, 0x31, &reg31HInit);
   dc2_get_a_reg(i2cAddr, 0x32, &reg32HInit);
   dc2_get_a_reg(i2cAddr, 0x33, &reg33HInit);
   dc2_get_a_reg(i2cAddr, 0x35, &reg35HInit);

   dc2_get_a_reg(i2cAddr, 0x88, &reg88H);
	reg88H &= 0xF7;
	dc2_set_a_reg(i2cAddr, 0x88, reg88H);
   dc2_set_a_reg(i2cAddr, 0x37, 0x40);

  sum = 0;
   for(j=0;j<5;j++)
   	{

   reg30H = 0x80;
   dc2_set_a_reg(i2cAddr, 0x30, 0xb0);
   dc2_set_a_reg(i2cAddr, 0x31, 0xb0);
   dc2_set_a_reg(i2cAddr, 0x35, 0xb0);
   dc2_set_a_reg(i2cAddr, 0x32, reg30H);
   dc2_set_a_reg(i2cAddr, 0x33, reg30H);

   for(i=0;i<8;i++)
   {
	reg88H |= 0x08;
	dc2_set_a_reg(i2cAddr, 0x88, reg88H);
	reg88H &= 0xF7;
	dc2_set_a_reg(i2cAddr, 0x88, reg88H);
   	task_time_sleep(1);
	dc2_get_a_reg(i2cAddr, 0x37, &reg37H);
	dc2_get_a_reg(i2cAddr, 0x38, &reg38H);
	if((reg37H == 0x40) && (reg38H == 0x00))
	{
		reg30H = reg30H + (0x80 >> (i + 1));
	}
	else
	{
		reg30H = reg30H - (0x80 >> (i + 1));
	}
	dc2_set_a_reg(i2cAddr, 0x32, reg30H);
   	dc2_set_a_reg(i2cAddr, 0x33, reg30H);
   }
   sum = sum + reg30H;
   	}
   reg30H = sum / 5;

   dc2_set_a_reg(i2cAddr, 0x30, reg30HInit);
   dc2_set_a_reg(i2cAddr, 0x31, reg31HInit);
   dc2_set_a_reg(i2cAddr, 0x32, reg32HInit);
   dc2_set_a_reg(i2cAddr, 0x33, reg33HInit);
   dc2_set_a_reg(i2cAddr, 0x35, reg35HInit);
   
   *pData = reg30H;
*/
   return (DEM_OK);
}
#else
DEM_RETURN st0_wbagc_get_agc2sd (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     uTemp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_WBAGC_1);
   st0_get_a_reg( i2cAddr, ST0_RID_WBAGC_2);
   uTemp  = (u_int16)(gDemReg[ST0_RID_WBAGC_1].value);
   uTemp += (((u_int16)(gDemReg[ST0_RID_WBAGC_2].value) & 0x0003) << 8);
   *pData = uTemp;
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_stl_init                                                */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the symbol timing recovery loop     */
/*               by programming the STLOOP_n registers.                      */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_stl_init (u_int8 i2cAddr)
{
   /* phase clear, and freeze all blocks that work at symbol rate. */
   gDemReg[ST0_RID_STLOOP_10].value = gDemReg[ST0_RID_STLOOP_10].value | 0x20;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_10);
   /* STL integral path clear */
   gDemReg[ST0_RID_STLOOP_11].value = gDemReg[ST0_RID_STLOOP_11].value | 0x02;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_11);
   /* STL integral path clear release, and disable STL integral path,
    * STL integral path will be enable only after the WBAGC lock
    */
   gDemReg[ST0_RID_STLOOP_11].value = gDemReg[ST0_RID_STLOOP_11].value & 0xFC;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_11);
   /* disable STL direct path */
   gDemReg[ST0_RID_STLOOP_10].value = gDemReg[ST0_RID_STLOOP_10].value & 0xBF;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_10);
   return (DEM_OK);
}

DEM_RETURN st0_stl_freeze (u_int8 i2cAddr)
{
   /* phase clear, and freeze all blocks that work at symbol rate. */
   gDemReg[ST0_RID_STLOOP_10].value = gDemReg[ST0_RID_STLOOP_10].value | 0x20;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_10);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_stl_unfreeze                                            */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function unfreeze all blocks that work at symbol rate   */
/*               by programming the PHASE_CLR of the STLOOP_10 register.     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_stl_unfreeze (u_int8 i2cAddr)
{
   /* unfreeze all blocks that work at symbol rate. */
   gDemReg[ST0_RID_STLOOP_10].value = gDemReg[ST0_RID_STLOOP_10].value & 0xDF;
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_10);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_set_symbol_rate                                         */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               uSR - a 32-bit symbol timing frequency.                     */
/*                                                                           */
/*  DESCRIPTION: The function sets 32-bit symbol timing frequency            */
/*               by programming STLOOP_5 / STLOOP_6 / STLOOP_7 / STLOOP_8    */
/*               register.                                                   */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
//DEM_RETURN dc2_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR, u_int32 uSR_inv, u_int8 LowSym, u_int8 DDSStep)
DEM_RETURN dc2_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR, u_int32 uSR_inv, u_int8 LowSym, u_int8 reg6FH, u_int8 reg12H)

{
   u_int8 RegValue;
   
   RegValue = (u_int8)(uSR & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x55, RegValue);
   RegValue = (u_int8)((uSR >> 8) & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x56, RegValue);
   RegValue = (u_int8)((uSR >> 16) & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x57, RegValue);
   RegValue = (u_int8)((uSR >> 24) & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x58, RegValue);
   
   RegValue = (u_int8)(uSR_inv & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x5C, RegValue);
   RegValue = (u_int8)((uSR_inv >> 8) & 0x000000FF);
   dc2_set_a_reg(i2cAddr, 0x5D, RegValue);
   
   dc2_get_a_reg(i2cAddr, 0x5A, &RegValue);
   if((uSR_inv & 0x00010000) == 0)
   	RegValue = RegValue & 0x7F;
   else
   	RegValue = RegValue | 0x80;
   dc2_set_a_reg(i2cAddr, 0x5A, RegValue);

   dc2_get_a_reg(i2cAddr, 0x89, &RegValue);
   RegValue = (RegValue & 0xFE) | LowSym;
   dc2_set_a_reg(i2cAddr, 0x89, RegValue);
   
/*   dc2_get_a_reg(i2cAddr, 0x12, &RegValue);
   DDSStep = (DDSStep << 4) & 0xF0;
   RegValue &= 0x0F;
   RegValue |= DDSStep;
   dc2_set_a_reg(i2cAddr, 0x12, RegValue);
*/
   dc2_set_a_reg(i2cAddr, 0x6F, reg6FH);
   dc2_set_a_reg(i2cAddr, 0x12, reg12H);
   return (DEM_OK);
}
#else
DEM_RETURN st0_set_symbol_rate (u_int8 i2cAddr, u_int32 uSR)
{
   gDemReg[ST0_RID_STLOOP_5].value = (u_int8)(uSR & 0x000000FF);
   gDemReg[ST0_RID_STLOOP_6].value = (u_int8)((uSR >> 8) & 0x000000FF);
   gDemReg[ST0_RID_STLOOP_7].value = (u_int8)((uSR >> 16) & 0x000000FF);
   gDemReg[ST0_RID_STLOOP_8].value = (u_int8)((uSR >> 24) & 0x000000FF);
   
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_5);
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_6);
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_7);
   st0_set_a_reg( i2cAddr, ST0_RID_STLOOP_8);
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_symbol_rate                                         */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets 32-bit symbol timing frequency            */
/*               by reading STLOOP_5 / STLOOP_6 / STLOOP_7 / STLOOP_8 REGs.  */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_symbol_rate (u_int8 i2cAddr, u_int32 *pData)
{
   u_int32     uTemp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_STLOOP_5);
   st0_get_a_reg( i2cAddr, ST0_RID_STLOOP_6);
   st0_get_a_reg( i2cAddr, ST0_RID_STLOOP_7);
   st0_get_a_reg( i2cAddr, ST0_RID_STLOOP_8);
   
   uTemp  = ((u_int32)(gDemReg[ST0_RID_STLOOP_8].value) << 24);
   uTemp += ((u_int32)(gDemReg[ST0_RID_STLOOP_7].value) << 16);
   uTemp += ((u_int32)(gDemReg[ST0_RID_STLOOP_6].value) << 8);
   uTemp += (u_int32)(gDemReg[ST0_RID_STLOOP_5].value);
   *pData = uTemp;
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_crl_init                                                */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the carrier recovery loop           */
/*               by programming the CRL_n registers.                         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_crl_init (u_int8 i2cAddr)
{
   /* Frequency Offset Clear */
   gDemReg[ST0_RID_CRL_3].value = 0x00;
   gDemReg[ST0_RID_CRL_4].value = 0x00;
   gDemReg[ST0_RID_CRL_5].value = 0x00;
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_3);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_4);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_5);
   gDemReg[ST0_RID_CRL_6].value = 0x00;
   gDemReg[ST0_RID_CRL_7].value = 0x00;
   gDemReg[ST0_RID_CRL_8].value = 0x00;
   gDemReg[ST0_RID_CRL_9].value = 0x00;
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_6);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_7);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_8);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_9);
   /* Disable Sweep */
   gDemReg[ST0_RID_CRL_10].value = gDemReg[ST0_RID_CRL_10].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_10);
   return (DEM_OK);
}


DEM_RETURN st0_set_sweep_rate (u_int8 i2cAddr)
{
//   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   gDemReg[ST0_RID_CRL_0].value = 0x35;
//   #else
//   gDemReg[ST0_RID_CRL_0].value = gDemReg[ST0_RID_CRL_0].start;
//   #endif
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_0);
   gDemReg[ST0_RID_CRL_9].value = (gDemReg[ST0_RID_CRL_9].value&0x0F+1<<5);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_9);
   return (DEM_OK);
}

//DEM_RETURN st0_set_sweep_rate (u_int8 i2cAddr)
//{
//   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
//   gDemReg[ST0_RID_CRL_0].value = 0x71;
//   #else
//   gDemReg[ST0_RID_CRL_0].value = gDemReg[ST0_RID_CRL_0].start;
//   #endif
//   st0_set_a_reg( i2cAddr, ST0_RID_CRL_0);
//   gDemReg[ST0_RID_CRL_9].value = gDemReg[ST0_RID_CRL_9].value & 0x0F;
//   st0_set_a_reg( i2cAddr, ST0_RID_CRL_9);
//   return (DEM_OK);
//}

DEM_RETURN st0_set_iphase (u_int8 i2cAddr, u_int32 uIPHASE)
{
   gDemReg[ST0_RID_CRL_6].value  = (u_int8)(uIPHASE & 0x000000FF);
   gDemReg[ST0_RID_CRL_7].value  = (u_int8)((uIPHASE >> 8) & 0x000000FF);
   gDemReg[ST0_RID_CRL_8].value  = (u_int8)((uIPHASE >> 16) & 0x000000FF);
   gDemReg[ST0_RID_CRL_9].value |= (u_int8)((uIPHASE >> 24) & 0x0000000F);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_6);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_7);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_8);
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_9);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_pmfagc_init                                             */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the post-filter digital AGC module  */
/*               by programming the PMFAGC_n registers.                      */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_pmfagc_init (u_int8 i2cAddr)
{
   /* 1: override the internal PMFAGC lock status bit to "unlocked" */
   gDemReg[ST0_RID_PMFAGC_1].value = gDemReg[ST0_RID_PMFAGC_1].value | 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_PMFAGC_1);
   /* 2: reset the PMFAGC accumulator. */
   gDemReg[ST0_RID_PMFAGC_2].value = 0x00;
   gDemReg[ST0_RID_PMFAGC_3].value = 0x00;
   gDemReg[ST0_RID_PMFAGC_4].value = 0x00;
   st0_set_a_reg( i2cAddr, ST0_RID_PMFAGC_2);
   st0_set_a_reg( i2cAddr, ST0_RID_PMFAGC_3);
   st0_set_a_reg( i2cAddr, ST0_RID_PMFAGC_4);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_fec_init                                                */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initialize the FEC modules                     */
/*               by programming the PMFAGC_n registers.                      */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_fec_init (u_int8 i2cAddr)
{
   /* 1: software reset the deinterleaver & descrambler */
   st0_di_swreset (i2cAddr);
   /* 2: force the deinterleaver sync detector to unlock */
   gDemReg[ST0_RID_DEINT_SYNC_0].value = gDemReg[ST0_RID_DEINT_SYNC_0].value | 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_DEINT_SYNC_0);
   gDemReg[ST0_RID_DEINT_SYNC_0].value = gDemReg[ST0_RID_DEINT_SYNC_0].value & 0x7F;
   st0_set_a_reg( i2cAddr, ST0_RID_DEINT_SYNC_0);
   /* 3: set the polarity of the lock indicator signal, and enable the DI_SYNC
    *    & the SYNC interrupts, clear the DI_SYNC & the SYNC interrupt events.
    */
   gDemReg[ST0_RID_CTRL_5].value = 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_CTRL_5);
   /* 4: set the number of states required to unlock */
   gDemReg[ST0_RID_DEINT_SYNC_0].value = gDemReg[ST0_RID_DEINT_SYNC_0].value | 0x02;
   st0_set_a_reg( i2cAddr, ST0_RID_DEINT_SYNC_0);
   /* 5: software reset the Reed-Solomon block */
   st0_rs_swreset (i2cAddr);
   return (DEM_OK);
}


DEM_RETURN st0_acquisition_1 (u_int8 i2cAddr)
{
   /* enable the detection of corner points */
   // gDemReg[ST0_RID_CTRL_8].value = gDemReg[ST0_RID_CTRL_8].value | 0x08;
   // st0_set_a_reg( i2cAddr, ST0_RID_CTRL_8);
   /* unfreeze all blocks that work at symbol rate. */
   st0_stl_unfreeze (i2cAddr);
   /* set the roll value of WBAGC */
   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   gDemReg[ST0_RID_WBAGC_9].value = 0x01; /* high byte of a 12-bit unsigned */
   gDemReg[ST0_RID_WBAGC_4].value = 0x80; /*  low byte of a 12-bit unsigned */
   #else
   gDemReg[ST0_RID_WBAGC_9].value = gDemReg[ST0_RID_WBAGC_9].start; /* high byte of a 12-bit unsigned */
   gDemReg[ST0_RID_WBAGC_4].value = gDemReg[ST0_RID_WBAGC_4].start; /*  low byte of a 12-bit unsigned */
   #endif   
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_9);
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_4);
   /* set the acq_thresh of WBAGC, acq_thresh = 256. */
   gDemReg[ST0_RID_WBAGC_2].value = gDemReg[ST0_RID_WBAGC_2].value & 0xC3;
   // gDemReg[ST0_RID_WBAGC_2].value = gDemReg[ST0_RID_WBAGC_2].value | 0x20;
   gDemReg[ST0_RID_WBAGC_2].value = gDemReg[ST0_RID_WBAGC_2].value | 0x28;
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_2);
   /* enable the WBAGC loop */
   gDemReg[ST0_RID_WBAGC_3].value = gDemReg[ST0_RID_WBAGC_3].value | 0x10;
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_3);
   return (DEM_OK);
}

DEM_RETURN st0_acquisition_2 (u_int8 i2cAddr)
{
   /* set the roll value of WBAGC */
   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   gDemReg[ST0_RID_WBAGC_9].value = 0x12; /* high byte of a 12-bit unsigned */
   gDemReg[ST0_RID_WBAGC_4].value = 0x80; /*  low byte of a 12-bit unsigned */
   #else
   gDemReg[ST0_RID_WBAGC_9].value = gDemReg[ST0_RID_WBAGC_9].start; /* high byte of a 12-bit unsigned */
   gDemReg[ST0_RID_WBAGC_4].value = gDemReg[ST0_RID_WBAGC_4].start; /*  low byte of a 12-bit unsigned */   
   #endif
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_9);
   st0_set_a_reg( i2cAddr, ST0_RID_WBAGC_4);
   /* enable sweep */
   gDemReg[ST0_RID_CRL_10].value = gDemReg[ST0_RID_CRL_10].value | 0x01;
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_10);
   
   /* override the internal PMFAGC lock status bit to "unlocked" */
   gDemReg[ST0_RID_PMFAGC_1].value = gDemReg[ST0_RID_PMFAGC_1].value & 0x7F;
   st0_set_a_reg( i2cAddr, ST0_RID_PMFAGC_1);
   return (DEM_OK);
}

DEM_RETURN st0_acquisition_3 (u_int8 i2cAddr)
{
   /* disable sweep */
   gDemReg[ST0_RID_CRL_10].value = gDemReg[ST0_RID_CRL_10].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_CRL_10);
   /* disable the detection of corner points */
   // gDemReg[ST0_RID_CTRL_8].value = gDemReg[ST0_RID_CTRL_8].value & 0xF7;
   // st0_set_a_reg( i2cAddr, ST0_RID_CTRL_8);
   return (DEM_OK);
}


/*****************************************************************************/
/*  FUNCTION:    st0_start_bert                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               ui8Mode  - the BERT working mode.                           */
/*                                                                           */
/*  DESCRIPTION: The function starts the integrated BERT with the specified  */
/*               mode by programming the bits of BERT_0 register.            */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_start_bert (u_int8 i2cAddr, u_int8 ui8Mode)
{
   u_int8 RegValue;
   RegValue = ui8Mode & 0x1F;
   RegValue = RegValue | 0x80;
   dc2_set_a_reg(i2cAddr, 0xA0, RegValue);
   return (DEM_OK);
}
#else
DEM_RETURN st0_start_bert (u_int8 i2cAddr, u_int8 ui8Mode)
{
   gDemReg[ST0_RID_BERT_0].value = ui8Mode & 0x1F;
   gDemReg[ST0_RID_BERT_0].value = gDemReg[ST0_RID_BERT_0].value | 0x80;
   st0_set_a_reg( i2cAddr, ST0_RID_BERT_0);
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_stop_bert                                               */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function stops the integrated BER tester                */
/*               by programming the BERT_ON bit of BERT_0 register.          */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_stop_bert (u_int8 i2cAddr)
{
   gDemReg[ST0_RID_BERT_0].value = gDemReg[ST0_RID_BERT_0].value & 0x1F;
   st0_set_a_reg( i2cAddr, ST0_RID_BERT_0);
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_bert_status                                         */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the status of the integrated BER tester   */
/*               by reading the bits of BERT_0 register.                     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_bert_status (u_int8 i2cAddr, u_int8 *pData)
{
   dc2_get_a_reg(i2cAddr, 0xA0, pData);
   return (DEM_OK);
}
#else
DEM_RETURN st0_get_bert_status (u_int8 i2cAddr, u_int8 *pData)
{
   st0_get_a_reg( i2cAddr, ST0_RID_BERT_0);
   *pData = gDemReg[ST0_RID_BERT_0].value;
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_bert_error                                          */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets 16-bit internal byte/bit error counter    */
/*               by reading BERT_1 / BERT_2 registers.                       */
/*                                                                           */
/*               This result is the raw bit/byte error count and includes    */
/*               any error falling within the R-S redundancy bytes.          */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_bert_error (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   u_int8	RegValue;

   dc2_get_a_reg(i2cAddr, 0xA2, &RegValue);
   ui16Temp = RegValue << 8;
   dc2_get_a_reg(i2cAddr, 0xA1, &RegValue);
   ui16Temp += RegValue ;
   *pData = ui16Temp;
   return (DEM_OK);
}
DEM_RETURN dc2_get_fec_error (u_int8 i2cAddr)
{
   u_int16     ui16Temp;
   u_int8	RegValue;

   dc2_set_a_reg(i2cAddr,0xDF, 0x81);
   dc2_get_a_reg(i2cAddr, 0xD1, &RegValue);
   ui16Temp = RegValue << 8;
   dc2_get_a_reg(i2cAddr, 0xD0, &RegValue);
   ui16Temp += RegValue ;
//   trace("NUM OF PKT: %2.2x\n", ui16Temp);

   dc2_get_a_reg(i2cAddr, 0xD3, &RegValue);
   ui16Temp = RegValue << 8;
   dc2_get_a_reg(i2cAddr, 0xD2, &RegValue);
   ui16Temp += RegValue ;
  // trace("Corrected PKT: %2.2x\n", ui16Temp);

   dc2_get_a_reg(i2cAddr, 0xD5, &RegValue);
   ui16Temp = RegValue << 8;
   dc2_get_a_reg(i2cAddr, 0xD4, &RegValue);
   ui16Temp += RegValue ;
//   trace("Uncorrected PKT: %2.2x\n", ui16Temp);
   return (DEM_OK);
}
DEM_RETURN dc2_get_mse (u_int8 i2cAddr)
{
   u_int16     ui16Temp;
   u_int8	RegValue;

   dc2_get_a_reg(i2cAddr, 0x08, &RegValue);
   ui16Temp = RegValue << 8;
   dc2_get_a_reg(i2cAddr, 0x07, &RegValue);
   ui16Temp += RegValue ;
 //  trace("MSE: %2.2x\n", ui16Temp);
   dc2_get_locks (i2cAddr);


   return (DEM_OK);
}
DEM_RETURN dc2_get_locks (u_int8 i2cAddr)
{
   u_int16     ui16Temp;
   u_int8	RegValue;

   dc2_get_a_reg(i2cAddr, 0x43, &RegValue);
//   trace("AGC Lock Reg38H: 0x%2.2x\n", RegValue);
   dc2_get_a_reg(i2cAddr, 0x82, &RegValue);
 //  trace("EQU Lock Reg82H: 0x%2.2x\n", RegValue);
   dc2_get_a_reg(i2cAddr, 0x91, &RegValue);
 //  trace("FEC Lock Reg91H: 0x%2.2x\n", RegValue);
   dc2_get_a_reg(i2cAddr, 0xDF, &RegValue);
 //  trace("MPEG Lock RegDFH: 0x%2.2x\n", RegValue);

   return (DEM_OK);
}
#else
DEM_RETURN st0_get_bert_error (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_BERT_1);
   st0_get_a_reg( i2cAddr, ST0_RID_BERT_2);
   
   ui16Temp  = ((u_int16)(gDemReg[ST0_RID_BERT_2].value) << 8);
   ui16Temp += (u_int16)(gDemReg[ST0_RID_BERT_1].value);
   *pData = ui16Temp;
   return (DEM_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    st0_get_lock_status                                         */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data. TRUE - Locked.      */
/*                                                                           */
/*  DESCRIPTION: The function get the current lock status of STV0297         */
/*               by reading the SYNCSTATE bit of RS_DESC_15 register.        */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_get_lock_status (u_int8 i2cAddr, bool *pData)
{
   u_int8 RegValue,reg91H,reg43H;
 //  u_int8 reg3BH,reg3CH,reg35H,reg32H,regd0H,regd1H,regd2H,regd3H,regd4H,regd5H,reg07H,reg08H,reg82H;
   /* read the internal register */
   dc2_get_a_reg( i2cAddr, 0xDF, &RegValue);
   dc2_get_a_reg( i2cAddr, 0x91, &reg91H);
   dc2_get_a_reg( i2cAddr, 0x43, &reg43H);
 /*  dc2_get_a_reg( i2cAddr, 0x3B, &reg3BH);
   dc2_get_a_reg( i2cAddr, 0x3C, &reg3CH);
   dc2_get_a_reg( i2cAddr, 0x35, &reg35H);
   dc2_get_a_reg( i2cAddr, 0x32, &reg32H);
   dc2_get_a_reg( i2cAddr, 0xd0, &regd0H);
   dc2_get_a_reg( i2cAddr, 0xd1, &regd1H);
   dc2_get_a_reg( i2cAddr, 0xd2, &regd2H);
   dc2_get_a_reg( i2cAddr, 0xd3, &regd3H);
   dc2_get_a_reg( i2cAddr, 0xd4, &regd4H);
   dc2_get_a_reg( i2cAddr, 0xd5, &regd5H);
   dc2_get_a_reg( i2cAddr, 0x07, &reg07H);
   dc2_get_a_reg( i2cAddr, 0x08, &reg08H);
   dc2_get_a_reg( i2cAddr, 0x82, &reg82H);
  */
  /* get the lock status */
   if ( ((RegValue & 0x80) == 0x80) && ((reg91H & 0x23) == 0x03) && ((reg43H & 0x08) == 0x08))
   {
      *pData = TRUE;
   }
   else
   {
      *pData = FALSE;
   }
  /*  trace(" register 0xDF is ..............%x\n",RegValue);
    trace(" register 0x91 is ..............%x\n",reg91H);
    trace(" register 0x43 is ..............%x\n",reg43H);
    trace(" register 0x3B is ..............%x\n",reg3BH);
    trace(" register 0x3C is ..............%x\n",reg3CH);
    trace(" register 0x35 is ..............%x\n",reg35H);
    trace(" register 0x32 is ..............%x\n",reg32H);
    trace(" register 0xD0 is ..............%x\n",regd0H);
    trace(" register 0xd1 is ..............%x\n",regd1H);
    trace(" register 0xd2 is ..............%x\n",regd2H);
    trace(" register 0xd3 is ..............%x\n",regd3H);
    trace(" register 0xd4 is ..............%x\n",regd4H);
    trace(" register 0xd5 is ..............%x\n",regd5H);
    trace(" register 0x07 is ..............%x\n",reg07H);
    trace(" register 0x08 is ..............%x\n",reg08H);
    trace(" register 0x82 is ..............%x\n",reg82H);
    */
   return (DEM_OK);
}
#else
DEM_RETURN st0_get_lock_status (u_int8 i2cAddr, bool *pData)
{
   /* read the internal register */
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_15);
// st0_catch_regs (i2cAddr);
   /* get the lock status */
   if ( gDemReg[ST0_RID_RS_DESC_15].value & 0x80 )
   {
      *pData = TRUE;
   }
   else
   {
      *pData = FALSE;
   }
   return (DEM_OK);
}
#endif
/*****************************************************************************/
/*  FUNCTION:    st0_start_ct                                                */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function clears the counters, and then enables them     */
/*               by programming the CT_CLEAR bit of RS_DESC_15 register.     */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Can be called from a non-interrupt or an interrupt context. */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DEM_RETURN dc2_start_ct (u_int8 i2cAddr)
{
   u_int8 RegValue;

   dc2_get_a_reg(i2cAddr, 0xDF, &RegValue);
   RegValue &= 0xFE;
   dc2_set_a_reg(i2cAddr, 0xDF, RegValue);
   RegValue |= 0x01;
   dc2_set_a_reg(i2cAddr, 0xDF, RegValue);
   return (DEM_OK);
}
#else
DEM_RETURN st0_start_ct (u_int8 i2cAddr)
{
   /* Firstly, clear all counters */
   gDemReg[ST0_RID_RS_DESC_15].value = gDemReg[ST0_RID_RS_DESC_15].value & 0xFE;
   st0_set_a_reg( i2cAddr, ST0_RID_RS_DESC_15);
   /* Then, enable all counters */
   gDemReg[ST0_RID_RS_DESC_15].value = gDemReg[ST0_RID_RS_DESC_15].value | 0x01;
   st0_set_a_reg( i2cAddr, ST0_RID_RS_DESC_15);
   return (DEM_OK);
}
#endif
/*****************************************************************************/
/*  FUNCTION:    st0_get_blk_ct                                              */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the 16-bit block counter                  */
/*               by reading RS_DESC_0 / RS_DESC_1 registers.                 */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_blk_ct (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_0);
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_1);
   
   ui16Temp  = ((u_int16)(gDemReg[ST0_RID_RS_DESC_1].value) << 8);
   ui16Temp += (u_int16)(gDemReg[ST0_RID_RS_DESC_0].value);
   *pData = ui16Temp;
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_corr_ct                                             */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the 16-bit corrected block counter        */
/*               by reading RS_DESC_2 / RS_DESC_3 registers.                 */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_corr_ct (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_2);
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_3);
   
   ui16Temp  = ((u_int16)(gDemReg[ST0_RID_RS_DESC_3].value) << 8);
   ui16Temp += (u_int16)(gDemReg[ST0_RID_RS_DESC_2].value);
   *pData = ui16Temp;
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_get_uncorr_ct                                           */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*               pData - the pointer to the return data.                     */
/*                                                                           */
/*  DESCRIPTION: The function gets the 16-bit uncorrected block counter      */
/*               by reading RS_DESC_4 / RS_DESC_5 registers.                 */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_get_uncorr_ct (u_int8 i2cAddr, u_int16 *pData)
{
   u_int16     ui16Temp;
   
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_4);
   st0_get_a_reg( i2cAddr, ST0_RID_RS_DESC_5);
   
   ui16Temp  = ((u_int16)(gDemReg[ST0_RID_RS_DESC_5].value) << 8);
   ui16Temp += (u_int16)(gDemReg[ST0_RID_RS_DESC_4].value);
   *pData = ui16Temp;
   return (DEM_OK);
}

/*****************************************************************************/
/*  FUNCTION:    st0_init_CN_estimation                                      */
/*                                                                           */
/*  PARAMETERS:  i2cAddr  - the i2c address for accessing STV0297 registers. */
/*                                                                           */
/*  DESCRIPTION: The function initializes a look-up table gST0CN[][] and     */
/*               other global variables for C/N estimation.                  */
/*               The look-up table gST0CN[iQ][iD] gives the C/N estimated    */
/*               value for a given QAM size [iQ] and C/N ratio [iD].         */
/*                                                                           */
/*  RETURNS:     DEM_OK if successful, DEM_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEM_RETURN st0_init_CN_estimation (u_int8 i2cAddr)
{
   int32    iQ, iD;
   
   for (iQ=0; iQ<5; iQ++)
   {
      for (iD=0; iD<40; iD++)
      {
         gST0CN[iQ][iD] = 100000;
      }
   }
   /* QAM == 16 */
   iQ = (int32)(ST0_QAM16);
   for (iD=0; iD<15; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((15 - iD) * 1000);
   }
   gST0CN[iQ][15] = 10500;
   gST0CN[iQ][16] =  9000;
   gST0CN[iQ][17] =  8120;
   gST0CN[iQ][18] =  7300;
   gST0CN[iQ][19] =  6530;
   gST0CN[iQ][20] =  5870;
   gST0CN[iQ][21] =  5310;
   gST0CN[iQ][22] =  4790;
   gST0CN[iQ][23] =  4320;
   gST0CN[iQ][24] =  3920;
   gST0CN[iQ][25] =  3590;
   gST0CN[iQ][26] =  3270;
   gST0CN[iQ][27] =  3000;
   gST0CN[iQ][28] =  2760;
   gST0CN[iQ][29] =  2560;
   gST0CN[iQ][30] =  2420;
   gST0CN[iQ][31] =  2260;
   gST0CN[iQ][32] =  2150;
   gST0CN[iQ][33] =  2060;
   gST0CN[iQ][34] =  1980;
   gST0CN[iQ][35] =  1910;
   gST0CN[iQ][36] =  1850;
   gST0CN[iQ][37] =  1810;
   gST0CN[iQ][38] =  1750;
   gST0CN[iQ][39] =  1740;
   /* QAM == 32 */
   iQ = (int32)(ST0_QAM32);
   for (iD=0; iD<18; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((18 - iD) * 1000);
   }
   gST0CN[iQ][18] = 10500;
   gST0CN[iQ][19] =  9120;
   gST0CN[iQ][20] =  8100;
   gST0CN[iQ][21] =  7300;
   gST0CN[iQ][22] =  6560;
   gST0CN[iQ][23] =  5930;
   gST0CN[iQ][24] =  5380;
   gST0CN[iQ][25] =  4920;
   gST0CN[iQ][26] =  4520;
   gST0CN[iQ][27] =  4130;
   gST0CN[iQ][28] =  3800;
   gST0CN[iQ][29] =  3520;
   gST0CN[iQ][30] =  3290;
   gST0CN[iQ][31] =  3120;
   gST0CN[iQ][32] =  2980;
   gST0CN[iQ][33] =  2850;
   gST0CN[iQ][34] =  2730;
   gST0CN[iQ][35] =  2650;
   gST0CN[iQ][36] =  2560;
   gST0CN[iQ][37] =  2510;
   gST0CN[iQ][38] =  2480;
   gST0CN[iQ][39] =  2440;
   /* QAM == 64 */
   iQ = (int32)(ST0_QAM64);
   #ifndef STV0297_CNS
   for (iD=0; iD<21; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((21 - iD) * 1000);
   }
   gST0CN[iQ][21] = 10500;
   gST0CN[iQ][22] =  9300;
   gST0CN[iQ][23] =  8400;
   gST0CN[iQ][24] =  7600;
   gST0CN[iQ][25] =  6850;
   gST0CN[iQ][26] =  6250;
   gST0CN[iQ][27] =  5750;
   gST0CN[iQ][28] =  5250;
   gST0CN[iQ][29] =  4850;
   gST0CN[iQ][30] =  4450;
   gST0CN[iQ][31] =  4200;
   gST0CN[iQ][32] =  3900;
   gST0CN[iQ][33] =  3700;
   gST0CN[iQ][34] =  3550;
   gST0CN[iQ][35] =  3400;
   gST0CN[iQ][36] =  3300;
   gST0CN[iQ][37] =  3200;
   gST0CN[iQ][38] =  3130;
   gST0CN[iQ][39] =  3060;
   #else
   for (iD=0; iD<42; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((42 - iD) * 500);
   }
   gST0CN[iQ][42] = 10500;
   gST0CN[iQ][43] =  9800;
   gST0CN[iQ][44] =  9300;
   gST0CN[iQ][45] =  8800;
   gST0CN[iQ][46] =  8400;
   gST0CN[iQ][47] =  7950;
   gST0CN[iQ][48] =  7600;
   gST0CN[iQ][49] =  7200;
   gST0CN[iQ][50] =  6850;
   gST0CN[iQ][51] =  6500;
   gST0CN[iQ][52] =  6250;
   gST0CN[iQ][53] =  6950;
   gST0CN[iQ][54] =  5750;
   gST0CN[iQ][55] =  5250;
   gST0CN[iQ][56] =  5250;
   gST0CN[iQ][57] =  5050;
   gST0CN[iQ][58] =  4850;
   gST0CN[iQ][59] =  4630;
   gST0CN[iQ][60] =  4450;
   gST0CN[iQ][61] =  4320;
   gST0CN[iQ][62] =  4200;
   gST0CN[iQ][63] =  4040;
   gST0CN[iQ][64] =  3900;
   gST0CN[iQ][65] =  3790;
   gST0CN[iQ][66] =  3700;
   gST0CN[iQ][67] =  3620;
   gST0CN[iQ][68] =  3550;
   gST0CN[iQ][69] =  3470;
   gST0CN[iQ][70] =  3400;
   gST0CN[iQ][71] =  3345;
   gST0CN[iQ][72] =  3300;
   gST0CN[iQ][73] =  3245;
   gST0CN[iQ][74] =  3200;
   gST0CN[iQ][75] =  3160;
   gST0CN[iQ][76] =  3130;
   gST0CN[iQ][77] =  3090;
   gST0CN[iQ][78] =  3060;
   gST0CN[iQ][79] =  3010;
   gST0CN[iQ][80] =  2960;
   gST0CN[iQ][81] =  2910;
   gST0CN[iQ][82] =  2860;
   gST0CN[iQ][83] =  2810;
   gST0CN[iQ][84] =  2760;
   gST0CN[iQ][85] =  2710;
   gST0CN[iQ][86] =  2660;
   gST0CN[iQ][87] =  2610;
   gST0CN[iQ][88] =  2560;
   gST0CN[iQ][89] =  2510;
   gST0CN[iQ][90] =  2460;
   gST0CN[iQ][91] =  2410;
   gST0CN[iQ][92] =  2360;
   gST0CN[iQ][93] =  2310;
   gST0CN[iQ][94] =  2260;
   gST0CN[iQ][95] =  2210;
   gST0CN[iQ][96] =  2160;
   gST0CN[iQ][97] =  2110;
   gST0CN[iQ][98] =  2060;
   gST0CN[iQ][99] =  2010;
   gST0CN[iQ][100] =  1960;
   gST0CN[iQ][101] =  1910;
   gST0CN[iQ][102] =  1860;
   gST0CN[iQ][103] =  1810;
   gST0CN[iQ][104] =  1760;
   gST0CN[iQ][105] =  1710;
   gST0CN[iQ][106] =  1660;
   gST0CN[iQ][107] =  1610;
   #endif
   /* QAM == 128 */
   iQ = (int32)(ST0_QAM128);
   for (iD=0; iD<24; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((24 - iD) * 1000);
   }
   gST0CN[iQ][24] = 10500;
   gST0CN[iQ][25] =  9660;
   gST0CN[iQ][26] =  8780;
   gST0CN[iQ][27] =  7970;
   gST0CN[iQ][28] =  7310;
   gST0CN[iQ][29] =  6750;
   gST0CN[iQ][30] =  6220;
   gST0CN[iQ][31] =  5810;
   gST0CN[iQ][32] =  5430;
   gST0CN[iQ][33] =  5090;
   gST0CN[iQ][34] =  4880;
   gST0CN[iQ][35] =  4700;
   gST0CN[iQ][36] =  4500;
   gST0CN[iQ][37] =  4340;
   gST0CN[iQ][38] =  4270;
   gST0CN[iQ][39] =  4150;
   /* QAM == 256 */
   iQ = (int32)(ST0_QAM256);
   for (iD=0; iD<28; iD++)
   {
      gST0CN[iQ][iD] = 10500 + ((28 - iD) * 1000);
   }
   gST0CN[iQ][28] = 10500;
   gST0CN[iQ][29] =  9600;
   gST0CN[iQ][30] =  9000;
   gST0CN[iQ][31] =  8400;
   gST0CN[iQ][32] =  7800;
   gST0CN[iQ][33] =  7400;
   gST0CN[iQ][34] =  7100;
   gST0CN[iQ][35] =  6700;
   gST0CN[iQ][36] =  6550;
   gST0CN[iQ][37] =  6370;
   gST0CN[iQ][38] =  6200;
   gST0CN[iQ][39] =  6150;
   return (DEM_OK);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         5/26/04 2:54:14 AM     Steven Shen     CR(s) 
 *        9022 9023 : The DEMOD_DCF8722 driver version 1.20. Add the support of
 *         getting the signal strength and Fix some bugs.
 *  2    mpeg      1.1         5/20/04 3:24:11 AM     Steven Shen     CR(s) 
 *        9254 9255 : Modify the function to support the Auto-QAM detection 
 *        mode.
 *  1    mpeg      1.0         3/15/04 10:30:42 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/

