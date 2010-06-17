/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                   Conexant Systems Inc. (c) 2003 - 2004                  */
/*                            Shanghai, CHINA                               */
/*                          All Rights Reserved                             */
/****************************************************************************/
/*
 * Filename:      DRV_8722.C
 *
 * Description:   SGS-THOMSON DCF8722 FrontEnd (Tuner + Demodulator) Driver.
 *                The driver is ONLY for SGS-THOMSON.
 *
 * Author:        Steven Shen
 *
 ****************************************************************************/
/* $Header: DRV_8722.C, 5, 5/31/04 1:10:46 AM, Steven Shen$
 * $Id: DRV_8722.C,v 1.4, 2004-05-31 06:10:46Z, Steven Shen$
 ****************************************************************************/

/***************************/
/*       Header Files      */
/***************************/
#include "stbcfg.h"
#include "kal.h"
#include "iic.h"

/* header file of the cable dcf8722 can tuner driver */
#include "DRV_8722.h"
/* header file of the demodulator chip STV0297 driver */
#include "STV0297.h"
/* header file of the debug output function */
#include "DEBUGINF.h"


/***************************/
/*     Local Constants     */
/***************************/


/***************************/
/*    Global Variables     */
/***************************/
/* the STV0297 internal registers */
#include "REG0297.C"

#if (defined m88dc2000)
double MSELOG[] = {
 0.00000, 3.01030, 4.77121, 6.02060, 6.98970, 7.78151, 8.45098, 9.03090, 9.54243,10.00000,
10.41393,10.79181,11.13943,11.46128,11.76091,12.04120,12.30449,12.55273,12.78754,13.01030,
13.22219,13.42423,13.61728,13.80211,13.97940,14.14973,14.31364,14.47158,14.62398,14.77121,
14.91362,15.05150,15.18514,15.31479,15.44068,15.56303,15.68202,15.79784,15.91065,16.02060,
16.12784,16.23249,16.33468,16.43453,16.53213,16.62758,16.72098,16.81241,16.90196,16.98970,
17.07570,17.16003,17.24276,17.32394,17.40363,17.48188,17.55875,17.63428,17.70852,17.78151,
17.85330,17.92392,17.99341,18.06180,18.12913,18.19544,18.26075,18.32509,18.38849,18.45098,
18.51258,18.57332,18.63323,18.69232,18.75061,18.80814,18.86491,18.92095,18.97627,19.03090
};
#endif

#ifndef STV0297_CNS
extern int32   gST0CN[5][40];
#else
extern int32   gST0CN[5][108];
#endif

CNIM_CFG       gCNimCfg;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*        =======================================================            */
/*        = TUNER OPERATION FUNCTIONS OF CABLE FRONT-END DRIVER =            */
/*        =======================================================            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static DRV_RETURN dcf872x_tuner_write (CNIM_CFG *pCFG);
static DRV_RETURN dcf872x_tuner_set_freq (CNIM_CFG *pCFG, u_int32 uFreq);
static DRV_RETURN dcf872x_tuner_get_freq (CNIM_CFG *pCFG, u_int32 *pData);
static DRV_RETURN dcf872x_tuner_set_cp (CNIM_CFG *pCFG, u_int8 ui8CP);
static DRV_RETURN dcf872x_tuner_get_cp (CNIM_CFG *pCFG, u_int8 *pData);
static DRV_RETURN dcf872x_tuner_set_step (CNIM_CFG *pCFG, u_int32 uStep);
static DRV_RETURN dcf872x_tuner_init (CNIM_CFG *pCFG);

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_write                                         */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*                                                                           */
/*  DESCRIPTION: The function writes the control bytes to the tuner with i2c */
/*               bus (through i2c repeater).                                 */
/*               Before writing, control bytes must be set to value to write */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DRV_RETURN dcf872x_tuner_write (CNIM_CFG *pCFG)
{
   IICTRANS    iicTransBuf;
   u_int8      ui8Data[6];
   u_int8      ui8Cmd[6];
   bool        bReturn;
   
   /* setup data and commands for i2c transaction */
   ui8Data[0] = pCFG->TunerB[0];
   ui8Data[1] = pCFG->TunerB[1];
   ui8Data[2] = pCFG->TunerB[2];
   ui8Data[3] = pCFG->TunerB[3];
   ui8Data[4] = pCFG->TunerB[4];
   ui8Cmd[0]  = IIC_START;
   ui8Cmd[1]  = IIC_DATA;
   ui8Cmd[2]  = IIC_DATA;
   ui8Cmd[3]  = IIC_DATA;
   ui8Cmd[4]  = IIC_DATA;
   ui8Cmd[5]  = IIC_STOP;
   
   iicTransBuf.dwCount = 6;
   iicTransBuf.pData   = ui8Data;
   iicTransBuf.pCmd    = ui8Cmd;
   
   if ( pCFG->TunerI2cRepeater == 1 )
   {
   	#if (defined m88dc2000)
	dc2_start_i2c_repeater (pCFG->DemodAddr);
	#else
	st0_start_i2c_repeater (pCFG->DemodAddr);
	#endif
   }
   
   bReturn = iicTransaction (&iicTransBuf, I2C_BUS_CABLE_FE);
   if (TRUE == bReturn)
   {
      return (DRV_OK);
   }
   else
   {
      return (DRV_ERROR);
   }
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_set_freq                                      */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*               uFreq - the signal frequency to tune                        */
/*                                                                           */
/*  DESCRIPTION: The function set the tuner to the signal frequency F(IN).   */
/*               N = F(OSC)/F(REF) = F(IN)/F(REF) + F(IF)/F(REF)             */
/*                 = F(IN)/F(REF) + TunerIFN                                 */
/*                                                                           */
/*  NOTES:       Bandwidth 8 MHz = 128 * 62500, 6 MHz =  96 * 62500, 1M->16  */
/*               Bandwidth 8 MHz = 256 * 31250, 6 MHz = 192 * 31250, 1M->32  */
/*               Bandwidth 8 MHz = 160 * 50000, 6 MHz = 120 * 62500, 1M->20  */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined TDCH_G101F)
static DRV_RETURN dcf872x_tuner_set_freq (CNIM_CFG *pCFG, u_int32 uFreq)
{
   u_int32  uDivider;
   u_int32  uOffset;
   
   /* calculate the divider */
   uDivider = uFreq / (pCFG->TunerREF);
   uOffset  = uFreq % (pCFG->TunerREF);
   
//   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   if (uOffset > 30000)
   {  /* If the frequency offset is greater than 30KHz, use next frequency. */
      uDivider ++;
   }
//   #endif
   
   uDivider = uDivider + (pCFG->TunerIFN);
   /* set the programmable divider */
   pCFG->TunerB[1] = (u_int8)((uDivider & 0x00007F00) >> 8);
   pCFG->TunerB[2] = (u_int8)(uDivider & 0x000000FF);
   /* get the tuner actual frequency */
   dcf872x_tuner_get_freq (pCFG, &(pCFG->TunerFREQ));
   /* before setting the new tuner charge pump current, save the old value. */
//   dcf872x_tuner_get_cp (pCFG, &(pCFG->TunerCP));
   /* set the tuner charge pump current */
   
	if ((uFreq >446*1000000) && (uFreq <=858*1000000))
	{
		pCFG->TunerB[4] = 0x04;
	}
	else if ((uFreq > 160*1000000) && (uFreq <=446*1000000))
	{
		pCFG->TunerB[4] = 0x02;
	}
	else if ((uFreq >= 51*1000000) && (uFreq <=160*1000000))
	{		
		pCFG->TunerB[4] = 0x01;
	}
	else
	{
		pCFG->TunerB[4] = 0x04;
	}

/////////////////////////////////////////////////////////////////
   
   /* write the configuration bytes to the tuner through the i2c repeater */
   dcf872x_tuner_write (pCFG);
   /*
    * delay enough time to wait for the tuner lock, then go on.
    * the delay time is about 100ms. The delay must be provided!!!
    */
   task_time_sleep ((u_int32)(10));
//    trace(" Tuner frequency is ........%d\n",uFreq);
/*    trace(" TunerB[0] is ..............%x\n",pCFG->TunerB[0]);
    trace(" TunerB[1] is ..............%x\n",pCFG->TunerB[1]);
    trace(" TunerB[2] is ..............%x\n",pCFG->TunerB[2]);
    trace(" TunerB[3] is ..............%x\n",pCFG->TunerB[3]);
    trace(" TunerB[4] is ..............%x\n",pCFG->TunerB[4]);
 */  
   /* recover the old tuner charge pump current. */
//   dcf872x_tuner_set_cp (pCFG, pCFG->TunerCP);
   return (DRV_OK);
}

#else
static DRV_RETURN dcf872x_tuner_set_freq (CNIM_CFG *pCFG, u_int32 uFreq)
{
   u_int32  uDivider;
   u_int32  uOffset;
   
   /* calculate the divider */
   uDivider = uFreq / (pCFG->TunerREF);
   uOffset  = uFreq % (pCFG->TunerREF);
   
//   #if !defined(TOWER_CABLE_TUNER) || (TOWER_CABLE_TUNER==NO)
   if (uOffset > 30000)
   {  /* If the frequency offset is greater than 30KHz, use next frequency. */
      uDivider ++;
   }
//   #endif
   
   uDivider = uDivider + (pCFG->TunerIFN);
   /* set the programmable divider */
   pCFG->TunerB[1] = (u_int8)((uDivider & 0x00007F00) >> 8);
   pCFG->TunerB[2] = (u_int8)(uDivider & 0x000000FF);
   /* get the tuner actual frequency */
   dcf872x_tuner_get_freq (pCFG, &(pCFG->TunerFREQ));
   /* before setting the new tuner charge pump current, save the old value. */
   dcf872x_tuner_get_cp (pCFG, &(pCFG->TunerCP));
   /* set the tuner charge pump current */
   
   #if defined(TOWER_CABLE_TUNER) && (TOWER_CABLE_TUNER==YES)
   dcf872x_tuner_set_cp (pCFG, DCF872x_CP_50);
   #else
   dcf872x_tuner_set_cp (pCFG, DCF872x_CP_220);
   #endif

/////////////////////////////////////////////////////////////////

//   modified by 赖云良on 2005-10-10

//  对于7042/3A Tuner, Control Byte 2 的最低两位根据频率设置
//						P1		P0
// [51 ~149] MHz			1		0
// [149~429] MHz			0		1
// [429~858] MHz			0		0
//    na					1		1

	if ((uFreq >429*1000000) && (uFreq <=858*1000000))
	{
		pCFG->TunerB[4] &= 0xFC;
	}
	else if ((uFreq > 149*1000000) && (uFreq <=429*1000000))
	{
		pCFG->TunerB[4] &= 0xFD;
		pCFG->TunerB[4] |= 0x01;		
	}
	else if ((uFreq >= 51*1000000) && (uFreq <=149*1000000))
	{		
		pCFG->TunerB[4] &= 0xFE;
		pCFG->TunerB[4] |= 0x02;	
	}
	else
	{
		pCFG->TunerB[4] |= 0x03;
	}

/////////////////////////////////////////////////////////////////
   
   /* write the configuration bytes to the tuner through the i2c repeater */
   dcf872x_tuner_write (pCFG);
   /*
    * delay enough time to wait for the tuner lock, then go on.
    * the delay time is about 100ms. The delay must be provided!!!
    */
//   task_time_sleep ((u_int32)(DCF872x_TUNER_DELAY));
   task_time_sleep ((u_int32)(10));

   //    trace(" Tuner frequency is ........%d\n",uFreq);

   /* recover the old tuner charge pump current. */
   dcf872x_tuner_set_cp (pCFG, pCFG->TunerCP);
   return (DRV_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_get_freq                                      */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*               pData - pointer to the return data                          */
/*                                                                           */
/*  DESCRIPTION: The function get the signal frequency F(IN).                */
/*               F(IN) = F(OSC) - F(IF) = Divider * F(STEP) - F(IF)          */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DRV_RETURN dcf872x_tuner_get_freq (CNIM_CFG *pCFG, u_int32 *pData)
{
   u_int32  uDivider;
   
   uDivider = (((u_int32)(pCFG->TunerB[1]) & 0x0000007F) << 8);
   uDivider = uDivider + ((u_int32)(pCFG->TunerB[2]) & 0x000000FF);
   
   if ( uDivider < (pCFG->TunerIFN) )
   {
      return (DRV_ERROR);
   }
   else
   {
      uDivider = uDivider - (pCFG->TunerIFN);
      *pData   = (pCFG->TunerREF) * uDivider;
      return (DRV_OK);
   }
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_set_cp                                        */
/*                                                                           */
/*  PARAMETERS:  pCFG  - pointer to the CNIM_CFG structure                   */
/*               ui8CP - the definition of tuner charge pump current mode    */
/*                                                                           */
/*  DESCRIPTION: The function sets the tuner charge pump current.            */
/*               The tuner default charge pump current is 50 uA.             */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined TDCH_G101F)
static DRV_RETURN dcf872x_tuner_set_cp (CNIM_CFG *pCFG, u_int8 ui8CP)
{
   switch ( ui8CP )
   {
      case DCF872x_CP_220:		// 20uA
         /* TunerB[3] - x0xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xBF;
         break;
      
      case DCF872x_CP_50:		// 100uA
         /* TunerB[3] - x1xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x40;
         break;
      
      default:
         /* TunerB[3] - x1xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x40;
         break;
   }
   return (DRV_OK);
}

#else
static DRV_RETURN dcf872x_tuner_set_cp (CNIM_CFG *pCFG, u_int8 ui8CP)
{
   switch ( ui8CP )
   {
      case DCF872x_CP_220:
         /* TunerB[3] - x1xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x40;
         break;
      
      case DCF872x_CP_50:
         /* TunerB[3] - x0xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xBF;
         break;
      
      default:
         /* TunerB[3] - x0xx xxxx b */
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xBF;
         break;
   }
   return (DRV_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_get_cp                                        */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*               pData - pointer to the return data                          */
/*                                                                           */
/*  DESCRIPTION: The function gets the current tuner charge pump current.    */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined TDCH_G101F)
static DRV_RETURN dcf872x_tuner_get_cp (CNIM_CFG *pCFG, u_int8 *pData)
{
   /* TunerB[3] - x?xx xxxx b */
   if ( pCFG->TunerB[3] & 0x40 )
   {
      *pData = DCF872x_CP_50;
   }
   else
   {
      *pData = DCF872x_CP_220;
   }
   return (DRV_OK);
}

#else
static DRV_RETURN dcf872x_tuner_get_cp (CNIM_CFG *pCFG, u_int8 *pData)
{
   /* TunerB[3] - x?xx xxxx b */
   if ( pCFG->TunerB[3] & 0x40 )
   {
      *pData = DCF872x_CP_220;
   }
   else
   {
      *pData = DCF872x_CP_50;
   }
   return (DRV_OK);
}
#endif

/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_set_step                                      */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*               uStep - the definition of tuner step                        */
/*                                                                           */
/*  DESCRIPTION: The function sets the tuner reference frequency (step).     */
/*               The tuner default IF is 36000000 Hz. (FOR 36 MHz ONLY)      */
/*               The tuner default step is 62500 Hz.                         */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if(defined TDCH_G101F)
static DRV_RETURN dcf872x_tuner_set_step (CNIM_CFG *pCFG, u_int32 uStep)
{
   switch ( uStep )
   {
      case DCF872x_STEP_31250:
         /* TunerB[3] - xxxx x01x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x02;
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xFB;
         pCFG->TunerREF = 31250;
         pCFG->TunerIFN = 1156;  /* TunerIFN = TunerIF / TunerREF */
         break;
      
      case DCF872x_STEP_50000:
         /* TunerB[3] - xxxx xx0x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xFD;
         pCFG->TunerREF = 50000;
         pCFG->TunerIFN = 722;   /* TunerIFN = TunerIF / TunerREF */
         break;
      
      case DCF872x_STEP_62500:
         /* TunerB[3] - xxxx x11x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x06;
         pCFG->TunerREF = 62500;
         pCFG->TunerIFN = 578;   /* TunerIFN = TunerIF / TunerREF */
         break;
      
      default:
         /* TunerB[3] - xxxx x11x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x06;
         pCFG->TunerREF = 62500;
         pCFG->TunerIFN = 578;   /* TunerIFN = TunerIF / TunerREF */
         break;
   }
   return (DRV_OK);
}

#else
static DRV_RETURN dcf872x_tuner_set_step (CNIM_CFG *pCFG, u_int32 uStep)
{
   switch ( uStep )
   {
      case DCF872x_STEP_31250:
         /* TunerB[3] - xxxx x01x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x02;
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xFB;
         pCFG->TunerREF = 31250;
         pCFG->TunerIFN = 1152;  /* TunerIFN = TunerIF / TunerREF */
         break;
      
      case DCF872x_STEP_50000:
         /* TunerB[3] - xxxx xx0x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] & 0xFD;
         pCFG->TunerREF = 50000;
         pCFG->TunerIFN = 720;   /* TunerIFN = TunerIF / TunerREF */
         break;
      
      case DCF872x_STEP_62500:
         /* TunerB[3] - xxxx x11x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x06;
         pCFG->TunerREF = 62500;
         pCFG->TunerIFN = 576;   /* TunerIFN = TunerIF / TunerREF */
         break;
      
      default:
         /* TunerB[3] - xxxx x11x b */
         pCFG->TunerB[3] = pCFG->TunerB[3] | 0x06;
         pCFG->TunerREF = 62500;
         pCFG->TunerIFN = 576;   /* TunerIFN = TunerIF / TunerREF */
         break;
   }
   return (DRV_OK);
}
#endif
/*****************************************************************************/
/*  FUNCTION:    dcf872x_tuner_init                                          */
/*                                                                           */
/*  PARAMETERS:  pCFG - pointer to the CNIM_CFG structure                    */
/*                                                                           */
/*  DESCRIPTION: The function initialize the tuner part in THOMSON DCF872x.  */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined TDCH_G101F)
static DRV_RETURN dcf872x_tuner_init (CNIM_CFG *pCFG)
{
   /* set the tuner output IF in KHz - always 36 MHz */
   pCFG->TunerIF  = 36125;
   /* set the tuner bandwidth in Hz - always 8 MHz */
   pCFG->TunerBW  = 8000000;
   /* the tuner module is controlled by STV0297 i2c repeater */
   pCFG->TunerI2cRepeater = 1;
   /* set the tuner i2c address to 0xC0 */
   pCFG->TunerAddr = 0xC0;
   /* initialize the i2c control bytes for tuner */
   pCFG->TunerB[0] = pCFG->TunerAddr;  /* address byte, always */
   pCFG->TunerB[1] = 0x00;             /* prog. divider byte 1 */
   pCFG->TunerB[2] = 0x00;             /* prog. divider byte 2 */
   pCFG->TunerB[3] = 0x86;             /* control byte 1 */
   pCFG->TunerB[4] = 0x00;             /* control byte 2 */
   pCFG->TunerB[5] = 0x00;             /* readback byte  */
   /* set the tuner reference frequency (step) - 62500 Hz */
   dcf872x_tuner_set_step (pCFG, DCF872x_STEP_62500);
   /* set the tuner charge pump current - 50 uA */
   dcf872x_tuner_set_cp (pCFG, DCF872x_CP_220);
   return (DRV_OK);
}

#else
static DRV_RETURN dcf872x_tuner_init (CNIM_CFG *pCFG)
{
   /* set the tuner output IF in KHz - always 36 MHz */
   pCFG->TunerIF  = 36000;
   /* set the tuner bandwidth in Hz - always 8 MHz */
   pCFG->TunerBW  = 8000000;
   /* the tuner module is controlled by STV0297 i2c repeater */
   pCFG->TunerI2cRepeater = 1;
   /* set the tuner i2c address to 0xC0 */
   pCFG->TunerAddr = 0xC0;
   /* initialize the i2c control bytes for tuner */
   pCFG->TunerB[0] = pCFG->TunerAddr;  /* address byte, always */
   pCFG->TunerB[1] = 0x00;             /* prog. divider byte 1 */
   pCFG->TunerB[2] = 0x00;             /* prog. divider byte 2 */
   pCFG->TunerB[3] = 0x86;             /* control byte 1 */
   pCFG->TunerB[4] = 0x00;             /* control byte 2 */
   pCFG->TunerB[5] = 0x00;             /* readback byte  */
   /* set the tuner reference frequency (step) - 62500 Hz */
   dcf872x_tuner_set_step (pCFG, DCF872x_STEP_62500);
   /* set the tuner charge pump current - 50 uA */
   dcf872x_tuner_set_cp (pCFG, DCF872x_CP_50);
   return (DRV_OK);
}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*****************************************************************************/
/*  FUNCTION:    dcf872x_get_CNE                                             */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*                                                                           */
/*  DESCRIPTION: The function gets the current C/N estimator.                */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if (defined m88dc2000)
DRV_RETURN dcf872x_get_CNE (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int16    MSE;
   u_int32	    MSESUM;
   u_int8 	    i;
   double	    snr;
   
   MSESUM = 0;
   for(i=0;i<30;i++)
   {
   	dc2_get_noise_accumulator(ui8Addr, &MSE);
	MSESUM += MSE;
   }
   MSE = MSESUM / 30;
   if(MSE > 80)
   	MSE = 80;

   dc2_get_qam_size (ui8Addr, (&pCNimCfg->DemodQAM));
   switch(pCNimCfg->DemodQAM)
   {
   	case 0:		// QAM16
   		snr = 34.08;
		break;
   	case 1:		// QAM32
   		snr = 37.60;
		break;
   	case 2:		// QAM128
   		snr = 43.72;
		break;
   	case 3:		// QAM256
   		snr = 46.39;
		break;
   	case 4:		// QAM64
   	default:
   		snr = 40.31;
		break;
   }
   snr = snr - MSELOG[MSE-1];
   snr*=10;
   pCNimCfg->DemodCNE = (u_int32)snr;
   return (DRV_OK);
}

#else
#ifndef STV0297_CNS
DRV_RETURN dcf872x_get_CNE (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int16     ui16Temp;
   int32       i, iQ, iD;
   int32       iCurrentMean, iComputedMean, iOldMean;
   
   st0_get_qam_size (ui8Addr, (&pCNimCfg->DemodQAM));
   iQ = pCNimCfg->DemodQAM;
   
   /* Removed by steven shen. May 19th, 2004 */
   #if (0)
   /* if in MOD_QAMAUTO mode, update the current QAM mode of the tuning SPEC. */
   if (pCNimCfg->SignalQAM == MOD_QAMAUTO)
   {
      switch (iQ)
      {
         case ST0_QAM16:
            pCNimCfg->SignalQAM = MOD_QAM16;
            break;
         case ST0_QAM32:
            pCNimCfg->SignalQAM = MOD_QAM32;
            break;
         case ST0_QAM64:
            pCNimCfg->SignalQAM = MOD_QAM64;
            break;
         case ST0_QAM128:
            pCNimCfg->SignalQAM = MOD_QAM128;
            break;
         case ST0_QAM256:
            pCNimCfg->SignalQAM = MOD_QAM256;
            break;
         default:
            break;
      } /* endswitch (iQ) */
   } /* endif (pCNimCfg->SignalQAM == MOD_QAMAUTO) */
   #endif
   
   /*
    * the STV0297 noise estimation must be filtered. The used filter is:
    * Estimation(n) = 63/64*Estimation(n-1) + 1/64*NoiseEstimation
    */
   for (i=0; i<100; i++)
   {
      st0_get_noise_accumulator (ui8Addr, (&ui16Temp));
      pCNimCfg->DemodCNEmean = (pCNimCfg->DemodCNEmean * 63 + (int32)(ui16Temp)) >> 6;
   }
   
   iComputedMean = pCNimCfg->DemodCNEmean - pCNimCfg->DemodCNEoffset;
   iCurrentMean = gST0CN[iQ][0];
   iD = 1;
   while ( iD < 40 )
   {
      iOldMean = iCurrentMean;
      iCurrentMean = gST0CN[iQ][iD];
      if ( iCurrentMean <= iComputedMean )
      {
         break;
      }
      else
      {
         iD ++;
      }
   }
   pCNimCfg->DemodCNE = iD;
   return (DRV_OK);
}
#else
DRV_RETURN dcf872x_get_CNE (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int16     ui16Temp;
   int32       i, iQ, iD;
   int32       iCurrentMean, iComputedMean, iOldMean;
   
   st0_get_qam_size (ui8Addr, (&pCNimCfg->DemodQAM));
   iQ = pCNimCfg->DemodQAM;
   
   /* Removed by steven shen. May 19th, 2004 */
   #if (0)
   /* if in MOD_QAMAUTO mode, update the current QAM mode of the tuning SPEC. */
   if (pCNimCfg->SignalQAM == MOD_QAMAUTO)
   {
      switch (iQ)
      {
         case ST0_QAM16:
            pCNimCfg->SignalQAM = MOD_QAM16;
            break;
         case ST0_QAM32:
            pCNimCfg->SignalQAM = MOD_QAM32;
            break;
         case ST0_QAM64:
            pCNimCfg->SignalQAM = MOD_QAM64;
            break;
         case ST0_QAM128:
            pCNimCfg->SignalQAM = MOD_QAM128;
            break;
         case ST0_QAM256:
            pCNimCfg->SignalQAM = MOD_QAM256;
            break;
         default:
            break;
      } /* endswitch (iQ) */
   } /* endif (pCNimCfg->SignalQAM == MOD_QAMAUTO) */
   #endif
   
   /*
    * the STV0297 noise estimation must be filtered. The used filter is:
    * Estimation(n) = 63/64*Estimation(n-1) + 1/64*NoiseEstimation
    */
   for (i=0; i<100; i++)
   {
      st0_get_noise_accumulator (ui8Addr, (&ui16Temp));
      pCNimCfg->DemodCNEmean = (pCNimCfg->DemodCNEmean * 63 + (int32)(ui16Temp)) >> 6;
   }
   
   iComputedMean = pCNimCfg->DemodCNEmean - pCNimCfg->DemodCNEoffset;
   iCurrentMean = gST0CN[iQ][0];
   iD = 1;
   while ( iD < 108 )
   {
      iOldMean = iCurrentMean;
      iCurrentMean = gST0CN[iQ][iD];
      if ( iCurrentMean <= iComputedMean )
      {
         break;
      }
      else
      {
         iD ++;
      }
   }
   i = (iD/2)*10;
   i += (iD%2)*5;
   pCNimCfg->DemodCNE = i;
 //  trace("---- CNE: %d; %d\n", pCNimCfg->DemodCNEmean, pCNimCfg->DemodCNE);
   return (DRV_OK);
}
#endif
#endif

/*****************************************************************************/
/*  FUNCTION:    dcf872x_set_bert                                            */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*                                                                           */
/*  DESCRIPTION: The function sets the working mode of the integrated BERT.  */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_set_bert (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8BERTmode = 0x00;
   u_int8      ui8Count;
   
   /* set the BERT source */
   pCNimCfg->DemodBERTsrc = (u_int8)ST0_BERT_BIT_SRC;
   if ( pCNimCfg->DemodBERTsrc == ST0_BERT_BYTE_SRC )
   {
      ui8BERTmode |= 0x10;
   }
   
   /* set the BERT stop mode */
   pCNimCfg->DemodBERTsp = (u_int8)ST0_BERT_AUTO_STOP;
   if ( pCNimCfg->DemodBERTsp == ST0_BERT_MANUAL_STOP )
   {
      ui8BERTmode |= 0x08;
   }
   
   /* set the final BERT working mode */
   pCNimCfg->DemodBERTmode = ui8BERTmode | (u_int8)ST0_BERT_NBYTE;
   
   /* calculate the number of bytes during which errors are to be detected. */
   ui8Count = ((u_int8)(ST0_BERT_NBYTE) << 1) + 12;
   pCNimCfg->DemodBERTnb = ((u_int32)(0x00000001) << ui8Count);
   
   /* clear the waiting flag */
   pCNimCfg->DemodBERTwait = 0;
   
   return (DRV_OK);
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_get_bert                                            */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*                                                                           */
/*  DESCRIPTION: The function calculates the current BER.                    */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_get_bert (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   float       fError, fCount;
   
   #if (defined m88dc2000)
   if (pCNimCfg->DemodBERTwait == 0)
   {
      /* start the BERT */
      dc2_start_bert (ui8Addr, pCNimCfg->DemodBERTmode);
      pCNimCfg->DemodBERTwait = 1;
   }
   else
   {
      /* check whether the BERT is stoped. */
      dc2_get_bert_status (ui8Addr, &(pCNimCfg->DemodBERTst));
      if ( (pCNimCfg->DemodBERTst & 0x80) == 0x00 )
      {
         /* get the BER error counter */
         dc2_get_bert_error (ui8Addr, &(pCNimCfg->DemodBERTerr));
         /* calculate the Bit Error Rate */
         fError = (float)(pCNimCfg->DemodBERTerr);
         fCount = (float)(pCNimCfg->DemodBERTnb);
         pCNimCfg->DemodBER = fError / fCount;
         pCNimCfg->DemodBERTwait = 0;

//         printf("###pCNimCfg->DemodBER:%f\n", pCNimCfg->DemodBER);//eric for mtg 10-12
   //      trace("###pCNimCfg->DemodBER:%f\n", pCNimCfg->DemodBER);//eric for mtg 10-12
         dc2_get_fec_error(ui8Addr);
         dc2_get_mse(ui8Addr);
         dc2_get_locks(ui8Addr);
      }
   }
   #else
   if (pCNimCfg->DemodBERTwait == 0)
   {
      /* start the BERT */
      st0_start_bert (ui8Addr, pCNimCfg->DemodBERTmode);
      pCNimCfg->DemodBERTwait = 1;
   }
   else
   {
      /* check whether the BERT is stoped. */
      st0_get_bert_status (ui8Addr, &(pCNimCfg->DemodBERTst));
      if ( (pCNimCfg->DemodBERTst & 0x80) == 0x00 )
      {
         /* get the BER error counter */
         st0_get_bert_error (ui8Addr, &(pCNimCfg->DemodBERTerr));
         /* calculate the Bit Error Rate */
         fError = (float)(pCNimCfg->DemodBERTerr);
         fCount = (float)(pCNimCfg->DemodBERTnb);
         pCNimCfg->DemodBER = fError / fCount;
         pCNimCfg->DemodBERTwait = 0;
      }
   }
   #endif
   return (DRV_OK);
}

#if (defined m88dc2000)
//const agc2sd[12] = {250, 235, 204, 197, 191, 177, 130, 103, 96, 83, 73 ,2};
//const agc2sd[17] = {390, 379, 369, 360, 320, 296, 290, 280, 260, 221, 194, 181, 174, 138, 126, 115, 103};
const agc2sd[16] = {359, 319, 297, 290, 275, 261, 219, 193, 183, 177, 138, 129, 121, 113, 103, 88};
DRV_RETURN dcf872x_get_strength (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int8 	i;
   
   /* get the current WBAGC agc2sd value. */
   dc2_get_agc2sd (ui8Addr, &(pCNimCfg->Demodagc2sd));
 //   trace("agc2sd = .............%d\n", pCNimCfg->Demodagc2sd);

   /* according to the WBAGC agc2sd value, to estimate the signal strength. */
/*
   if (pCNimCfg->Demodagc2sd >= 250)         
   {
      pCNimCfg->SignalStrength = 35;
   }
   else if (pCNimCfg->Demodagc2sd <= 2)    
   {
      pCNimCfg->SignalStrength = 90;
   }
   else 									
   {
   	for(i=1;i<12;i++)
   	{
   		if(pCNimCfg->Demodagc2sd >= agc2sd[i])
			break;
   	}
      pCNimCfg->SignalStrength = 35 + 5 * (i - 1) + 5 * (u_int32)(agc2sd[i-1] -pCNimCfg->Demodagc2sd) / (agc2sd[i-1] -agc2sd[i]);
   }
*/
//    trace(" SignalStrength is ..............%d\n",pCNimCfg->SignalStrength);
//    pCNimCfg->SignalStrength = pCNimCfg->SignalStrength * 10;


   if (pCNimCfg->Demodagc2sd >= 359)         
   {
      pCNimCfg->SignalStrength = 30;
   }
   else if (pCNimCfg->Demodagc2sd <= 88)    
   {
      pCNimCfg->SignalStrength = 105;
   }
   else 									
   {
   	for(i=1;i<16;i++)
   	{
   		if(pCNimCfg->Demodagc2sd >= agc2sd[i])
			break;
   	}
      pCNimCfg->SignalStrength = 30 + 5 * (i - 1) + 5 * (u_int32)(agc2sd[i-1] -pCNimCfg->Demodagc2sd) / (agc2sd[i-1] -agc2sd[i]);
   }

 //   trace(" SignalStrength is ..............%d\n",pCNimCfg->SignalStrength);
    pCNimCfg->SignalStrength = pCNimCfg->SignalStrength * 10;
//    pCNimCfg->SignalStrength = pCNimCfg->Demodagc2sd;
   return (DRV_OK);
}

#else
#ifndef STV0297_CNS
DRV_RETURN dcf872x_get_strength (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   
   /* get the current WBAGC agc2sd value. */
   st0_wbagc_get_agc2sd (ui8Addr, &(pCNimCfg->Demodagc2sd));
   // debug_out (TL_INFO, "agc2sd = %d\n", pCNimCfg->Demodagc2sd);
   /* according to the WBAGC agc2sd value, to estimate the signal strength. */
   if (pCNimCfg->Demodagc2sd <= 460)         /* > 75 dBuV --- strong signal */
   {
      pCNimCfg->SignalStrength = 255;
   }
   else if (pCNimCfg->Demodagc2sd <= 520)    /* > 35 dBuV --- 255 to 75 */
   {
      pCNimCfg->SignalStrength = 255 - (pCNimCfg->Demodagc2sd - 460) * 3;
   }
   else if (pCNimCfg->Demodagc2sd < 670)     /* < 35 dBuV --- weak signal */
   {
      pCNimCfg->SignalStrength = 75 - (pCNimCfg->Demodagc2sd - 520) / 2;
   }
   else if (pCNimCfg->Demodagc2sd >= 670)    /* no signal or very weak signal */
   {
      pCNimCfg->SignalStrength = 0;
   }
   return (DRV_OK);
}
#else
struct stv0297_strength_s {
    u_int32 RF_Level;
    u_int32 wbagc;
};
#define STV0297_S_MAX 186
   const struct stv0297_strength_s stv0297_s[] = {
       { 165, 102300 },
       { 170, 101893 },
       { 175,  98540 },
       { 180,  92243 },
       { 185,  86017 },
       { 190,  80613 },
       { 195,  76170 },
       { 200,  72593 },
       { 205,  69533 },
       { 210,  67330 },
       { 215,  65830 },
       { 220,  64580 },
       { 225,  63610 },
       { 230,  62823 },
       { 235,  62063 },
       { 240,  61403 },
       { 245,  60853 },
       { 250,  60290 },
       { 255,  59717 },
       { 260,  59180 },
       { 265,  58657 },
       { 270,  58173 },
       { 275,  57743 },
       { 280,  57303 },
       { 285,  56877 },
       { 290,  56477 },
       { 295,  56193 },
       { 300,  55900 },
       { 305,  55467 },
       { 310,  55033 },
       { 315,  54610 },
       { 320,  54183 },
       { 325,  53790 },
       { 330,  53400 },
       { 335,  53000 },
       { 340,  52627 },
       { 345,  51393 },
       { 350,  50140 },
       { 355,  49727 },
       { 360,  49347 },
       { 365,  49017 },
       { 370,  48730 },
       { 375,  48550 },
       { 380,  48440 },
       { 385,  48353 },
       { 390,  48283 },
       { 395,  48253 },
       { 400,  48240 },
       { 405,  48223 },
       { 410,  48200 },
       { 415,  48167 },
       { 420,  48130 },
       { 430,  48073 },
       { 440,  48017 },
       { 450,  47963 },
       { 460,  47890 },
       { 470,  47807 },
       { 480,  47727 },
       { 490,  47643 },
       { 500,  47550 },
       { 505,  47463 },
       { 510,  47403 },
       { 515,  47343 },
       { 520,  47277 },
       { 525,  47227 },
       { 530,  47183 },
       { 535,  47130 },
       { 540,  47080 },
       { 545,  47033 },
       { 550,  46993 },
       { 555,  46920 },
       { 560,  46833 },
       { 565,  46777 },
       { 570,  46707 },
       { 575,  46630 },
       { 580,  46550 },
       { 585,  46463 },
       { 590,  46383 },
       { 595,  46323 },
       { 600,  46250 },
       { 605,  46120 },
       { 610,  45990 },
       { 615,  45883 },
       { 620,  45693 },
       { 625,  45333 },
       { 630,  45033 },
       { 635,  44887 },
       { 640,  44750 },
       { 645,  44630 },
       { 650,  44517 },
       { 655,  44367 },
       { 660,  44197 },
       { 665,  44040 },
       { 670,  43897 },
       { 675,  43783 },
       { 680,  43663 },
       { 685,  43533 },
       { 690,  43440 },
       { 695,  43383 },
       { 700,  43303 },
       { 705,  43197 },
       { 710,  43100 },
       { 715,  43013 },
       { 720,  42930 },
       { 725,  42860 },
       { 730,  42790 },
       { 735,  42713 },
       { 740,  42653 },
       { 745,  42603 },
       { 750,  42540 },
       { 755,  42480 },
       { 760,  42423 },
       { 765,  42373 },
       { 770,  42330 },
       { 775,  42267 },
       { 780,  42217 },
       { 785,  42187 },
       { 790,  42150 },
       { 795,  42113 },
       { 800,  42077 },
       { 805,  42040 },
       { 810,  42010 },
       { 815,  41983 },
       { 820,  41953 },
       { 825,  41920 },
       { 830,  41893 },
       { 835,  41870 },
       { 840,  41847 },
       { 845,  41820 },
       { 850,  41800 },
       { 855,  41767 },
       { 860,  41737 },
       { 865,  41730 },
       { 870,  41713 },
       { 875,  41693 },
       { 880,  41667 },
       { 885,  41643 },
       { 890,  41630 },
       { 895,  41617 },
       { 900,  40950 },
       { 905,  40020 },
       { 910,  39523 },
       { 915,  39060 },
       { 920,  38603 },
       { 925,  38180 },
       { 930,  37757 },
       { 935,  37307 },
       { 940,  36867 },
       { 945,  36433 },
       { 950,  35983 },
       { 955,  35443 },
       { 960,  34917 },
       { 965,  34440 },
       { 970,  33927 },
       { 975,  33440 },
       { 980,  32953 },
       { 985,  32450 },
       { 990,  31980 },
       { 995,  31553 },
       {1000,  31120 },
       {1005,  30587 },
       {1010,  30037 },
       {1015,  29517 },
       {1020,  28983 },
       {1025,  28480 },
       {1030,  27973 },
       {1035,  27433 },
       {1040,  26927 },
       {1045,  26460 },
       {1050,  25957 },
       {1055,  25317 },
       {1060,  24650 },
       {1065,  23980 },
       {1070,  23220 },
       {1075,  22480 },
       {1080,  21667 },
       {1085,  20667 },
       {1090,  19577 },
       {1095,  18217 },
       {1100,  16150 },
       {1105,  12253 },
       {1106,   8763 },
       {1107,   6170 },
       {1108,   2283 },
       {1109,     73 },
       {1110,      0 }
   	};
DRV_RETURN dcf872x_get_strength (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int16 agc;
   u_int32 sum;
   int i, j, k;
   
   /* get the current WBAGC agc2sd value. */
   sum = 0;
   for(i=0; i<10; i++)
   {
   	st0_wbagc_get_agc2sd (ui8Addr, &agc);
   	sum += 100*agc;
   }
   sum = sum / 10;
   if(sum >= stv0297_s[0].wbagc)
   {
   	pCNimCfg->SignalStrength = 0;
   }
   else if(sum <= stv0297_s[STV0297_S_MAX-1].wbagc)
   {
        pCNimCfg->SignalStrength = 150;
   }  
   else
   {
   	i = 0;
   	j = STV0297_S_MAX-1;
   	while(1)   	{
   	      k = (i+j)/2;
   	      if(sum >= stv0297_s[k].wbagc)
   	      	{
   	      	    j = k;
   	      	}
   	      else
   	      	{
   	      	    if(sum >= stv0297_s[k+1].wbagc)
   	      	    {
   	      	    	pCNimCfg->SignalStrength = stv0297_s[k].RF_Level - 10;
   	      	    	break;
   	      	    }
   	      	    i = k;
   	      	}
   	}
  }
 //  trace("----: strength=%d; sum=%d\n", pCNimCfg->SignalStrength, sum);
   return (DRV_OK);
}
#endif
#endif

DRV_RETURN dcf872x_init_statics (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   
   /* initialize the block counts */
   pCNimCfg->DemodBlkCnt = 0;
   pCNimCfg->DemodCorrCnt = 0;
   pCNimCfg->DemodUncorrCnt = 0;
   /* start the block counts */
   #if (defined m88dc2000)
   dc2_start_ct (ui8Addr);
   #else
   st0_start_ct (ui8Addr);
   #endif
   pCNimCfg->DemodCNE = 0;
   pCNimCfg->DemodBERTwait = 0;
   return (DRV_OK);
}

DRV_RETURN dcf872x_get_statics (u_int32 uUnit)
{
   //CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   //u_int8      ui8Addr = pCNimCfg->DemodAddr;
   
   //st0_get_blk_ct (ui8Addr, (&pCNimCfg->DemodBlkCnt));
   //st0_get_corr_ct (ui8Addr, (&pCNimCfg->DemodCorrCnt));
   //st0_get_uncorr_ct (ui8Addr, (&pCNimCfg->DemodUncorrCnt));
   //debug_out (TL_INFO, "   Received Block = %d\n", pCNimCfg->DemodBlkCnt);
   //debug_out (TL_INFO, "  Corrected Block = %d\n", pCNimCfg->DemodCorrCnt);
   //debug_out (TL_INFO, "Uncorrected Block = %d\n", pCNimCfg->DemodUncorrCnt);
   dcf872x_get_CNE (uUnit);
   dcf872x_get_bert (uUnit);
   dcf872x_get_strength (uUnit);
   //debug_out (TL_INFO, "Strength = %d\n", pCNimCfg->SignalStrength);
   return (DRV_OK);
}


DRV_RETURN dcf872x_print_statics (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   
   debug_out (TL_INFO, "======================================\n");
   debug_out (TL_INFO, "   Frequency (Hz) = %d\n", pCNimCfg->SignalFREQ);
   debug_out (TL_INFO, "         QAM Mode = %d\n", pCNimCfg->SignalQAM);
   debug_out (TL_INFO, "SymbolRate (Baud) = %d\n", pCNimCfg->SignalSR);
   if (DEM_LOCKED == pCNimCfg->LockStatus.CurrSYNC)
   {
      debug_out (TL_INFO, "             SYNC = YES\n");
      debug_out (TL_INFO, "   Received Block = %d\n", pCNimCfg->DemodBlkCnt);
      debug_out (TL_INFO, "  Corrected Block = %d\n", pCNimCfg->DemodCorrCnt);
      debug_out (TL_INFO, "Uncorrected Block = %d\n", pCNimCfg->DemodUncorrCnt);
      debug_out (TL_INFO, "              CNE = %d\n", pCNimCfg->DemodCNE);
      debug_out (TL_INFO, "              BER = %f\n", pCNimCfg->DemodBER);
   }
   else
   {
      debug_out (TL_INFO, "             SYNC = NO\n");
   }
   debug_out (TL_INFO, "======================================\n\n");
   return (DRV_OK);
}


/*****************************************************************************/
/*  FUNCTION:    dcf872x_get_lockstatus                                      */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*               pLocked - pointer to a boolean to indicate the lock status  */
/*                                                                           */
/*  DESCRIPTION: The function get the current lock status.                   */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_get_lockstatus (u_int32 uUnit, bool *pLocked)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   
   /* get the lock status of the STV0297 QAM demodulator */
   #if (defined m88dc2000)
   dc2_get_lock_status ((pCNimCfg->DemodAddr), pLocked);
   #else
   st0_get_lock_status ((pCNimCfg->DemodAddr), pLocked);
   #endif
   if (TRUE == *pLocked)
   {
      if (DEM_LOCKED != pCNimCfg->LockStatus.CurrSYNC)
      {
         pCNimCfg->LockStatus.PrevSYNC = pCNimCfg->LockStatus.CurrSYNC;
         pCNimCfg->LockStatus.CurrSYNC = DEM_LOCKED;
      }
   }
   else
   {
      if (DEM_LOCKED == pCNimCfg->LockStatus.CurrSYNC)
      {
         pCNimCfg->LockStatus.PrevSYNC = pCNimCfg->LockStatus.CurrSYNC;
         pCNimCfg->LockStatus.CurrSYNC = DEM_LOCK_LOSS;
         pCNimCfg->LockStatus.uNumLOSS ++;
      }
   }
   return (DRV_OK);
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_set_symbol_rate                                     */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*                                                                           */
/*  DESCRIPTION: The function sets THOMSON Cable Front-End DCF8722 to        */
/*               the specified symbol rate (0.87 < SR < 11.7).               */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_set_symbol_rate (u_int32 uUnit)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int32     uSymbolRate;
   #if (defined m88dc2000)
   u_int32     uSymbolRateInv;
   u_int8	LowSym;
//   u_int8	DDSStep;
   u_int8	reg6FH, reg12H;
   double       fTemp;

   fTemp = (double)(pCNimCfg->SignalSR + 10000) / (double)(pCNimCfg->DemodClkExt);
   fTemp = fTemp * 4294967.296;
   uSymbolRate = (u_int32) (fTemp);
   
   fTemp = (double)(pCNimCfg->DemodClkExt) * 1000.0 / (double)(pCNimCfg->SignalSR);
   fTemp = fTemp * 2048.0;
   uSymbolRateInv = (u_int32) (fTemp);

//   if((pCNimCfg->SignalSR) <= 1.8)
   if((pCNimCfg->SignalSR) <= 1800000)
   	LowSym = 1;
   else
   	LowSym = 0;
/*   
   fTemp = (double)(pCNimCfg->SignalSR) * 15.0 / 14.0;
   DDSStep = (u_int8) (fTemp);
   if(DDSStep == 0)
   	DDSStep = 1;
*/
   if((pCNimCfg->SignalSR) >= 6700000)
   {
   	reg6FH = 0x0D;
	reg12H = 0x30;
   }
   else if((pCNimCfg->SignalSR) >= 4000000)
   {
   	fTemp = (double)(pCNimCfg->SignalSR) / 1000000;
	fTemp = 22 * 4.096 / fTemp;
   	reg6FH = (u_int8) fTemp;
	reg12H = 0x30;
   }
   else if((pCNimCfg->SignalSR) >= 2000000)
   {
   	fTemp = (double)(pCNimCfg->SignalSR) / 1000000;
	fTemp = 14 * 4.096 / fTemp;
   	reg6FH = (u_int8) fTemp;
	reg12H = 0x20;
   }
   else
   {
   	fTemp = (double)(pCNimCfg->SignalSR) / 1000000;
	fTemp = 7 * 4.096 / fTemp;
   	reg6FH = (u_int8) fTemp;
	reg12H = 0x10;
   }

   pCNimCfg->DemodSR = uSymbolRate;
//   dc2_set_symbol_rate ((pCNimCfg->DemodAddr), uSymbolRate, uSymbolRateInv, LowSym, DDSStep);
   dc2_set_symbol_rate ((pCNimCfg->DemodAddr), uSymbolRate, uSymbolRateInv, LowSym, reg6FH, reg12H);
   
   #else
   u_int32     uTemp;
   float       fTemp;
   
   if ( pCNimCfg->SignalSR % 1000 )
   {
      uTemp = pCNimCfg->SignalSR / 1000;
      if (uTemp > 8192L) /* 2^13 */
      {
         uTemp = (uTemp << 18);  /* plus 2^18 */
         uTemp = uTemp / (pCNimCfg->DemodClkExt);
         uTemp = (uTemp << 14);  /* plus 2^14 */
      }
      else if (uTemp > 4096) /* 2^12 */
      {
         uTemp = (uTemp << 19);  /* plus 2^19 */
         uTemp = uTemp / (pCNimCfg->DemodClkExt);
         uTemp = (uTemp << 13);  /* plus 2^13 */
      }
      else if (uTemp > 2048) /* 2^11 */
      {
         uTemp = (uTemp << 20);  /* plus 2^20 */
         uTemp = uTemp / (pCNimCfg->DemodClkExt);
         uTemp = (uTemp << 12);  /* plus 2^12 */
      }
      else if (uTemp > 1024) /* 2^10 */
      {
         uTemp = (uTemp << 21);  /* plus 2^21 */
         uTemp = uTemp / (pCNimCfg->DemodClkExt);
         uTemp = (uTemp << 11);  /* plus 2^11 */
      }
      else if (uTemp > 512) /* 2^9 */
      {
         uTemp = (uTemp << 22);  /* plus 2^22 */
         uTemp = uTemp / (pCNimCfg->DemodClkExt);
         uTemp = (uTemp << 10);  /* plus 2^10 */
      }
      else
      {
         /* Never be here. Symbol rate is error. */
         return (DRV_ERROR);
      }
      uSymbolRate = uTemp;
   }
   else
   {
      fTemp = (float)(pCNimCfg->SignalSR) / (float)(pCNimCfg->DemodClkExt);
      fTemp = (fTemp * 4194304.0) / 1000.0;
      fTemp = fTemp * 1024.0;
      uSymbolRate = (u_int32)(fTemp);
   }
   
   pCNimCfg->DemodSR = uSymbolRate;
   st0_set_symbol_rate ((pCNimCfg->DemodAddr), uSymbolRate);
   #endif
   return (DRV_OK);
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_reinit                                              */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to re-initialize.          */
/*                                                                           */
/*  DESCRIPTION: The function re-initializes THOMSON Cable Front-End DCF8722 */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_reinit (u_int32 uUnit)
{
   CNIM_CFG    *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   
   #if (defined m88dc2000)
   /* software reset all internal modules of the demodulator. */
   dc2_global_soft_reset (ui8Addr);
   /* initialize all demodulator internal registers with default values */
   dc2_init_regs (ui8Addr);
      
   #else
   /* software reset all internal modules of the demodulator. */
   st0_swreset (ui8Addr);
   /* initialize all demodulator internal registers with default values */
   st0_init_regs (ui8Addr);
   
   /* check the option of the initial quadrature demodulator */
   if ( pCNimCfg->DemodINITDEM == ST0_INITDEM_ENABLE )
   {  /* if the initial quadrature demodulator is enabled, configure it */
      /* NOT IMPLEMENTED */
   }
   else
   {
      st0_disable_initdem (ui8Addr);
   }
   
   /* initialize the WB AGC */
   st0_wbagc_init (ui8Addr);
   /* initialize the PMF AGC */
   st0_pmfagc_init (ui8Addr);
   /* initialize Symbol Timing Loop */
   st0_stl_init (ui8Addr);
   /* initialize Carrier Recovery Loop */
   st0_crl_init (ui8Addr);
   /* initialize Equalizer */
   st0_equ_init (ui8Addr);
   /* initialize FEC */
   st0_fec_init (ui8Addr);
   #endif
   
   /* initialize the lock status */
   pCNimCfg->LockStatus.PrevSYNC = DEM_NO_LOCKED;
   pCNimCfg->LockStatus.CurrSYNC = DEM_NO_LOCKED;
   pCNimCfg->LockStatus.uNumLOSS = 0;
   
   return (DRV_OK);
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_connect                                             */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to connect.                */
/*               pTuning - pointer to the TUNING_SPEC structure containing   */
/*                         parameters for the requested connection.          */
/*               bForced - if TRUE, to set the parameters is forced.         */
/*                                                                           */
/*  DESCRIPTION: The function connects THOMSON Cable Front-End DCF8722 to    */
/*               the specified signal.                                       */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_BAD_SIGNAL if unsuccessful.       */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
#if 0   //eric 1010 for mtg
//#if defined(TOWER_CABLE_TUNER) && (TOWER_CABLE_TUNER==YES)

DRV_RETURN dcf872x_connect (u_int32 uUnit, TUNING_SPEC *pTuning, bool bForced)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   
   /**************************************************************************/
   /*   Check and Store the parameters of the specified signal to be tuned.
    */
   if (TRUE == bForced)
   {
      pCNimCfg->SignalFREQ = pTuning->tune.nim_cable_tune.frequency;
      pCNimCfg->SignalSR = pTuning->tune.nim_cable_tune.symbol_rate;
      pCNimCfg->SignalQAM = pTuning->tune.nim_cable_tune.modulation;
      pCNimCfg->SignalAutoSP = pTuning->tune.nim_cable_tune.auto_spectrum;
      pCNimCfg->SignalSI = pTuning->tune.nim_cable_tune.spectrum;
      
      pCNimCfg->TunerFlag = 0x01;
      pCNimCfg->DemodFlag = 0x07;
   }
   else
   {
      pCNimCfg->TunerFlag = 0x00;
      pCNimCfg->DemodFlag = 0x00;
      
      /* check the Annex mode, ITU-J83 Annex mode must be always ANNEX_A */
      if ( pTuning->tune.nim_cable_tune.annex != ANNEX_A )
      {
         return (DRV_BAD_SIGNAL);
      }
      
      pCNimCfg->SignalAutoSP = pTuning->tune.nim_cable_tune.auto_spectrum;
      
      /* check the signal frequency */
      if ( pTuning->tune.nim_cable_tune.frequency != pCNimCfg->SignalFREQ )
      {  /* if the frequency of the signal to be tuned is different. */
         /* store the new frequency */
         pCNimCfg->SignalFREQ = pTuning->tune.nim_cable_tune.frequency;
         /* set the changing flag */
         pCNimCfg->TunerFlag = 0x01;
      }
      /* check the QAM size */
      if ( pTuning->tune.nim_cable_tune.modulation != pCNimCfg->SignalQAM )
      {  /* if the QAM size of the signal to be tuned is different. */
         /* store the new QAM size */
         pCNimCfg->SignalQAM = pTuning->tune.nim_cable_tune.modulation;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x01;
      }
      /* check the symbol rate */
      if ( pTuning->tune.nim_cable_tune.symbol_rate != pCNimCfg->SignalSR )
      {  /* if the symbol rate of the signal to be tuned is different. */
         /* store the new symbol rate */
         pCNimCfg->SignalSR = pTuning->tune.nim_cable_tune.symbol_rate;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x02;
      }
      /* check the spectrum inversion option */
      if ( pTuning->tune.nim_cable_tune.spectrum != pCNimCfg->SignalSI )
      {  /* if the spectrum inversion option is different. */
         /* store the new spectrum inversion option */
         pCNimCfg->SignalSI = pTuning->tune.nim_cable_tune.spectrum;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x04;
      }
   }
   
   /*
   debug_out (TL_INFO, "CONNECT Param: freq    = %d\n", pCNimCfg->SignalFREQ);
   debug_out (TL_INFO, "CONNECT Param: QAM     = %d\n", pCNimCfg->SignalQAM);
   debug_out (TL_INFO, "CONNECT Param: sym rate= %d\n", pCNimCfg->SignalSR);
   debug_out (TL_INFO, "CONNECT Param: auto_sp = %d\n", pCNimCfg->SignalAutoSP);
   debug_out (TL_INFO, "CONNECT Param: spec inv= %d\n", pCNimCfg->SignalSI);
   debug_out (TL_INFO, "CONNECT Param: annex   = %d\n", pCNimCfg->SignalAnnex);
   */
   
   /**************************************************************************/
   /*   Check the changing flag and Apply the new parameters of the signal.
    */
   if (pCNimCfg->TunerFlag)
   {  /* if the tuner frequency needs change, set tuner to the new frequency.
       */
      dcf872x_tuner_set_freq (pCNimCfg, pCNimCfg->SignalFREQ);
   }
   
   /* always reinitialize the QAM demodulator */
   dcf872x_reinit (uUnit);
   /* if the QAM size has been changed, set to the new QAM size */
   if (pCNimCfg->DemodFlag & 0x01)
   {
      switch ( pCNimCfg->SignalQAM )
      {
         case MOD_QAMAUTO:
            pCNimCfg->DemodQAM = /*ST0_QAM64;*/ST0_QAM_AUTO;
         //   trace("qam:auto!\n");
            break;
        
         case MOD_QAM16:
            pCNimCfg->DemodQAM = ST0_QAM16;
           // trace("qam:16!\n");
            break;
         
         case MOD_QAM32:
            pCNimCfg->DemodQAM = ST0_QAM32;
       //     trace("qam:32!\n");
            break;
         
         case MOD_QAM64:
            pCNimCfg->DemodQAM = ST0_QAM64;
       //     trace("qam:64!\n");
            break;
         
         case MOD_QAM128:
            pCNimCfg->DemodQAM = ST0_QAM128;
        //    trace("qam:128!\n");
            break;
         
         case MOD_QAM256:
            pCNimCfg->DemodQAM = ST0_QAM256;
         //   trace("qam:256!\n");
            break;
         
         default:
            pCNimCfg->DemodQAM = /*ST0_QAM64;*/ST0_QAM_UNSUPPORTED;
        //    trace("qam:unsupport!\n");
            return (DRV_BAD_SIGNAL);
      }
   }
   
   st0_set_qam_size (ui8Addr, (pCNimCfg->DemodQAM));
   st0_stl_unfreeze (ui8Addr);   

   /* if the symbol rate has been changed, set to the new symbol rate */
   if (pCNimCfg->DemodFlag & 0x02)
   {
//      dcf872x_set_symbol_rate (uUnit);
//      st0_set_sweep_rate (ui8Addr);
//      st0_stl_freeze (ui8Addr);
//      st0_set_iphase (ui8Addr, 0x0001C49E);
   }

   dcf872x_set_symbol_rate (uUnit);
   st0_set_sweep_rate (ui8Addr);
   st0_stl_freeze (ui8Addr);
   st0_set_iphase (ui8Addr, 0x0001C49E);
      
      
   /* if the spectrum inversion option has been changed, set it. */
   if (pCNimCfg->DemodFlag & 0x04)
   {
      if ( pCNimCfg->SignalSI == SPECTRUM_NORMAL )
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_NORMAL;
      }
      else
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_INVERSION;
      }
      //st0_set_spec_inv (ui8Addr, (pCNimCfg->DemodSI));
   }
   
   st0_set_spec_inv (ui8Addr, (pCNimCfg->DemodSI));
   /* start the acquisition phase 1 processing */
   st0_acquisition_1 (ui8Addr);
   /* waiting for about 30 ms after the acquisition phase 1 processing */
   task_time_sleep ((u_int32)(DCF872x_ACQP1_DELAY));
   /* check the WBAGC lock status, and get the agc2sd value. */
   st0_wbagc_get_acq (ui8Addr, &(pCNimCfg->DemodWBAGCLock));
   if (ST0_WBAGC_LOCKED == pCNimCfg->DemodWBAGCLock)
   {
      dcf872x_get_strength (uUnit);
   }
   /* start the acquisition phase 2 processing */
   st0_acquisition_2 (ui8Addr);
   /* waiting for about 30 ms after the acquisition phase 2 processing */
   task_time_sleep ((u_int32)(DCF872x_ACQP2_DELAY));
   /* start the acquisition phase 3 processing */
   st0_acquisition_3 (ui8Addr);
   
   return (DRV_OK);
}

#else /*#if defined(TOWER_CABLE_TUNER) && (TOWER_CABLE_TUNER==YES)*/

DRV_RETURN dcf872x_connect (u_int32 uUnit, TUNING_SPEC *pTuning, bool bForced)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr = pCNimCfg->DemodAddr;
   u_int8	i,j,regE3H,regE4H,value;
   
   /**************************************************************************/
   /*   Check and Store the parameters of the specified signal to be tuned.
    */
   if (TRUE == bForced)
   {
      pCNimCfg->SignalFREQ = pTuning->tune.nim_cable_tune.frequency;
      pCNimCfg->SignalSR = pTuning->tune.nim_cable_tune.symbol_rate;
      pCNimCfg->SignalQAM = pTuning->tune.nim_cable_tune.modulation;
      pCNimCfg->SignalAutoSP = pTuning->tune.nim_cable_tune.auto_spectrum;
      pCNimCfg->SignalSI = pTuning->tune.nim_cable_tune.spectrum;
      
      pCNimCfg->TunerFlag = 0x01;
      pCNimCfg->DemodFlag = 0x07;
   }
   else
   {
      pCNimCfg->TunerFlag = 0x00;
      pCNimCfg->DemodFlag = 0x00;
      
      /* check the Annex mode, ITU-J83 Annex mode must be always ANNEX_A */
      if ( pTuning->tune.nim_cable_tune.annex != ANNEX_A )
      {
         return (DRV_BAD_SIGNAL);
      }
      
      pCNimCfg->SignalAutoSP = pTuning->tune.nim_cable_tune.auto_spectrum;
      
      /* check the signal frequency */
      if ( pTuning->tune.nim_cable_tune.frequency != pCNimCfg->SignalFREQ )
      {  /* if the frequency of the signal to be tuned is different. */
         /* store the new frequency */
         pCNimCfg->SignalFREQ = pTuning->tune.nim_cable_tune.frequency;
         /* set the changing flag */
         pCNimCfg->TunerFlag = 0x01;
      }
      /* check the QAM size */
      if ( pTuning->tune.nim_cable_tune.modulation != pCNimCfg->SignalQAM )
      {  /* if the QAM size of the signal to be tuned is different. */
         /* store the new QAM size */
         pCNimCfg->SignalQAM = pTuning->tune.nim_cable_tune.modulation;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x01;
      }
      /* check the symbol rate */
      if ( pTuning->tune.nim_cable_tune.symbol_rate != pCNimCfg->SignalSR )
      {  /* if the symbol rate of the signal to be tuned is different. */
         /* store the new symbol rate */
         pCNimCfg->SignalSR = pTuning->tune.nim_cable_tune.symbol_rate;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x02;
      }
      /* check the spectrum inversion option */
      if ( pTuning->tune.nim_cable_tune.spectrum != pCNimCfg->SignalSI )
      {  /* if the spectrum inversion option is different. */
         /* store the new spectrum inversion option */
         pCNimCfg->SignalSI = pTuning->tune.nim_cable_tune.spectrum;
         /* set the changing flag */
         pCNimCfg->DemodFlag |= 0x04;
      }
   }
   
   /*
   debug_out (TL_INFO, "CONNECT Param: freq    = %d\n", pCNimCfg->SignalFREQ);
   debug_out (TL_INFO, "CONNECT Param: QAM     = %d\n", pCNimCfg->SignalQAM);
   debug_out (TL_INFO, "CONNECT Param: sym rate= %d\n", pCNimCfg->SignalSR);
   debug_out (TL_INFO, "CONNECT Param: auto_sp = %d\n", pCNimCfg->SignalAutoSP);
   debug_out (TL_INFO, "CONNECT Param: spec inv= %d\n", pCNimCfg->SignalSI);
   debug_out (TL_INFO, "CONNECT Param: annex   = %d\n", pCNimCfg->SignalAnnex);
   */
   
   /**************************************************************************/
   /*   Check the changing flag and Apply the new parameters of the signal.
    */
   if (pCNimCfg->TunerFlag)
   {  /* if the tuner frequency needs change, set tuner to the new frequency.
       */
      dcf872x_tuner_set_freq (pCNimCfg, pCNimCfg->SignalFREQ);
   }
   
   #if (defined m88dc2000)
   /* always reinitialize the QAM demodulator */
   dcf872x_reinit (uUnit);
   /* if the QAM size has been changed, set to the new QAM size */
   if (pCNimCfg->DemodFlag & 0x01)
   {
      switch ( pCNimCfg->SignalQAM )
      {
       case MOD_QAMAUTO:
            pCNimCfg->DemodQAM =ST0_QAM_AUTO;
			trace("qam:Auto!\n");
            break;
         
         case MOD_QAM16:
            pCNimCfg->DemodQAM = ST0_QAM16;
			trace("qam:16!\n");
            break;
         
         case MOD_QAM32:
            pCNimCfg->DemodQAM = ST0_QAM32;
			trace("qam:32!\n");
            break;
         
         case MOD_QAM64:
            pCNimCfg->DemodQAM = ST0_QAM64;
	//		trace("qam:64!\n");
            break;
         
         case MOD_QAM128:
            pCNimCfg->DemodQAM = ST0_QAM128;
			trace("qam:128!\n");
            break;
         
         case MOD_QAM256:
            pCNimCfg->DemodQAM = ST0_QAM256;
			trace("qam:256!\n");
            break;
         
         default:
            pCNimCfg->DemodQAM = ST0_QAM_UNSUPPORTED;
			trace("qam:unsupport!\n");
            return (DRV_BAD_SIGNAL);
      }
      
      dc2_set_qam_size (ui8Addr, (pCNimCfg->DemodQAM));
   }
   /* if the symbol rate has been changed, set to the new symbol rate */
   if (pCNimCfg->DemodFlag & 0x02)
   {
      dcf872x_set_symbol_rate (uUnit);
   }
   /* if the spectrum inversion option has been changed, set it. */
   if (pCNimCfg->DemodFlag & 0x04)
   {
      if ( pCNimCfg->SignalSI == SPECTRUM_NORMAL )
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_NORMAL;
      }
      else
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_INVERSION;
      }
      dc2_set_spec_inv (ui8Addr, (pCNimCfg->DemodSI));
   }

//   dcf872x_get_strength (uUnit);

   dc2_soft_reset(ui8Addr);
  //  trace(" M88DC2000 SoftReset..............\n");

#if (defined TDCH_G101F)
   for(i=0;i<4;i++)
   {
   	task_time_sleep(50);
      dc2_get_agc_lock(ui8Addr, &(pCNimCfg->DemodWBAGCLock));
      if (ST0_WBAGC_LOCKED == pCNimCfg->DemodWBAGCLock)
      	{		
      		extern DEM_RETURN dc2_set_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 ui8RegValue);
		extern DEM_RETURN dc2_get_a_reg (u_int8 ui8Addr, u_int8 ui8RegAddr, u_int8 *pRegValue);

		dcf872x_get_strength (uUnit);
		   dc2_get_a_reg( ui8Addr, 0xE3, &regE3H);
		   dc2_get_a_reg( ui8Addr, 0xE4, &regE4H);
		   if (((regE3H & 0xC0) == 0x00) && ((regE4H & 0xC0) == 0x00))
		   {
			   dc2_get_a_reg( ui8Addr, 0x55, &value);
		   }
		   else
		   {
			   dc2_get_a_reg( ui8Addr, 0x3B, &value);
		   }
		if(value <= 0x86)
		{
			   dc2_set_a_reg(ui8Addr, 0x32, 0x7F);
			   dc2_soft_reset(ui8Addr);
			   for(j=0;j<4;j++)
			   {
			   	task_time_sleep(50);
			      dc2_get_agc_lock(ui8Addr, &(pCNimCfg->DemodWBAGCLock));
			      if (ST0_WBAGC_LOCKED == pCNimCfg->DemodWBAGCLock)
			      	{
				//	trace(" j =  ..............%x\n",j);
				  	break;
			      	}
			   }
		}
//		trace(" i =  ..............%x\n",i);
	  	break;
      	}
   }
  #else
   for(i=0;i<4;i++)
   {
   	task_time_sleep(50);
      dc2_get_agc_lock(ui8Addr, &(pCNimCfg->DemodWBAGCLock));
      if (ST0_WBAGC_LOCKED == pCNimCfg->DemodWBAGCLock)
      	{
		dcf872x_get_strength (uUnit);
	//	trace(" i =  ..............%x\n",i);
	  	break;
      	}
   }
  #endif
      
   #else
   /* always reinitialize the QAM demodulator */
   dcf872x_reinit (uUnit);
   /* if the QAM size has been changed, set to the new QAM size */
   if (pCNimCfg->DemodFlag & 0x01)
   {
      switch ( pCNimCfg->SignalQAM )
      {
         case MOD_QAMAUTO:
            pCNimCfg->DemodQAM = ST0_QAM_AUTO;
            break;
         
         case MOD_QAM16:
            pCNimCfg->DemodQAM = ST0_QAM16;
            break;
         
         case MOD_QAM32:
            pCNimCfg->DemodQAM = ST0_QAM32;
            break;
         
         case MOD_QAM64:
            pCNimCfg->DemodQAM = ST0_QAM64;
            break;
         
         case MOD_QAM128:
            pCNimCfg->DemodQAM = ST0_QAM128;
            break;
         
         case MOD_QAM256:
            pCNimCfg->DemodQAM = ST0_QAM256;
            break;
         
         default:
            pCNimCfg->DemodQAM = ST0_QAM_UNSUPPORTED;
            return (DRV_BAD_SIGNAL);
      }
      
      st0_set_qam_size (ui8Addr, (pCNimCfg->DemodQAM));
      st0_stl_unfreeze (ui8Addr);
   }
   /* if the symbol rate has been changed, set to the new symbol rate */
   if (pCNimCfg->DemodFlag & 0x02)
   {
      dcf872x_set_symbol_rate (uUnit);
      st0_set_sweep_rate (ui8Addr);
      st0_stl_freeze (ui8Addr);
      st0_set_iphase (ui8Addr, 0x0001C49E);
   }
   /* if the spectrum inversion option has been changed, set it. */
   if (pCNimCfg->DemodFlag & 0x04)
   {
      if ( pCNimCfg->SignalSI == SPECTRUM_NORMAL )
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_NORMAL;
      }
      else
      {
         pCNimCfg->DemodSI = ST0_SPECTRUM_INVERSION;
      }
      st0_set_spec_inv (ui8Addr, (pCNimCfg->DemodSI));
   }
   /* start the acquisition phase 1 processing */
   st0_acquisition_1 (ui8Addr);
   /* waiting for about 30 ms after the acquisition phase 1 processing */
   task_time_sleep ((u_int32)(DCF872x_ACQP1_DELAY));
   /* check the WBAGC lock status, and get the agc2sd value. */
   st0_wbagc_get_acq (ui8Addr, &(pCNimCfg->DemodWBAGCLock));
   if (ST0_WBAGC_LOCKED == pCNimCfg->DemodWBAGCLock)
   {
      dcf872x_get_strength (uUnit);
   }
   /* start the acquisition phase 2 processing */
   st0_acquisition_2 (ui8Addr);
   /* waiting for about 30 ms after the acquisition phase 2 processing */
   task_time_sleep ((u_int32)(DCF872x_ACQP2_DELAY));
   /* start the acquisition phase 3 processing */
   st0_acquisition_3 (ui8Addr);
   #endif
   
   return (DRV_OK);
}

#endif /*#if defined(TOWER_CABLE_TUNER) && (TOWER_CABLE_TUNER==YES)*/

/*****************************************************************************/
/*  FUNCTION:    dcf872x_get_tuning                                          */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to access.                 */
/*               pTuning - pointer to the TUNING_SPEC structure containing   */
/*                         parameters to store.                              */
/*                                                                           */
/*  DESCRIPTION: The function get the parameters of the current tuning.      */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_ERROR if unsuccessful.            */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_get_tuning (u_int32 uUnit, TUNING_SPEC *pTuning)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   
   /* return the current signal type */
   pTuning->type = pCNimCfg->UnitType;
   /* return the current signal parameters, such frequency, symbol rate, ... */
   pTuning->tune.nim_cable_tune.frequency     = pCNimCfg->SignalFREQ;
   pTuning->tune.nim_cable_tune.symbol_rate   = pCNimCfg->SignalSR;
   pTuning->tune.nim_cable_tune.modulation    = pCNimCfg->SignalQAM;
   pTuning->tune.nim_cable_tune.auto_spectrum = pCNimCfg->SignalAutoSP;
   pTuning->tune.nim_cable_tune.spectrum      = pCNimCfg->SignalSI;
   pTuning->tune.nim_cable_tune.annex         = pCNimCfg->SignalAnnex;
   return (DRV_OK);
}

/*****************************************************************************/
/*  FUNCTION:    dcf872x_init                                                */
/*                                                                           */
/*  PARAMETERS:  uUnit  - the unit id of the unit to initialize.             */
/*                                                                           */
/*  DESCRIPTION: The function initializes THOMSON Cable Front-End DCF8722    */
/*                                                                           */
/*  RETURNS:     DRV_OK if successful, DRV_INVALID_CHIPID if unsuccessful.   */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DRV_RETURN dcf872x_init (u_int32 uUnit)
{
   CNIM_CFG   *pCNimCfg = (CNIM_CFG *)(&gCNimCfg);
   u_int8      ui8Addr;
   u_int8      ui8Vers;
   
   gCNimCfg.UnitId   = uUnit;             /* the unit id */
   gCNimCfg.UnitType = DEMOD_NIM_CABLE;   /* the unit type */
   
   /**************************************************************************/
   /*          Configure the tuner module of the Cable Front-end
    */
   dcf872x_tuner_init (pCNimCfg);
   
   /**************************************************************************/
   /*       Configure the demodulator module of the Cable Front-end
    */
   
   /* set i2c slave address to access the demodulator internal registers. */
   ui8Addr              = (u_int8)ST0_I2CADDR;
   pCNimCfg->DemodAddr  = (u_int8)ST0_I2CADDR;
   
   /* get the version information from register through i2c bus and check it */
   #if (defined m88dc2000)
   dc2_get_ver (ui8Addr, &ui8Vers);
   if ( ui8Vers != DC2_VERSION )
   {
      return (DRV_INVALID_CHIPID);
   }
   #else
   st0_get_ver (ui8Addr, &ui8Vers);
   if ( ui8Vers != ST0_VERSION )
   {
      return (DRV_INVALID_CHIPID);
   }
   #endif
   pCNimCfg->DemodChipId = ui8Vers;
   
   /* the demodulator external CLOCK in KHz - 28.8 MHz */
   pCNimCfg->DemodClkExt = 28800;

   /* the sampled IF in KHz after A/D convert, for initial demodulator. */
   pCNimCfg->DemodF1 = pCNimCfg->TunerIF - pCNimCfg->DemodClkExt;
   /* disable the initial quadrature demodulator */
   pCNimCfg->DemodINITDEM = (u_int8)ST0_INITDEM_DISABLE;
   /* configure the working mode of the demodulator internal BERT. */
   dcf872x_set_bert (uUnit);
   /* initialize for C/N estimation */
   #if (defined m88dc2000)
   #else
   st0_init_CN_estimation (ui8Addr);
   #endif
   pCNimCfg->DemodCNEmean   = 3000;
   pCNimCfg->DemodCNEoffset = 0;
   /* initialize the lock status */
   pCNimCfg->LockStatus.PrevSYNC = DEM_NO_LOCKED;
   pCNimCfg->LockStatus.CurrSYNC = DEM_NO_LOCKED;
   pCNimCfg->LockStatus.uNumLOSS = 0;
   
   /* initialize the parameters of signal */
   pCNimCfg->SignalFREQ   = 0;
   pCNimCfg->SignalSR     = 0;
   pCNimCfg->SignalQAM    = MOD_QAM64;       /* EQU_0  - 0x49 */
   pCNimCfg->SignalAutoSP = 1;   /* auto-detect spectrum inversion */
   pCNimCfg->SignalSI     = SPECTRUM_NORMAL; /* CTRL_3 - 0x00 */
   pCNimCfg->SignalAnnex  = ANNEX_A;         /* CTRL_3 - 0x00 */
   
   /* software reset all internal modules of the demodulator. */
   #if (defined m88dc2000)
   dc2_global_soft_reset(ui8Addr);
   dc2_init_regs (ui8Addr);
   #else
   st0_swreset (ui8Addr);
   st0_equ_swreset (ui8Addr);
   st0_di_swreset (ui8Addr);
   st0_rs_swreset (ui8Addr);
   /* initialize all demodulator internal registers with default values */
   st0_init_regs (ui8Addr);
   #endif   
   /**************************************************************************/
   /*       Configure the hardware interrupt for the Cable Front-end
    */
   return (DRV_OK);
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         5/31/04 1:10:46 AM     Steven Shen     CR(s) 
 *        9273 9274 : Adjust the tuner frequency for the signal frequecny 
 *        shift.
 *  4    mpeg      1.3         5/26/04 2:39:51 AM     Steven Shen     CR(s) 
 *        9022 9023 : The DEMOD_DCF8722 driver version 1.20. Fix some bugs.
 *  3    mpeg      1.2         5/20/04 4:27:29 AM     Steven Shen     CR(s) 
 *        9254 9255 : Add the support for the Auto-QAM detection mode.
 *  2    mpeg      1.1         4/4/04 1:01:37 AM      Steven Shen     CR(s) 
 *        8674 8675 : Changed the default auto-detect spectrum inversion option
 *         to 1 (ENABLED).
 *  1    mpeg      1.0         3/15/04 10:30:38 AM    Matt Korte      CR(s) 
 *        8566 : Initial version of Thomson Cable Tuner/Demod
 * $
 *
 ****************************************************************************/
