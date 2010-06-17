 /****************************************************************************/ 
 /*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
 /*                       SOFTWARE FILE/MODULE HEADER                        */
 /*       Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003       */
 /*                              Austin, TX                                  */
 /*                         All Rights Reserved                              */
 /****************************************************************************/
 /*
  * Filename: iic.c
  *
  *
  * Description: API entry point for I2C functions.
  *
  *
  * Functions:                                                                
  ***** Exported Functions ***************************************************
  *    IICInit()                                                              
  *    iicTransaction()                                                       
  *    iicAddressTest()                                                       
  *    iicWriteReg()                                                          
  *    iicWriteIndexedReg()                                                   
  *    iicReadReg()                                                           
  *    iicReadIndexedReg()                                                    
  ***** Internal Functions ***************************************************
  *    SendByte()                                                             
  *    ReadByte()                                                             
  *    i2c_Start()                                                            
  *    i2c_Stop()                                                             
  *    IICIsr()                                                               
  *    IICWatchDogTimer()                                                     
  *    IICDeath()                                                             
  * Author: 
  *
  ****************************************************************************/
 /* $Header: iic.c, 66, 3/16/04 10:22:12 AM, Miles Bintz$
  ****************************************************************************/ 

#include "stbcfg.h"
#include <stdio.h>
#include <stdarg.h>
#include "basetype.h"
#include "globals.h"
#include "retcodes.h"
#include "kal.h"
#include "iic.h"
#include "iicprv.h"

/***********/
/* Aliases */
/***********/
#define IIC_ERROR                          (TRACE_I2C|TRACE_LEVEL_ALWAYS)
#define IIC_INFO                           (TRACE_I2C|TRACE_LEVEL_2)
#define IIC_VERBOSE                        (TRACE_I2C|TRACE_LEVEL_1)

/***************/
/* Global Data */
/***************/
typedef struct _IIC_DATA
{
   PFNISR gpfnIICChain;
   sem_id_t gsemidI2CDone;
   sem_id_t gsemidI2CAvailable;
   #if IIC_TYPE == IIC_TYPE_COLORADO
   HW_DWORD gRegMasterRead;
   #else /* IIC_TYPE != IIC_TYPE_COLORADO */
   HW_DWORD gRegIICStatReg;
   #endif /* IIC_TYPE == IIC_TYPE_COLORADO */
} IIC_SW_DATA, *pIIC_SW_DATA;

static IIC_SW_DATA sIICData[NUM_IIC_BANKS];

bool gbIICInit = FALSE;
static sem_id_t gsemidGpio;
static bool   gbSecondExtenderPresent = FALSE;
static bool   gbExtenderPresent = FALSE;
static u_int8 gbGpioExtenderState = 0xFF;
static u_int8 gbSecondGpioExtenderState = 0xFF;

void dprintf(char*string, ...);

/********************************/
/* Internal Function Prototypes */
/********************************/
static u_int8 iicReadGpioExtender(void);
static u_int8 iicReadSecondGpioExtender(void);
static u_int8 iicWriteGpioExtender(u_int8 bit_mask, u_int8 value);
static u_int8 iicWriteSecondGpioExtender(u_int8 bit_mask, u_int8 value);

/************************/
/* Function definitions */
/************************/

/*****************************************************************************/
/* Function: IICInit()                                                       */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Initializes internal data items.                             */
/*****************************************************************************/
void IICInit(void)
{
   int nRC = 0;
   #if IIC_TYPE == IIC_TYPE_COLORADO
   bool ksPrev;
   #endif /* IIC_TYPE == IIC_TYPE_COLORADO */
   int iBank;
   IICBUS bus = 0;
   char name_str[5]; /* Remember the terminating NULL character!! */
   u_int32 uInterrupt = 0;

   dprintf("->IICInit\n");
   /* Ignore repeated calls once one has succeeded */
   if (gbIICInit)
   {
      dprintf("<-ICInit\n");
      return;
   }

   #if IIC_TYPE == IIC_TYPE_COLORADO
   /* This helped Armond on the emulator.  The slave was interferring with
      the master. */
   *(LPREG)glpI2CCtrlW = 0x8F;
   #endif /* IIC_TYPE == IIC_TYPE_COLORADO */

   /* Allocate driver resources */
   for(iBank=0;iBank<NUM_IIC_BANKS;iBank++)
   {
      /* Create the semaphores */
      sprintf(name_str, "IDS%d", iBank);
      sIICData[iBank].gsemidI2CDone = sem_create(0, name_str);
      if(0 == sIICData[iBank].gsemidI2CDone)
      {
         trace_new(IIC_ERROR, 
         "IICInit: Could not create IIC Done Sem for Bank:%d.\n", iBank);
      }
      sprintf(name_str, "IAS%d", iBank);
      sIICData[iBank].gsemidI2CAvailable = sem_create(1, name_str);
      if(0 == sIICData[iBank].gsemidI2CAvailable)
      {
         trace_new(IIC_ERROR, 
         "IICInit: Could not create IIC Available Sem for Bank:%d.\n", iBank);
      }


      /* Register the IIC interrupt */
      #if NUM_IIC_BANKS == 1
      bus = I2C_BUS_0;
      uInterrupt = INT_I2C;
      nRC = int_register_isr(INT_I2C, (PFNISR)IICIsr, FALSE, FALSE, 
                                             &(sIICData[iBank].gpfnIICChain));
      #else /* NUM_IIC_BANKS != 1 */
      if(0 == iBank)
      {
         bus = I2C_BUS_0;
         uInterrupt = INT_I2C0;
         nRC = int_register_isr(INT_I2C0, (PFNISR)IICIsr, FALSE, FALSE, 
                                             &(sIICData[iBank].gpfnIICChain));
      }
      else if(1 == iBank)
      {
         bus = I2C_BUS_1;
         uInterrupt = INT_I2C1;
         nRC = int_register_isr(INT_I2C1, (PFNISR)IICIsr, FALSE, FALSE, 
                                             &(sIICData[iBank].gpfnIICChain));
      }
      #endif /* NUM_IIC_BANKS == 1 */

      if(RC_OK == nRC)
      {
         #if IIC_TYPE != IIC_TYPE_COLORADO
         /* Set up the Mode Reg */
         iic_hw_init(bus);
         #endif /* IIC_TYPE != IIC_TYPE_COLORADO */

         /* Because startup code might have done IIC transactions in a polled
            mode, if we just enable the interrupt we may get one immediately.
            First, clear the pending interrupt. */
         clear_pic_interrupt( PIC_FROM_INT_ID(uInterrupt), INT_FROM_INT_ID(uInterrupt) );
         nRC = int_enable(uInterrupt);
         if (RC_OK != nRC)
         {
            trace_new(IIC_ERROR, 
               "IICInit: Error Enabling ISR. RC=%x\n", nRC);
            sem_delete(sIICData[iBank].gsemidI2CDone);
            sem_delete(sIICData[iBank].gsemidI2CAvailable);
            return;
         }

         #if IIC_TYPE == IIC_TYPE_COLORADO
         /* Disable IIC interrupts, they will be enabled as needed. */
         ksPrev = critical_section_begin();
         /* Release the data and clock lines. */
         *(LPREG)glpI2CCtrlW = I2C_CTRLW_OVRDDCSDA | I2C_CTRLW_OVRDDCSCL |
                               I2C_CTRLW_OVRI2CSDA | I2C_CTRLW_OVRI2CSCL;
         CNXT_SET_VAL(glpI2CMasterCtrlW, I2C_MASTER_CTLW_INTENABLE, 0);
         CNXT_SET_VAL(glpI2CSlaveCtrlW, I2C_SLAVE_CTLW_INTENABLE, 0);
         critical_section_end(ksPrev);
         #endif /* IIC_TYPE == IIC_TYPE_COLORADO */
      }
   }

   /* Create the GPIO Sem */
   gsemidGpio = sem_create(1, "IIGS");
   if(0 == gsemidGpio)
   {
      trace_new(IIC_ERROR, "IICInit: Could not create IIC GPIO Sem.\n");
      return;
   }

   /* Initialization complete */
   gbIICInit = TRUE;

   /* Check to see if I2C extenders are present and set a flag to */
   /* indicate whether or not they can be used.                   */
   if(gbIICInit)
   {
     #if (I2C_ADDR_NIM_EXT != I2C_BUS_NONE)
     gbExtenderPresent = iicAddressTest(I2C_ADDR_NIM_EXT,
                                        I2C_BUS_NIM_EXT,
                                        FALSE);
     if(gbExtenderPresent)
       iicWriteGpioExtender(0xFF, gbGpioExtenderState);
     #else
     gbExtenderPresent = FALSE;
     #endif
    
     #if (I2C_ADDR_GPIO2_EXT != I2C_BUS_NONE)
     gbSecondExtenderPresent = iicAddressTest(I2C_ADDR_GPIO2_EXT,
                                              I2C_BUS_GPIO2_EXT,
                                              FALSE);
     if(gbSecondExtenderPresent)
       iicWriteSecondGpioExtender(0xFF, gbSecondGpioExtenderState);
     #else
     gbSecondExtenderPresent = FALSE;
     #endif
   }

   dprintf("<-ICInit\n");

   return;
}

/*****************************************************************************/
/* Function: iicTransaction()                                                */
/*                                                                           */
/* Parameters: PIICTRANS pTrans - Pointer I2C Transaction Commands and data. */
/*             IICBUS    bus    - I2C target bus                             */
/*                                                                           */
/* Returns: bool - TRUE on success otherwise, dwError holds an error code.   */
/*                                                                           */
/* Description: Performs an I2C transaction.                                 */
/*****************************************************************************/
#if IIC_TYPE == IIC_TYPE_COLORADO
bool iicTransaction(PIICTRANS pTrans, IICBUS bus)
{
   BYTE* pData;
   BYTE* pCmd;
   DWORD dwCount;
   bool bRead = FALSE;  
   bool bAckRecd;
   int iBank = 0;
   bool bStartStopChecking;

   dprintf("->iicTransaction\n");

   /* Check the IIC bus */
   if(bus != I2C_BUS_0)
   {
      dprintf("I2C ERROR: Only IIC bus supported\n");
      pTrans->dwError = IIC_ERROR_NOADDRESSACK;
      return FALSE;
   }

   if (!pTrans)
   {
      dprintf("I2C ERROR:NULL pTrans\n");
      return FALSE;
   }

   pTrans->dwError = IIC_ERROR_NOERR;

   /* Perform some error checking, just to keep everyone honest. */

   pCmd = pTrans->pCmd;
   if (!pCmd)
   {
      pTrans->dwError = IIC_ERROR_INVALIDCMD;
      dprintf("I2C ERROR:Invalid Command Pointer!\n");
      return FALSE;
   }
   if ( pCmd[0] & IIC_PARTIAL )
   {
      bStartStopChecking = FALSE;
   }
   else
   {
      bStartStopChecking = TRUE;
   }

   pData = pTrans->pData;
   if (!pData)
   {
      pTrans->dwError = IIC_ERROR_INVALIDDATA;
      dprintf("I2C ERROR:Invalid Data Pointer!\n");
      return FALSE;
   }
   
   /* We use an I2C address definition of 0 to imply that a device is not */
   /* installed in a particular customer's IRD. If a call is made to this */
   /* function with an address of 0, therefore, someone has forgotten to  */
   /* remove code that talks to a nonexistant device so dump a warning    */
   /* message in debug builds.                                            */
   #ifdef DEBUG
   if ((*pData == (BYTE)0) && (bStartStopChecking))
   {
     dprintf("I2C ERROR: NULL address passed. Check device supported on target IRD!\n");
     pTrans->dwError = IIC_ERROR_INVALIDADDR;
     error_log(ERROR_WARNING|MOD_I2C);
     return(FALSE);
   }
   #else
   /* In release builds, just reject the transaction */
   if((*pData == (BYTE)0) && (bStartStopChecking))
   {
     pTrans->dwError = IIC_ERROR_INVALIDADDR;
     error_log(ERROR_WARNING|MOD_I2C);
     return(FALSE);
   }
   #endif

   if (gbIICInit)
   {
      /* Make sure that the hardware is not in use */
      sem_get(sIICData[iBank].gsemidI2CAvailable, KAL_WAIT_FOREVER);
   }

   /* Set the master to the correct bus */
   if (CNXT_GET_VAL(glpI2CCtrlW, I2C_CTRLW_I2CMASTER) != bus)
      CNXT_SET_VAL(glpI2CCtrlW, I2C_CTRLW_I2CMASTER, bus);

   for (dwCount=0; dwCount<pTrans->dwCount; pData++, pCmd++, dwCount++)
   {
      if (*pCmd & IIC_START)
      {
         i2c_Start();
         if (!SendByte(*pData, TRUE))
         {
            i2c_Stop();
            if (!(*pCmd & IIC_NO_DEATH))
            {
               trace_new(TRACE_I2C|TRACE_LEVEL_ALWAYS, "IIC 1, no addr ack 0x%02X\n", *pData);
               IICDeath(RC_I2C_NO_ADDRESS_ACK);
            }

            pTrans->dwError = IIC_ERROR_NOADDRESSACK;
/*            trace("IICTrans 1, IIC_ERROR_NOADDRESSACK\n"); */
            if (gbIICInit)
               sem_put(sIICData[iBank].gsemidI2CAvailable);
            return FALSE;
         }
         bRead = (*pData & 1) ? TRUE : FALSE;
      }
      else if (*pCmd & IIC_STOP)
      {
         i2c_Stop();
         dwCount = pTrans->dwCount;
      }
      else
      {
         /* The first command needs to contain the IIC_START flag, if
            this is not the case, a Start command will be isssued by default. */
         if ((dwCount == 0) && (!(*pCmd & IIC_START)) && (bStartStopChecking))
         {
            dprintf("I2C: No start command found, issuing default start\n");
            i2c_Start();
            if (!SendByte(*pData, TRUE))
            {
               i2c_Stop();
               if (!(*pCmd & IIC_NO_DEATH))
               {
                  trace_new(TRACE_I2C|TRACE_LEVEL_ALWAYS, "IIC 2, no addr ack 0x%02X\n", *pData);
                  IICDeath(RC_I2C_NO_ADDRESS_ACK);
                  error_log(ERROR_WARNING|MOD_I2C);
               }

/*               trace("IICTrans 2, IIC_ERROR_NOADDRESSACK\n"); */

               pTrans->dwError = IIC_ERROR_NOADDRESSACK;
               if (gbIICInit)  
               {
                  sem_put(sIICData[iBank].gsemidI2CAvailable);
               }
               error_log(ERROR_WARNING|MOD_I2C);
               return FALSE;
            }
            bRead = (*pData & 1) ? TRUE : FALSE;
         }
         else if (bRead)
         {
            *pData = ReadByte((BYTE)((*pCmd & IIC_ACK) ? 1 : 0));
         }
         else
         {
            bAckRecd = SendByte(*pData, FALSE);
            /* if no ack and checking specified by oring in IIC_ACK, flag error */
            if (!bAckRecd && (*pCmd & IIC_ACK))
            {
               if (!(*pCmd & IIC_NO_DEATH))
               {
                  trace_new(TRACE_I2C|TRACE_LEVEL_ALWAYS, "IIC 3, no data ack 0x%02X\n", *pData);
                  IICDeath(RC_I2C_NO_ACK);
               }
               pTrans->dwError = IIC_ERROR_NOACK;
               if (gbIICInit)  
               {
                  sem_put(sIICData[iBank].gsemidI2CAvailable);
               }
               i2c_Stop();       /* Try to give some chance of the bus not being dead */
               error_log(ERROR_WARNING|MOD_I2C);
               return FALSE;

            } /* endif no ack and ack checking on */
         }
      }

      if ((dwCount == pTrans->dwCount) && (!(*pCmd & IIC_STOP)) && (bStartStopChecking))
      {
         i2c_Stop();
         dprintf("I2C: Issuing default stop condition\n");
      }
   }

   /* Verify that the bus is not hung! */
   if (((!CNXT_GET(glpI2CCtrlR, I2C_CTRLR_I2CSDA)) || 
       (!CNXT_GET(glpI2CCtrlR, I2C_CTRLR_I2CSCL))) && (gbIICInit) && (bStartStopChecking))
   {
      pCmd = pTrans->pCmd;
      pData = pTrans->pData;

      trace_new(IIC_ERROR, "I2C Bus is hung! sda=%d, scl=%d\n", 
         CNXT_GET_VAL(glpI2CCtrlR, I2C_CTRLR_I2CSDA),
            CNXT_GET_VAL(glpI2CCtrlR, I2C_CTRLR_I2CSCL));
      for (dwCount=0; dwCount<pTrans->dwCount; pData++, pCmd++, dwCount++)
      {
         trace_new(IIC_ERROR, "Data = 0x%02X, Cmd = ", *pData);
         if (*pCmd & IIC_START)
            trace_new(IIC_ERROR, "IIC_START ");
         if (*pCmd & IIC_STOP)
            trace_new(IIC_ERROR, "IIC_STOP ");
         trace_new(IIC_ERROR, "\n");
      }
   }

   if (gbIICInit)
      sem_put(sIICData[iBank].gsemidI2CAvailable);

   dprintf("<-iicTransaction\n");
   return TRUE;
}

#else /* IIC_TYPE != IIC_TYPE_COLORADO */

bool iicTransaction(PIICTRANS pTrans, IICBUS bus)
{
   BYTE* pData;
   BYTE* pCmd;
   DWORD dwCount;
   bool bRead = FALSE;
   bool bAckRecd = FALSE;
   int iBank;
   bool bStartStopChecking;

   if(gbIICInit)
   {
      trace_new(IIC_INFO, "iicTransaction. Bus=%d\n", bus);
   }

   /* Check the IIC bus */
   switch(bus)
   {
      case I2C_BUS_0:
         iBank = 0;
         break;

      #if NUM_IIC_BANKS == 2
      case I2C_BUS_1:
         iBank = 1;
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         if(gbIICInit)
         {
            trace_new(IIC_ERROR, "iicTrans: Invalid IIC bus:%d!\n", bus);
         }
         pTrans->dwError = IIC_ERROR_NOADDRESSACK;
         return FALSE;
   }

   if(!pTrans)
   {
      trace_new(IIC_ERROR, "iicTrans: NULL pTrans. Bus=%d\n", bus);
      return FALSE;
   }

   pTrans->dwError = IIC_ERROR_NOERR;

   /* Perform some error checking, just to keep everyone honest. */

   pCmd = pTrans->pCmd;
   if(!pCmd)
   {
      pTrans->dwError = IIC_ERROR_INVALIDCMD;
      trace_new(IIC_ERROR, "iicTrans: Invalid Command Pointer! Bus=%d\n", bus);
      return FALSE;
   }
   if ( pCmd[0] & IIC_PARTIAL )
   {
      bStartStopChecking = FALSE;
   }
   else
   {
      bStartStopChecking = TRUE;
   }

   pData = pTrans->pData;
   if(!pData)
   {
      pTrans->dwError = IIC_ERROR_INVALIDDATA;
      trace_new(IIC_ERROR, "iicTrans: Invalid Data Pointer! Bus=%d\n", bus);
      return FALSE;
   }

   /* We use an I2C address definition of 0 to imply that a device is not */
   /* installed in a particular customer's IRD. If a call is made to this */
   /* function with an address of 0, therefore, someone has forgotten to  */
   /* remove code that talks to a nonexistant device so dump a warning    */
   /* message in debug builds.                                            */
   #ifdef DEBUG
   if((*pData == (BYTE)0) && (bStartStopChecking))
   {
      trace_new(IIC_ERROR, 
"iicTrans: NULL address passed. Check device supported on target IRD! Bus=%d\n", 
            bus);
      pTrans->dwError = IIC_ERROR_INVALIDADDR;
      error_log(ERROR_WARNING|MOD_I2C);
      return(FALSE);
   }
   #else
   /* In release builds, just reject the transaction */
   if((*pData == (BYTE)0) && (bStartStopChecking))
   {
     pTrans->dwError = IIC_ERROR_INVALIDADDR;
     error_log(ERROR_WARNING|MOD_I2C);
     return(FALSE);
   }
   #endif

   if (gbIICInit)
   {
      /* Make sure that the hardware is not in use */
      sem_get(sIICData[iBank].gsemidI2CAvailable, KAL_WAIT_FOREVER);
   }

   /* Start the transaction */
   for(dwCount=0; dwCount<pTrans->dwCount; pData++, pCmd++, dwCount++)
   {
      if(*pCmd & IIC_START)
      {
         if((*(pCmd+1)&IIC_STOP) || (((pTrans->dwCount-dwCount)<=1) && bStartStopChecking))
         {
            bAckRecd = SendByte(*pData, TRUE, TRUE, bus);
            dwCount++;
         }
         else
         {
            bAckRecd = SendByte(*pData, TRUE, FALSE, bus);
         }

         if(!bAckRecd)
         {
            if(!(*pCmd & IIC_NO_DEATH))
            {
               trace_new(IIC_ERROR, 
                  "iicTrans: 1, no addr ack 0x%02X. Bus=%d\n", *pData, bus);
               IICDeath(RC_I2C_NO_ADDRESS_ACK, bus);
            }

            pTrans->dwError = IIC_ERROR_NOADDRESSACK;
            if(gbIICInit)
            {
               sem_put(sIICData[iBank].gsemidI2CAvailable);
            }
            /* Clear the Stat reg value */
            sIICData[iBank].gRegIICStatReg = 0;
            return FALSE;
         }
         bRead = (*pData & 1) ? TRUE : FALSE;
      }
      else if(*pCmd & IIC_STOP)
      {
         /* Should never come here */
         trace_new(IIC_ERROR, 
            "Command is Stop! dwCount=%d. Bus=%d\n", dwCount, bus);
         i2c_SW_Stop(bus);
         dwCount = pTrans->dwCount;
      }
      else
      {
         /* The first command needs to contain the IIC_START flag, if
            this is not the case, a Start command will be isssued by default. */
         if((dwCount == 0) && (!(*pCmd & IIC_START)) && (bStartStopChecking))
         {
            trace_new(IIC_INFO, 
            "iicTrans: No start command found, issuing default start. Bus=%d\n", 
                  bus);

            if((*(pCmd+1)&IIC_STOP) || ((pTrans->dwCount-dwCount)<=1))
            {
               bAckRecd = SendByte(*pData, TRUE, TRUE, bus);
               dwCount++;
            }
            else
            {
               bAckRecd = SendByte(*pData, TRUE, FALSE, bus);
            }

            if(!bAckRecd)
            {
               if(!(*pCmd & IIC_NO_DEATH))
               {
                  trace_new(IIC_ERROR, 
                     "iicTrans: 1, no addr ack 0x%02X. Bus=%d\n", *pData, bus);
                  IICDeath(RC_I2C_NO_ADDRESS_ACK, bus);
               }

               pTrans->dwError = IIC_ERROR_NOADDRESSACK;
               if(gbIICInit)
               {
                  sem_put(sIICData[iBank].gsemidI2CAvailable);
               }
               /* Clear the Stat reg value */
               sIICData[iBank].gRegIICStatReg = 0;
               return FALSE;
            }
            bRead = (*pData & 1) ? TRUE : FALSE;
         }
         else if (bRead)
         {
            if((*(pCmd+1)&IIC_STOP) || (((pTrans->dwCount-dwCount)<=1) && bStartStopChecking))
            {
               *pData = ReadByte(((*pCmd & IIC_ACK) ? TRUE : FALSE), TRUE, bus);
               dwCount++;
            }
            else
            {
               *pData = ReadByte(((*pCmd & IIC_ACK) ? TRUE : FALSE), FALSE, bus);
            }
         }
         else
         {
            if((*(pCmd+1)&IIC_STOP) || (((pTrans->dwCount-dwCount)<=1) && bStartStopChecking))
            {
               bAckRecd = SendByte(*pData, FALSE, TRUE, bus);
               dwCount++;
            }
            else
            {
               bAckRecd = SendByte(*pData, FALSE, FALSE, bus);
            }

            /* if no ack and checking specified by oring in IIC_ACK, flag error */
            if(!bAckRecd && (*pCmd & IIC_ACK))
            {
               if (!(*pCmd & IIC_NO_DEATH))
               {
                  trace_new(IIC_ERROR, 
                     "iicTrans: 3, no data ack 0x%02X. Bus=%d\n", *pData, bus);
                  IICDeath(RC_I2C_NO_ACK, bus);
               }
               pTrans->dwError = IIC_ERROR_NOACK;
               if(gbIICInit)
               {
                  sem_put(sIICData[iBank].gsemidI2CAvailable);
               }
               i2c_SW_Stop(bus);
               /* Clear the Stat reg value */
               sIICData[iBank].gRegIICStatReg = 0;
               error_log(ERROR_WARNING|MOD_I2C);
               return FALSE;
            } /* endif no ack and ack checking on */
         }
      }
   }

   /* Verify that the bus is not hung! */
   if((!CNXT_IIC_STAT_SCL_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank)) || 
       !CNXT_IIC_STAT_SDA_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank))) &&
     (gbIICInit) && bStartStopChecking)
   {
      pCmd = pTrans->pCmd;
      pData = pTrans->pData;

      trace_new(IIC_ERROR, 
      "iicTrans: I2C Bus is hung! Bus=%d(1,2), Int=%d, WAck=%d, sda=%d, scl=%d\n",
         bus,
            CNXT_IIC_STAT_INT_IS_SET(&(sIICData[iBank].gRegIICStatReg)),
               CNXT_IIC_STAT_WRITEACK_IS_SET(&(sIICData[iBank].gRegIICStatReg)),
                  CNXT_IIC_STAT_SCL_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank)),
                     CNXT_IIC_STAT_SDA_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank)));
      for(dwCount=0; dwCount<pTrans->dwCount; pData++, pCmd++, dwCount++)
      {
         if(*pCmd & IIC_START)
         {
            trace_new(IIC_ERROR, 
               "Data[%d] = 0x%02X, Cmd=IIC_START\n", dwCount, *pData);
            continue;
         }
         if(*pCmd & IIC_STOP)
         {
            trace_new(IIC_ERROR, 
               "Data[%d] = 0x%02X, Cmd=IIC_STOP\n", dwCount, *pData);
            continue;
         }
         trace_new(IIC_ERROR, "Data[%d] = 0x%02X\n", dwCount, *pData);
      }
      trace_new(IIC_ERROR,
         "iicTrans: dwCount=%d(%d)\n", dwCount, pTrans->dwCount);
   }

   if(gbIICInit)
   {
      sem_put(sIICData[iBank].gsemidI2CAvailable);
   }

   /* Clear the Stat reg value */
   sIICData[iBank].gRegIICStatReg = 0;

   return TRUE;
}

#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

/*****************************************************************************/
/* Function: iicAddressTest()                                                */
/*                                                                           */
/* Parameters: BYTE   byAddr - 8-bit I2C device address.                     */
/*             IICBUS bus    - I2C target bus                                */
/*             bool   bDeath - Die if the address does not ack.              */
/*                                                                           */
/* Returns: bool - TRUE if address was acknowledged on the bus.              */
/*                                                                           */
/* Description: Checks to see if a device is on the bus.                     */
/*****************************************************************************/
bool iicAddressTest(BYTE byAddr, IICBUS bus, bool bDeath)
{
   IICTRANS iicTransBuf;
   BYTE byData[2];
   BYTE byCmd[2];

   byData[0] = byAddr;    /* Address */

   if (bDeath)
      byCmd[0] = IIC_START;
   else
      byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_STOP;

   iicTransBuf.dwCount = 2;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   return iicTransaction(&iicTransBuf, bus);
}

/*****************************************************************************/
/* Function: iicWriteReg()                                                   */
/*                                                                           */
/* Parameters: BYTE   byAddr - 8-bit I2C device address.                     */
/*             BYTE   byReg  - Register value to write.                      */
/*             IICBUS bus    - I2C target bus                                */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Write a non-indexed I2C device register.                     */
/*****************************************************************************/
bool iicWriteReg(BYTE byAddr, BYTE byReg, IICBUS bus)
{
   IICTRANS iicTransBuf;
   BYTE byData[4];
   BYTE byCmd[4];

   byData[0] = byAddr;    /* Address */
   byData[1] = byReg;

   byCmd[0] = IIC_START;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_STOP;

   iicTransBuf.dwCount = 3;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   return iicTransaction(&iicTransBuf, bus);
}

/*****************************************************************************/
/* Function: iicWriteIndexedReg()                                            */
/*                                                                           */
/* Parameters: BYTE   byAddr - 8-bit I2C device address.                     */
/*             BYTE   byIndex- Index of register.                            */
/*             BYTE   byReg  - Value to write.                               */
/*             IICBUS bus    - I2C target bus                                */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Write a non-indexed I2C device register.                     */
/*****************************************************************************/
bool iicWriteIndexedReg(BYTE byAddr, BYTE byIndex, BYTE byReg, IICBUS bus)
{
   IICTRANS iicTransBuf;
   BYTE byData[4];
   BYTE byCmd[4];

#if (CUSTOMER == VENDOR_D)
   if (byAddr == I2C_ADDR_BT865) {
      if (writeBT865Shadow(byIndex, byReg) == TRUE)
         return (TRUE);
   }
#endif

   byData[0] = byAddr;    // Address
   byData[1] = byIndex;
   byData[2] = byReg;

   byCmd[0] = IIC_START;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   return iicTransaction(&iicTransBuf, bus);
}

/*****************************************************************************/
/* Function: iicReadReg()                                                    */
/*                                                                           */
/* Parameters: BYTE   byAddr - 8-bit I2C device address.                     */
/*             BYTE   *pReg  - Register value read.                          */
/*             IICBUS bus    - I2C target bus                                */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Read a non-indexed I2C device register.                      */
/*****************************************************************************/
bool iicReadReg(BYTE byAddr, BYTE *pReg, IICBUS bus)
{
   IICTRANS iicTransBuf;
   BYTE byData[4];
   BYTE byCmd[4];
   bool bRet;

   byData[0] = byAddr | 0x01;    // Address

   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = 0;
   byCmd[2] = IIC_STOP;

   iicTransBuf.dwCount = 3;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRet = iicTransaction(&iicTransBuf, bus);
   *pReg = byData[1];

   return bRet;
}

/*****************************************************************************/
/* Function: iicReadIndexedReg()                                             */
/*                                                                           */
/* Parameters: BYTE   byAddr - 8-bit I2C device address.                     */
/*             BYTE   byIndex- Index of register.                            */
/*             BYTE   *pReg  - Value read.                                   */
/*             IICBUS bus    - I2C target bus                                */
/*                                                                           */
/* Returns: bool - TRUE on success.                                          */
/*                                                                           */
/* Description: Read a non-indexed I2C device register.                      */
/*****************************************************************************/
bool iicReadIndexedReg(BYTE byAddr, BYTE byIndex, BYTE *pReg, IICBUS bus)
{
   IICTRANS iicTransBuf;
   BYTE byData[8];
   BYTE byCmd[8];
   bool bRet;

#if (CUSTOMER == VENDOR_D)
   if (byAddr == I2C_ADDR_BT865) {
      readBT865Shadow(byIndex, pReg);
      return TRUE;
   }
#endif /* CUSTOMER == VENDOR_D) */

   byData[0] = byAddr;    // Address
   byData[1] = byIndex;
   byData[2] = byAddr | 0x01;

   byCmd[0] = IIC_START;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_START;
   byCmd[3] = 0;
   byCmd[4] = IIC_STOP;

   iicTransBuf.dwCount = 5;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRet = iicTransaction(&iicTransBuf, bus);
   *pReg = byData[3];

   return bRet;
}

/********************************************************************************/
/* The PCF8574 I2C GPIO extender does not allow multiple clients to toggle bits */
/* safely and atomically. As far as I can see, it is not possible to set read   */
/* or write modes on each pin and trying to read the device to retrieve its     */
/* current state results in the pins being switched to read mode. All in all,   */
/* this is not too helpful.                                                     */
/*                                                                              */
/* To get round this, we provide 2 semaphore-protected access functions which   */
/* allow read and write of groups of bits. Read is accomplished using a shadow  */
/* copy of the device state (we know the state at power up/reset and apply      */
/* changes to the shadow as we change the hardware).                            */
/********************************************************************************/
u_int8 read_gpio_extender(void)
{
  u_int8 bState;
  int    iRetcode;
  
  /* Grab the access semaphore */  
  iRetcode = sem_get(gsemidGpio, KAL_WAIT_FOREVER);
  if (iRetcode != RC_OK)
  {
    /* Failed to get the access sem within a ridiculously long timeout. Signal */
    /* an abort since something has obviously gone horribly wrong.             */
    fatal_exit(ERROR_FATAL | MOD_KAL | RC_KAL_TIMEOUT);
  }
  
  bState = iicReadGpioExtender();
  
  sem_put(gsemidGpio);
  
  return(bState);
}

/***********************************************************/
/* Set the states of a subset of pins in the GPIO extender */
/***********************************************************/
u_int8 write_gpio_extender(u_int8 bit_mask, u_int8 value)
{
  u_int8 bState;
  int    iRetcode;
  
  /* Get the access semaphore */
  iRetcode = sem_get(gsemidGpio, KAL_WAIT_FOREVER);
  if (iRetcode != RC_OK)
  {
    /* Failed to get the access sem within a ridiculously long timeout. Signal */
    /* an abort since something has obviously gone horribly wrong.             */
    fatal_exit(ERROR_FATAL | MOD_KAL | RC_KAL_TIMEOUT);
  }
  
  bState = iicWriteGpioExtender(bit_mask, value);
  
  sem_put(gsemidGpio);
  
  return(bState);
}

/***************************************************************/
/* Read what we think the state of the second GPIO extender is */
/***************************************************************/
u_int8 read_second_gpio_extender(void)
{
  u_int8 bState;
  int    iRetcode;
  
  /* Grab the access semaphore */  
  iRetcode = sem_get(gsemidGpio, KAL_WAIT_FOREVER);
  if (iRetcode != RC_OK)
  {
    /* Failed to get the access sem within a ridiculously long timeout. Signal */
    /* an abort since something has obviously gone horribly wrong.             */
    fatal_exit(ERROR_FATAL | MOD_KAL | RC_KAL_TIMEOUT);
  }
  
  bState = iicReadSecondGpioExtender();
  
  sem_put(gsemidGpio);
  
  return(bState);
}

/******************************************************************/
/* Set the states of a subset of pins in the second GPIO extender */
/******************************************************************/
u_int8 write_second_gpio_extender(u_int8 bit_mask, u_int8 value)
{
  u_int8 bState;
  int    iRetcode;
  
  /* Get the access semaphore */
  iRetcode = sem_get(gsemidGpio, KAL_WAIT_FOREVER);
  if (iRetcode != RC_OK)
  {
    /* Failed to get the access sem within a ridiculously long timeout. Signal */
    /* an abort since something has obviously gone horribly wrong.             */
    fatal_exit(ERROR_FATAL | MOD_KAL | RC_KAL_TIMEOUT);
  }
  
  bState = iicWriteSecondGpioExtender(bit_mask, value);
  
  sem_put(gsemidGpio);
  
  return(bState);
}

/*****************************************************************************/
/* Internal Functions                                                        */
/*****************************************************************************/
#if IIC_TYPE == IIC_TYPE_COLORADO
/*****************************************************************************/
/* Function: SendByte()                                                      */
/*                                                                           */
/* Parameters: BYTE byData - Data to send.                                   */
/*             bool bStart - Send a start command                            */
/*                                                                           */
/* Returns: bool - TRUE if acknowledge was received.                         */
/*                                                                           */
/* Description: Writes a byte of data on the I2C bus and returns the ack bit.*/
/*****************************************************************************/
bool SendByte(BYTE byData, bool bStart)
{
   bool bRet = TRUE;
   int iRetcode;
   u_int32 dwCommand = IIC_WRITE_COMMAND;
   int iBank = 0;

   /* Make sure that the master is not busy */
   if ((!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)) && (gbIICInit))
   {
      dprintf("IIC: SendByte waiting for master done.\n");
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: Send Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }

   #ifndef SEPARATE_START
   if (bStart)
      dwCommand |= I2C_MASTER_CTLW_STARTTRANSMIT;
   #endif /* SEPARATE_START */

   if (!gbIICInit)
   {  /* If the our init function has not been called, don't allow interrupts. */
      dwCommand &= ~I2C_MASTER_CTLW_INTENABLE;
   }

   /* Send the byte of data */
   CNXT_SET_VAL(glpI2CMasterData, I2C_DATA_MASK, byData);
   *(LPREG)glpI2CMasterCtrlW = dwCommand;

   if (gbIICInit)
   {
      /* Wait for isr to say that the command has finished */
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: iic Send Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }
   else
   {
      /* Poll the done bit to wait for the transaction to complete. */
      while (!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)); /* Wait for the done bit. */
      sIICData[iBank].gRegMasterRead = *glpI2CMasterCtrlR;
   }
   
/*
   if (glpI2CMasterCtrlR->ReceiveAck != gRegMasterRead.ReceiveAck)
      trace("ACK: Reg = %d, Var = %d\n", glpI2CMasterCtrlR->ReceiveAck, 
         gRegMasterRead.ReceiveAck);
*/

   /* Read the ack bit */
   if (sIICData[iBank].gRegMasterRead & I2C_MASTER_CTLR_RECEIVEACK) 
      bRet = FALSE;

/*
   dprintf("glpI2CMasterCtrlR->ReceiveAck = %d\n", glpI2CMasterCtrlR->ReceiveAck);
   dprintf("gRegMasterRead->ReceiveAck = %d\n", gRegMasterRead.ReceiveAck);
   dprintf("glpI2CMasterCtrlW->TransAck = %d\n", glpI2CMasterCtrlW->TransAck);
*/

   return bRet;
}

#else /* IIC_TYPE != IIC_TYPE_COLORADO */

/*****************************************************************************/
/* Function: SendByte()                                                      */
/*                                                                           */
/* Parameters: BYTE byData - Data to send.                                   */
/*             bool bStart - Send a start command                            */
/*             bool bStop - Send a stop after send?                          */
/*                                                                           */
/* Returns: bool - TRUE if acknowledge was received.                         */
/*                                                                           */
/* Description: Writes a byte of data on the I2C bus and returns the ack bit.*/
/*****************************************************************************/
bool SendByte(BYTE byData, bool bStart, bool bStop, IICBUS bus)
{
   bool bRet = FALSE;
   u_int32 uKalRetcode;
   u_int32 data = 0;
   int iBank;

   /* Check the IIC bus */
   switch(bus)
   {
      case I2C_BUS_0:
         iBank = 0;
         break;

      #if NUM_IIC_BANKS == 2
      case I2C_BUS_1:
         iBank = 1;
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         trace_new(IIC_ERROR, "SendByte: Invalid IIC bus:%d!\n", bus);
         return FALSE;
   }

   if(!gbIICInit)
   {
      /* Set up the Mode Reg. */
      iic_hw_init(bus);
   }

   /* Initialize target */
   data = 0;

   /* Start required ? */
   if(bStart)
   {
      CNXT_IIC_CTRL_START_ENABLE(TRUE, &data);
   }

   /* Set the byte of data */
   CNXT_IIC_CTRL_WRITEDATA_SET(1, &byData, &data);
   CNXT_IIC_CTRL_NUMBYTES_SET(1, &data);

   /* Stop required ? */
   if(bStop)
   {
      CNXT_IIC_CTRL_STOP_ENABLE(TRUE, &data);
   }

   /* Send the Transaction */
   *(LPREG)(CNXT_IIC_CTRL_REG_ADDR(iBank)) = data;

   if(gbIICInit)
   {
      /* Wait for isr to say that the command has finished */
      uKalRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != uKalRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR, "SendByte: Write Timeout on bus:%d.\n", iBank);
         #else
         i2c_SW_Stop(iBank);
         #endif /* DEBUG */
      }
   }
   else
   {
      /* Poll the done bit to wait for the transaction to complete. */
      while(!CNXT_IIC_STAT_INT_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank)));
/*
      {
         task_time_sleep(wait_time);
         if(wait_time>IIC_ERROR_TIMEOUT/1000)
         {
            break;
         }
         else
         {
            wait_time += 10;
         }
      }
*/
      /* Store the register and clear done bit */
      sIICData[iBank].gRegIICStatReg = *(LPREG)(CNXT_IIC_STAT_REG_ADDR(iBank));
      CNXT_IIC_STAT_CLEAR(CNXT_IIC_STAT_REG_ADDR(iBank));
   }

   /* Read the ack bit */
   bRet = CNXT_IIC_STAT_WRITEACK_IS_SET(&(sIICData[iBank].gRegIICStatReg));

   return bRet;
}

#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

#if IIC_TYPE == IIC_TYPE_COLORADO
/*****************************************************************************/
/* Function: ReadByte()                                                      */
/*                                                                           */
/* Parameters: bool bAck - Verify read with ack or not.                      */
/*                                                                           */
/* Returns: BYTE - Data read from I2C bus.                                   */
/*                                                                           */
/* Description: Reads a byte of data from the I2C bus and issues an ack if   */
/*              required.                                                    */
/*****************************************************************************/
BYTE ReadByte(bool bAck)
{
   BYTE byRet = 0;
   int iBank = 0;
   int iRetcode;

   /* Make sure that the master is not busy */
     if ((!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)) && (gbIICInit))
   {
      dprintf("IIC: ReadByte waiting for master done.\n");
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: Read Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }

   if (gbIICInit)
   {
      /* Send the read command */
      *(LPREG)glpI2CMasterCtrlW = IIC_READ_COMMAND |
                                  ((bAck) ? 0 : I2C_MASTER_CTLW_TRANSACK);
 
      /* Wait for isr to say that the command has finished */
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: iic Read Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }
   else /* The KAL has not been initialized, no ISR. */
   {
      *(LPREG)glpI2CMasterCtrlW = 
         (IIC_READ_COMMAND & ~I2C_MASTER_CTLW_INTENABLE) |
         ((bAck) ? 0 : I2C_MASTER_CTLW_TRANSACK);
      
      /* Poll the done bit to wait for the transaction to complete. */
      while (!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)); /* Wait for the done bit.*/
      sIICData[iBank].gRegMasterRead = *glpI2CMasterCtrlR;
   }

   /* Read the data */
   byRet = CNXT_GET_VAL(glpI2CMasterData, I2C_DATA_MASK);

   return byRet;
}

#else /* IIC_TYPE == IIC_TYPE_COLORADO */

/*****************************************************************************/
/* Function: ReadByte()                                                      */
/*                                                                           */
/* Parameters: bool bAck - Verify read with ack or not.                      */
/*             bool bStop - Send a stop after read?                          */
/*                                                                           */
/* Returns: BYTE - Data read from I2C bus.                                   */
/*                                                                           */
/* Description: Reads a byte of data from the I2C bus and issues an ack if   */
/*              required.                                                    */
/*****************************************************************************/
BYTE ReadByte(bool bAck, bool bStop, IICBUS bus)
{
   BYTE byRet = 0;
   u_int32 uKalRetcode;
   u_int32 data = 0;
   int iBank;

   /* Check the IIC bus */
   switch(bus)
   {
      case I2C_BUS_0:
         iBank = 0;
         break;

      #if NUM_IIC_BANKS == 2
      case I2C_BUS_1:
         iBank = 1;
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         trace_new(IIC_ERROR, "ReadByte: Invalid IIC bus:%d!\n", bus);
         return FALSE;
   }

   if(!gbIICInit)
   {
      /* Set up the Mode Reg. */
      iic_hw_init(bus);
   }

   /* Set the number of bytes to read */
   data = 0;
   CNXT_IIC_CTRL_NUMBYTES_SET(1, &data);

   /* Set Read Ack */
   if(bAck)
   {
      CNXT_IIC_CTRL_READACK_ENABLE(TRUE, &data);
   }

   /* Stop required ? */
   if(bStop)
   {
      CNXT_IIC_CTRL_STOP_ENABLE(TRUE, &data);
   }

   /* Send the Transaction */
   *(LPREG)(CNXT_IIC_CTRL_REG_ADDR(iBank)) = data;

   if (gbIICInit)
   {
      /* Wait for isr to say that the command has finished */
      uKalRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != uKalRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR, "ReadByte: Read Timeout on bus:%d.\n", iBank);
         #else
         i2c_SW_Stop(iBank);
         #endif /* DEBUG */
      }
   }
   else
   {
      /* Poll the done bit to wait for the transaction to complete. */
      while(!CNXT_IIC_STAT_INT_IS_SET(CNXT_IIC_STAT_REG_ADDR(iBank)));
/*
      {
         task_time_sleep(wait_time);
         if(wait_time>IIC_ERROR_TIMEOUT/1000)
         {
            break;
         }
         else
         {
            wait_time += 10;
         }
      }
*/
      /* Store the register and clear done bit */
      sIICData[iBank].gRegIICStatReg = *(LPREG)(CNXT_IIC_STAT_REG_ADDR(iBank));
      CNXT_IIC_STAT_CLEAR(CNXT_IIC_STAT_REG_ADDR(iBank));
   }

   /* Read the data */
   CNXT_IIC_READDATA_GET(CNXT_IIC_RDATA_REG_ADDR(iBank), &byRet, 1);

   return byRet;
}

#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

#if IIC_TYPE == IIC_TYPE_COLORADO
/*****************************************************************************/
/* Function: i2c_Start()                                                     */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Issues a start command on the I2C bus.                       */
/*****************************************************************************/
void i2c_Start(void)
{
#ifdef SEPARATE_START
   int iBank = 0;

   if (gbIICInit)
   {
      /* Make sure that the master is not busy */
      if (!glpI2CMasterCtrlR->Done)
      {
         dprintf("IIC: i2c_Start waiting for master done.\n");
         iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
         if(RC_OK != iRetcode)
         {
            #ifdef DEBUG
            trace_new(IIC_ERROR,"ERROR: Start Timeout\n");
            #else /* !DEBUG */
            IICDeath(RC_I2C_BUS_TIMEOUT);
            #endif /* DEBUG */
         }
      }
 
      /* Send the start command */
      *(LPREG)glpI2CMasterCtrlW = IIC_START_COMMAND;
 
      /* Wait for isr to say that the command has finished */
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: iic Start Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }
   else /* The KAL has not been initialized, no ISR. */
   {
      *(LPREG)glpI2CMasterCtrlW = 
         IIC_START_COMMAND & ~I2C_MASTER_CTLW_INTENABLE;
      
      /* Poll the done bit to wait for the transaction to complete. */
      while (!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)); /* Wait for the done bit. */
      gRegMasterRead = *glpI2CMasterCtrlR;
   }
#endif /* SEPARATE_START */
}

/*****************************************************************************/
/* Function: i2c_Stop()                                                      */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Issues a stop command on the I2C bus.                        */
/*****************************************************************************/
void i2c_Stop(void)
{
   int iBank = 0;
   int iRetcode;

   if (gbIICInit)
   {
      /* Make sure that the master is not busy */
      if (!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)) 
      {
         dprintf("IIC: i2c_Stop waiting for master done.\n");
         iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
         if(RC_OK != iRetcode)
         {
            #ifdef DEBUG
            trace_new(IIC_ERROR,"ERROR: Stop Timeout\n");
            #else /* !DEBUG */
            IICDeath(RC_I2C_BUS_TIMEOUT);
            #endif /* DEBUG */
         }
      }
 
      /* Send the stop command */
      *(LPREG)glpI2CMasterCtrlW = IIC_STOP_COMMAND;
 
      /* Wait for isr to say that the command has finished */
      iRetcode = sem_get(sIICData[iBank].gsemidI2CDone, IIC_ERROR_TIMEOUT/1000);
      if(RC_OK != iRetcode)
      {
         #ifdef DEBUG
         trace_new(IIC_ERROR,"ERROR: iic Stop Timeout\n");
         #else /* !DEBUG */
         IICDeath(RC_I2C_BUS_TIMEOUT);
         #endif /* DEBUG */
      }
   }
   else /* The KAL has not been initialized, no ISR. */
   {
      *(LPREG)glpI2CMasterCtrlW = 
         IIC_STOP_COMMAND & ~I2C_MASTER_CTLW_INTENABLE;
      
      /* Poll the done bit to wait for the transaction to complete. */
      while (!CNXT_GET(glpI2CMasterCtrlR, I2C_MASTER_CTLR_DONE)); /* Wait for the done bit. */
      sIICData[iBank].gRegMasterRead = *glpI2CMasterCtrlR;
   }
}
#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

/*****************************************************************************/
/* Function: IICIsr()                                                        */
/*                                                                           */
/* Parameters: u_int32 dwIntID  - Interrupt that the ISR is being asked to   */
/*                                service.                                   */
/*             bool    bFIQ     - If TRUE, the routine is running as a result*/
/*                                of a FIQ, else an IRQ.                     */
/*             PFNISR* pfnChain - A pointer to storage for any ISR to be     */
/*                                called after this function completes.      */
/*                                                                           */
/* Returns: int - RC_ISR_HANDLED - Interrupt fully handled by this routine.  */
/*                                 Do not chain.                             */
/*                RC_ISR_NOTHANDLED - Interrupt not handled by this function.*/
/*                                    KAL should chain to the function whose */
/*                                    pointer is stored in pfnChain          */
/*                                                                           */
/* Description:  Interrupt service routine for the I2C driver.               */
/*****************************************************************************/
int IICIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
   int nRet = RC_ISR_HANDLED;
   int iBank;

/*   isr_trace("->IICIsr\n",0,0); */

   switch(dwIntID)
   {
      #if IIC_TYPE == IIC_TYPE_COLORADO 
      case INT_I2C:
         iBank = 0;
         sIICData[iBank].gRegMasterRead = *glpI2CMasterCtrlR;  /* Copy the Master read reg to
                                                   a local copy.  It is reset when
                                                   the intr is disabled. */
         CNXT_SET_VAL(glpI2CMasterCtrlW, I2C_MASTER_CTLW_INTENABLE, 0); /* Disable the intr. */
      #else /* IIC_TYPE != IIC_TYPE_COLORADO */
      case INT_I2C0:
         iBank = 0;
         /* Handle the Interrupt */
         sIICData[iBank].gRegIICStatReg = *(LPREG)(CNXT_IIC_STAT_REG_ADDR(iBank));
                                                /* Copy the Master read reg to
                                                   a local copy.  It is reset when
                                                   the intr is cleared. */
         CNXT_IIC_STAT_CLEAR(CNXT_IIC_STAT_REG_ADDR(iBank));/* Clear the intr. */
      #endif /* IIC_TYPE == IIC_TYPE_COLORADO  */
         break;

      #if NUM_IIC_BANKS == 2
      case INT_I2C1:
         iBank = 1;
         /* Handle the Interrupt */
         sIICData[iBank].gRegIICStatReg = *(LPREG)(CNXT_IIC_STAT_REG_ADDR(iBank));
                                                /* Copy the Master read reg to
                                                   a local copy.  It is reset when
                                                   the intr is cleared. */
         CNXT_IIC_STAT_CLEAR(CNXT_IIC_STAT_REG_ADDR(iBank));/* Clear the intr. */
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         isr_trace_new(TRACE_I2C | TRACE_LEVEL_ALWAYS, 
            "IICIsr: Unknown interrupt: %d\n", dwIntID, 0);
         return(RC_ISR_NOTHANDLED);
   }

   sem_put(sIICData[iBank].gsemidI2CDone);       /* Signal the process. */

   /*   isr_trace("<-IICIsr\n",0,0); */
   return nRet;
}


/*****************************************************************************/
/* Function: IICDeath()                                                      */
/*                                                                           */
/* Parameters: void                                                          */
/*                                                                           */
/* Returns: void                                                             */
/*                                                                           */
/* Description: Death by I2C.  Something really bad has happend.  This will  */
/*              cause the system to shut down.                               */
/*****************************************************************************/
#if IIC_TYPE == IIC_TYPE_COLORADO
void IICDeath(u_int32 dwDeathCode)
{
   #ifdef DEBUG
   int iBank = 0;
   #endif

   if (gbIICInit)
   {
      #ifdef DEBUG
      isr_trace_new(TRACE_LEVEL_4 | TRACE_I2C, "IICDeath - System shuts down here!\n", 0, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CCtrlW = 0x%08X\n", *(LPREG)glpI2CCtrlW, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CCtrlR = 0x%08X\n", *(LPREG)glpI2CCtrlR, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CMasterCtrlW = 0x%08X\n", *(LPREG)glpI2CMasterCtrlW, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CMasterCtrlR = 0x%08X\n", *(LPREG)glpI2CMasterCtrlR, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CSlaveCtrlW = 0x%08X\n", *(LPREG)glpI2CSlaveCtrlW, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CSlaveCtrlR = 0x%08X\n", *(LPREG)glpI2CSlaveCtrlR, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CMasterData = 0x%08X\n", *(LPREG)glpI2CMasterData, 0);
      isr_trace_new(TRACE_LEVEL_2 | TRACE_I2C, "glpI2CSlaveData = 0x%08X\n", *(LPREG)glpI2CSlaveData, 0);
 
      sem_put(sIICData[iBank].gsemidI2CDone);
      #endif /* DEBUG */

      #if (EMULATION_LEVEL == FINAL_HARDWARE)
      // Send a stop command in an attempt to free the bus.
      *(LPREG)glpI2CMasterCtrlW = IIC_ERROR_STOP_COMMAND;
      // Send a dummy write & stop to free the bus
      SendByte(0, TRUE);
      *(LPREG)glpI2CMasterCtrlW = IIC_ERROR_STOP_COMMAND;
      #endif /* EMULATION_LEVEL == FINAL_HARDWARE */

      #ifndef DEBUG
      isr_error_log(ERROR_FATAL | dwDeathCode);  // Death to all that enter!
      #endif /* DEBUG */
   }
   else // The KAL has not been initialized, no ISR.
   {
      *(LPREG)glpI2CMasterCtrlW = 
         IIC_ERROR_STOP_COMMAND & ~I2C_MASTER_CTLW_INTENABLE;
   }
}

#else /* IIC_TYPE != IIC_TYPE_COLORADO */

void IICDeath(u_int32 dwDeathCode, IICBUS bus)
{
   int iBank;

   switch(bus)
   {
      case I2C_BUS_0:
         iBank = 0;
         break;

      #if NUM_IIC_BANKS == 2
      case I2C_BUS_1:
         iBank = 1;
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         trace_new(IIC_ERROR, "IICDeath: Invalid IIC bus:%d!\n", bus);
         return;
   }

   if(gbIICInit)
   {
      #ifdef DEBUG
      isr_trace_new(IIC_ERROR, "IICDeath - System shuts down here!\n", 0, 0);
      isr_trace_new(IIC_INFO, "IIC Mode Reg = 0x%08X\n", 
                        *(LPREG)(CNXT_IIC_MODE_REG_ADDR(iBank)), 0);
      isr_trace_new(IIC_INFO, "IIC Ctrl Reg = 0x%08X\n", 
                        *(LPREG)(CNXT_IIC_CTRL_REG_ADDR(iBank)), 0);
      isr_trace_new(IIC_INFO, "IIC Stat Reg = 0x%08X\n", 
                        *(LPREG)(CNXT_IIC_STAT_REG_ADDR(iBank)), 0);
      isr_trace_new(IIC_INFO, "IIC ReadData Reg = 0x%08X\n", 
                        *(LPREG)(CNXT_IIC_RDATA_REG_ADDR(iBank)), 0);

      sem_put(sIICData[iBank].gsemidI2CDone);
      #else /* DEBUG */
      isr_error_log(ERROR_FATAL | dwDeathCode);  /* Death to all that enter! */
      #endif /* DEBUG */
   }

   /* Send a stop command in an attempt to free the bus. */
   i2c_SW_Stop(bus);
   /* Send a Dummy Write & stop */
   SendByte(0, FALSE, TRUE, bus);
   SendByte(0, TRUE, TRUE, bus);

   return;
}

void iic_hw_init(IICBUS bus)
{
   static bool hw_init_done[NUM_IIC_BANKS];
   u_int32 data=0;
   int iBank;

   /* Get the correct hw instance */
   switch(bus)
   {
      case I2C_BUS_0:
         iBank = 0;
         break;

      #if NUM_IIC_BANKS == 2
      case I2C_BUS_1:
         iBank = 1;
         break;
      #endif /* NUM_IIC_BANKS == 2 */

      default:
         trace_new(IIC_ERROR, "IICDeath: Invalid IIC bus:%d!\n", bus);
         return;
   }

   /* Init done? */
   if(!hw_init_done[iBank])
   {
      /* Set up the Mode Reg. */
      CNXT_IIC_MODE_SCL_OVERRIDE(FALSE, &data);          /* Allow Hw Control */
      CNXT_IIC_MODE_SDA_OVERRIDE(FALSE, &data);          /* Allow Hw Control */
      CNXT_IIC_MODE_HWCTRL_ENABLE(TRUE, &data);          /* Allow Hw Control */
      CNXT_IIC_MODE_WAITST_ENABLE(TRUE, &data);          /* Allow Slow devices */
      CNXT_IIC_MODE_MULTIMAST_ENABLE(FALSE, &data);     /* We dont have a Second 
                                                            Master on the Bus */
      //CNXT_IIC_MODE_CLKDIV_SET(CNXT_IIC_CLK_DIVIDER_400KHZ, &data);
      CNXT_IIC_MODE_CLKDIV_SET(CNXT_IIC_CLK_DIVIDER_100KHZ, &data);
                                                         /* Allow for older 
                                                            devices */
      *(LPREG)(CNXT_IIC_MODE_REG_ADDR(iBank)) = data;
      /* Clear the Done bit */
      CNXT_IIC_STAT_CLEAR(CNXT_IIC_STAT_REG_ADDR(iBank));

      /* Finished hw init */
      hw_init_done[iBank] = TRUE;
   }

   return;
}

void i2c_SW_Stop(IICBUS bus)
{
   trace_new(IIC_ERROR, "!!!! IIC Stop Called for bus:%d!!!!\n", bus);
}
#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

/**********************************************/
/* Function to convert dprintf into trace_new */
/**********************************************/
void dprintf(char*string, ...)
{
   va_list arg;

   if (gbIICInit)
   {
      va_start(arg,string);
      trace_new(TRACE_I2C | TRACE_LEVEL_1,string, arg);
      va_end(arg);
   }
   return;
}

/********************************************************************/
/* IMPORTANT NOTE:                                                  */
/*                                                                  */
/* We must use shadow copies of these registers rather than reading */
/* the I2C extender part. Reading the part switches all pins to     */
/* input mode automatically and, as a result, does not guarantee to */
/* return the state that was last written to the part. As a specific*/
/* example, pin 0 on extender 1 is wired to the Modem OffHook LED   */
/* on Klondike boards and reading it always returns 0 regardless of */
/* the state of the LED due to the external wiring.                 */
/********************************************************************/

/********************************************************************/
/*  FUNCTION:    iicReadGpioExtender                                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the current state of the I2C GPIO extender  */
/*                                                                  */
/*  RETURNS:     Internal variable shadowing state of part.         */
/*                                                                  */
/********************************************************************/
static u_int8 iicReadGpioExtender(void)
{
  return( gbGpioExtenderState);
}

/********************************************************************/
/*  FUNCTION:    iicWriteGpioExtender                               */
/*                                                                  */
/*  PARAMETERS:  bit_mask - indicating which bits to set/clear      */
/*               value    - indicating the value of those bits      */
/*                                                                  */
/*  DESCRIPTION: Set the state of the I2C GPIO extender             */
/*                                                                  */
/*  RETURNS:     Previous state of the device.                      */
/*                                                                  */
/********************************************************************/
static u_int8 iicWriteGpioExtender(u_int8 bit_mask, u_int8 value)
{
  u_int8 bState;

  /* Get the existing state */
  bState = gbGpioExtenderState;

  if(gbExtenderPresent)
  {
    /* Set states of appropriate GPIO lines */
    gbGpioExtenderState = (gbGpioExtenderState & ~bit_mask) | (value & bit_mask);
  
    /* Write the new state to the device */
    iicWriteReg(I2C_ADDR_NIM_EXT, gbGpioExtenderState, I2C_BUS_NIM_EXT);
  }

  return(bState);
}

/********************************************************************/
/*  FUNCTION:    iicReadSecondGpioExtender                          */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the current state of the I2C GPIO extender  */
/*                                                                  */
/*  RETURNS:     Internal variable shadowing state of part.         */
/*                                                                  */
/********************************************************************/
static u_int8 iicReadSecondGpioExtender(void)
{
  return( gbSecondGpioExtenderState);
}

/********************************************************************/
/*  FUNCTION:    iicWriteSecondGpioExtender                         */
/*                                                                  */
/*  PARAMETERS:  bit_mask - indicating which bits to set/clear      */
/*               value    - indicating the value of those bits      */
/*                                                                  */
/*  DESCRIPTION: Set the state of the I2C GPIO extender             */
/*                                                                  */
/*  RETURNS:     Previous state of the device.                      */
/*                                                                  */
/********************************************************************/
static u_int8 iicWriteSecondGpioExtender(u_int8 bit_mask, u_int8 value)
{
  u_int8 bState;

  /* Get the existing state */
  bState = gbSecondGpioExtenderState;
  
  if (gbSecondExtenderPresent)
  {
    /* Set states of appropriate GPIO lines */
    gbSecondGpioExtenderState = (gbSecondGpioExtenderState & ~bit_mask) | value;
  
    /* Write the new state to the device */
    iicWriteReg(I2C_ADDR_GPIO2_EXT, gbSecondGpioExtenderState, I2C_BUS_GPIO2_EXT);
  
  }

  return(bState);
}

 /****************************************************************************
 * Modifications:
 * $Log: 
 *  66   mpeg      1.65        3/16/04 10:22:12 AM    Miles Bintz     CR(s) 
 *        8567 : fixed warnings
 *  65   mpeg      1.64        9/2/03 7:12:38 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        reordered header files to eliminate warnings when building for PSOS
 *        
 *  64   mpeg      1.63        7/28/03 12:42:36 PM    Joe Kroesche    SCR(s) 
 *        6975 7054 :
 *        removed unneeded header gpioext.h
 *        
 *  63   mpeg      1.62        4/2/03 1:03:00 PM      Billy Jackman   SCR(s) 
 *        5939 5940 :
 *        Correct the sense of the bStartStopChecking flag.
 *        
 *  62   mpeg      1.61        4/2/03 11:09:20 AM     Billy Jackman   SCR(s) 
 *        5939 5940 :
 *        Modified iicTransaction function to not do checking/enforcement of 
 *        start and
 *        stop command requirements in order to enable using iicTransaction to 
 *        do partial
 *        IIC operations.  This is controlled by the appearance of the new 
 *        command flag 
 *        IIC_PARTIAL in the first command byte of the transaction.
 *        
 *  61   mpeg      1.60        3/12/03 10:34:16 AM    Bobby Bradford  SCR(s) 
 *        5744 :
 *        Conditional definition of iBank in IICDeath (Colorado) to remove
 *        build compiler warning
 *        
 *  60   mpeg      1.59        3/10/03 5:34:16 PM     Dave Wilson     SCR(s) 
 *        5732 5731 :
 *        IICInit in previous version would corrupt 1 byte of the stack. A call
 *         to 
 *        sprintf was being made to format a 4 character string into a 4 
 *        character
 *        buffer and the terminating NULL was being written outside the bounds 
 *        of the
 *        array. Buffer size has been increase to 5 to correct this.
 *        
 *  59   mpeg      1.58        2/7/03 4:18:30 PM      Lucy C Allevato SCR(s) 
 *        5394 :
 *        Added IIC_NO_DEATH to first cmd in function iicReadReg, so that the 
 *        IRD won't die if attempting to read to an invalid address.
 *        
 *  58   mpeg      1.57        2/6/03 1:51:04 PM      Billy Jackman   SCR(s) 
 *        5332 :
 *        Make sure to clear the pending interrupts for each iic bank before 
 *        enabling
 *        the interrupt, to avoid a spurious interrupt that will end up putting
 *         an extra
 *        semaphore, corrupting the first operation on that bus.
 *        
 *  57   mpeg      1.56        1/30/03 9:25:34 PM     Billy Jackman   SCR(s) 
 *        5364 :
 *        Add read data register address parameter to invocation of 
 *        CNXT_IIC_READDATA_GET
 *        so it will work for multiple bus setups.
 *        
 *  56   mpeg      1.55        1/29/03 4:08:52 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        Fixed warning. IIC_BUS_NONE is actually I2C_BUS_NONE. Thanks Lucy!
 *        
 *  55   mpeg      1.54        1/29/03 2:51:22 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        Added debug statements and fixed a bug for brazos.
 *        
 *  54   mpeg      1.53        1/22/03 3:53:06 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        removed conditional compilation for RTOS=NOOS. 
 *        
 *  53   mpeg      1.52        1/22/03 3:39:12 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        removed use of hw timers. Tested on Colorado/Watchtv
 *        
 *  52   mpeg      1.51        1/22/03 3:36:48 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        Fixed up iBank in IICIsr
 *        
 *  51   mpeg      1.50        1/22/03 1:50:22 PM     Senthil Veluswamy SCR(s) 
 *        5284 :
 *        Fixed build errors about iBank.
 *        
 *  50   mpeg      1.49        1/22/03 12:44:14 PM    Senthil Veluswamy SCR(s) 
 *        5284 :
 *        Wrapped hwtimer_delete for Colorado. Timers are used only for 
 *        colorado (old code)
 *        
 *  49   mpeg      1.48        1/22/03 12:41:20 PM    Senthil Veluswamy SCR(s) 
 *        5284 :
 *        int_enable INT_I2C1 for brazos (Dave W). Remove use of hwtimers(Dave 
 *        W). removed use of pointer to access config data. 
 *        
 *  48   mpeg      1.47        12/18/02 12:48:46 PM   Senthil Veluswamy SCR(s) 
 *        5190 :
 *        typecast pointers to remove build errors.
 *        
 *  47   mpeg      1.46        12/17/02 4:10:24 PM    Senthil Veluswamy SCR(s) 
 *        5067 :
 *        removed // comments, modifications to use multiple IIC banks (Brazos 
 *        and after)
 *        
 *  46   mpeg      1.45        12/10/02 3:22:00 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Removed INCL_IIC definition from the top of the file. This prevents 
 *        the 
 *        interrupt controller definitions in the chip header from being 
 *        processed and
 *        causes HWLIB.H to fall over since it now uses these values to 
 *        construct the 
 *        KAL interrupt IDs.
 *        
 *  45   mpeg      1.44        12/4/02 1:40:06 PM     Dave Wilson     SCR(s) 
 *        5059 :
 *        Renamed I2C_BUS_x labels to use indices 0 and 1 rather than 1 and 2 
 *        so that
 *        the software definition matches the spec and app note.
 *        
 *  44   mpeg      1.43        11/27/02 9:54:50 AM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Replaced IICBUS type with u_int32 rather than the previous enum. The 
 *        enum
 *        caused warnings under SDT builds when code used the config file 
 *        labels to 
 *        indicate the bus type rather than the old, hardcoded IIC/DDC values.
 *        
 *  43   mpeg      1.42        11/26/02 4:55:56 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed from using hardcoded IIC to I2C_BUS_xxx labels from config 
 *        file
 *        
 *  42   mpeg      1.41        9/10/02 5:53:46 PM     Senthil Veluswamy SCR(s) 
 *        4571 :
 *        Deleted one line too many. Restored the Interrupt clear line in 
 *        IICIsr.
 *        
 *  41   mpeg      1.40        9/10/02 5:17:42 PM     Senthil Veluswamy SCR(s) 
 *        4571 :
 *        Bug fix - Was passing variable value instead of address to macro, in 
 *        check for IIC bus hang. Fixed this. Added debug statements.
 *        
 *  40   mpeg      1.39        9/5/02 5:05:58 PM      Senthil Veluswamy SCR(s) 
 *        4307 :
 *        Added workaround to bring IIC bus out of a Read Hang. 
 *        
 *  39   mpeg      1.38        9/3/02 4:12:32 PM      Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Removed warnings about unused vars, default var types
 *        
 *  38   mpeg      1.37        8/30/02 7:45:34 PM     Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Changes for using the new IIC interface
 *        
 *        
 *  37   mpeg      1.36        7/29/02 5:02:02 PM     Lucy C Allevato SCR(s) 
 *        4298 :
 *        iicTransaction returns if bus is not IIC.
 *        If address to write is 0, it sets pTrans->dwError to 
 *        IIC_ERROR_INVALIDADDR
 *        
 *  36   mpeg      1.35        5/9/02 5:58:28 PM      Craig Dry       SCR(s) 
 *        3733 :
 *        Eradicate IIC bitfield usage, Part 2.  The containing #ifdef
 *        code is removed along with the remnant bitfield code.
 *        
 *  35   mpeg      1.34        5/8/02 3:29:34 PM      Craig Dry       SCR(s) 
 *        3726 :
 *        Compiler header file includes should use angle brackets <>, rather 
 *        than quotes "".
 *        
 *  34   mpeg      1.33        5/7/02 6:00:02 PM      Craig Dry       SCR(s) 
 *        3726 :
 *        Eradicate bitfield use from I2C, step 1
 *        
 *        
 *  33   mpeg      1.32        4/5/02 11:53:28 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  32   mpeg      1.31        3/28/02 2:51:40 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  31   mpeg      1.30        3/21/01 12:40:22 PM    Dave Wilson     DCS1398: 
 *        Merge Vendor D source file changes (keep shadow of 865 registers)
 *        
 *  30   mpeg      1.29        3/19/01 7:02:34 PM     Joe Kroesche    #1452 - 
 *        replaced a dprintf with isr_trace_new where it was being called
 *        in an interrupt context
 *        
 *  29   mpeg      1.28        12/11/00 1:34:56 PM    Dave Wilson     Removed a
 *         couple of error_log calls what were being called in normal
 *        operation.
 *        
 *  28   mpeg      1.27        11/17/00 9:55:16 AM    Dave Wilson     Added 
 *        code to compile in or out the I2C PIO extender accesses
 *        
 *  27   mpeg      1.26        11/17/00 9:34:30 AM    Dave Wilson     Added 
 *        code to catch cases where people try to access devices which are
 *        not installed on a particular target platform.
 *        
 *  26   mpeg      1.25        9/25/00 1:02:36 PM     Dave Wilson     Changed 
 *        NIM extender access functions to use global I2C address definitions
 *        from relevant vendor header files.
 *        
 *  25   mpeg      1.24        8/29/00 8:45:22 PM     Miles Bintz     added 
 *        stdarg to list of includes
 *        
 *  24   mpeg      1.23        8/23/00 1:21:34 AM     Steve Glennon   Added 
 *        checking of acknowledges for data byte writes, if IIC_ACK is
 *        specified in addition to IIC_DATA. Handles IIC_NO_DEATH appropriately
 *        in the error situation.
 *        Part of DCS#489 for Customer B regarding distinguishing data and 
 *        address
 *        acknowledge failures from each other.
 *        
 *  23   mpeg      1.22        5/22/00 6:48:10 PM     Tim White       Fixed bug
 *         in iicWriteGpioExtender (write_gpio_extender) for cases when the
 *        value is being used without the mask applied (i.e. extraneous 1's 
 *        being set).
 *        
 *  22   mpeg      1.21        4/28/00 6:01:40 PM     Dave Wilson     Reverted 
 *        to shadow variable versions of I2C extender access functions
 *        
 *  21   mpeg      1.20        4/27/00 10:59:30 AM    Dave Wilson     It seems 
 *        that the GPIO extenders can be read after all. Fixed up the 
 *        access functions to use values read from the extender rather than
 *        local shadow copies.
 *        
 *  20   mpeg      1.19        4/21/00 2:49:42 PM     Dave Wilson     Replaced 
 *        isr_trace calls with isr_trace_new to allow IIC trace to be
 *        selectively enabled and disabled.
 *        
 *  19   mpeg      1.18        4/14/00 11:24:18 AM    Dave Wilson     Moved 
 *        GPIO extender access functions here from the KAL
 *        
 *  18   mpeg      1.17        3/28/00 6:00:52 PM     Dave Wilson     Added new
 *         APIs to allow access to second GPIO extender on Klondike
 *        
 *  17   mpeg      1.16        3/3/00 1:36:40 PM      Tim Ross        
 *        Eliminated code that was OS-dependent or inefficient for inclusion
 *        in the boot block using the RTOS==NOOS switch.
 *        
 *  16   mpeg      1.15        10/29/99 10:59:18 AM   Dave Wilson     Added 
 *        GPIO extender APIs
 *        
 *  15   mpeg      1.14        10/28/99 1:02:06 PM    Dave Wilson     Renamed 
 *        timer APIs
 *        
 *  14   mpeg      1.13        10/27/99 4:58:18 PM    Dave Wilson     Changed 
 *        WAIT_FOREVER to KAL_WAIT_FOREVER
 *        
 *  13   mpeg      1.12        10/14/99 7:09:40 PM    Dave Wilson     Removed 
 *        dependence on OpenTV headers
 *        
 *  12   mpeg      1.11        8/17/99 4:13:06 PM     Lucy C Allevato Replaced 
 *        critical_start/stop with new critical_section_begin/end.
 *        
 *  11   mpeg      1.10        8/11/99 6:31:34 PM     Dave Wilson     Changed 
 *        KAL calls to use new API.
 *        
 *  10   mpeg      1.9         7/20/99 6:00:14 PM     Rob Tilton      Added the
 *         ability to call the exported IIC API's before the KAL is
 *        initialized and before IICInit has been called.  Any IIC transaction 
 *        that 
 *        happends before IICInit will perform software polling since the ISR 
 *        is not
 *        available.  After IICInit has been called, all transactions will no 
 *        longer
 *        be polled.
 *        
 *  9    mpeg      1.8         5/27/99 9:44:02 AM     Rob Tilton      Added 
 *        more trace info before IICDeath is called.
 *        
 *  8    mpeg      1.7         12/21/98 1:45:58 PM    Rob Tilton      Replaced 
 *        the IICDeath during watchdog timeout with trace message for debug
 *        only builds.  IICDeath was happening too often when the processor was
 *         stopped
 *        from the debugger.
 *        
 *  7    mpeg      1.6         11/19/98 10:47:28 AM   Rob Tilton      Moved the
 *         death timer disable to the ISR.
 *        Check for bus hang after a transaction and display error messages.
 *        
 *  6    mpeg      1.5         11/15/98 5:47:30 PM    Steve Glennon   Made 
 *        repeated calls to IICInit get rejected. This will save on resources 
 *        getting erroneously used up if there are repeated calls.
 *        
 *  5    mpeg      1.4         11/15/98 5:26:20 PM    Steve Glennon   Added 
 *        ic2_Stop to places where  a start failed due to no acknowledge.
 *        Before, we were just leaving the bus active.
 *        
 *  4    mpeg      1.3         10/6/98 6:33:38 PM     Steve Glennon   Fixed up 
 *        some compiler warnings by splitting out if(!(a=b)) into
 *        a=b
 *        if !a
 *        Fixed up a comment block which was missing a closing starslash
 *        Made dprintf into a function which calls trace_new with trace_level_1
 *         (info)
 *        
 *  3    mpeg      1.2         8/10/98 6:15:58 PM     Rob Tilton      Added 
 *        death to iicAddressTest().
 *        
 *  2    mpeg      1.1         8/7/98 3:23:06 PM      Rob Tilton      Added 
 *        API's.
 *        
 *  1    mpeg      1.0         7/30/98 4:13:36 PM     Rob Tilton      
 * $
 * 
 *    Rev 1.64   02 Sep 2003 18:12:38   kroescjl
 * SCR(s) 7415 :
 * reordered header files to eliminate warnings when building for PSOS
 * 
 *    Rev 1.63   28 Jul 2003 11:42:36   kroescjl
 * SCR(s) 6975 7054 :
 * removed unneeded header gpioext.h
 * 
 *    Rev 1.62   02 Apr 2003 13:03:00   jackmaw
 * SCR(s) 5939 5940 :
 * Correct the sense of the bStartStopChecking flag.
 * 
 *    Rev 1.61   02 Apr 2003 11:09:20   jackmaw
 * SCR(s) 5939 5940 :
 * Modified iicTransaction function to not do checking/enforcement of start and
 * stop command requirements in order to enable using iicTransaction to do partial
 * IIC operations.  This is controlled by the appearance of the new command flag 
 * IIC_PARTIAL in the first command byte of the transaction.
 * 
 *    Rev 1.60   12 Mar 2003 10:34:16   bradforw
 * SCR(s) 5744 :
 * Conditional definition of iBank in IICDeath (Colorado) to remove
 * build compiler warning
 * 
 *    Rev 1.59   10 Mar 2003 17:34:16   dawilson
 * SCR(s) 5732 5731 :
 * IICInit in previous version would corrupt 1 byte of the stack. A call to 
 * sprintf was being made to format a 4 character string into a 4 character
 * buffer and the terminating NULL was being written outside the bounds of the
 * array. Buffer size has been increase to 5 to correct this.
 * 
 *    Rev 1.58   07 Feb 2003 16:18:30   allevalc
 * SCR(s) 5394 :
 * Added IIC_NO_DEATH to first cmd in function iicReadReg, so that the IRD won't die if attempting to read to an invalid address.
 * 
 *    Rev 1.57   06 Feb 2003 13:51:04   jackmaw
 * SCR(s) 5332 :
 * Make sure to clear the pending interrupts for each iic bank before enabling
 * the interrupt, to avoid a spurious interrupt that will end up putting an extra
 * semaphore, corrupting the first operation on that bus.
 * 
 *    Rev 1.56   30 Jan 2003 21:25:34   jackmaw
 * SCR(s) 5364 :
 * Add read data register address parameter to invocation of CNXT_IIC_READDATA_GET
 * so it will work for multiple bus setups.
 * 
 *    Rev 1.55   29 Jan 2003 16:08:52   velusws
 * SCR(s) 5284 :
 * Fixed warning. IIC_BUS_NONE is actually I2C_BUS_NONE. Thanks Lucy!
 * 
 *    Rev 1.54   29 Jan 2003 14:51:22   velusws
 * SCR(s) 5284 :
 * Added debug statements and fixed a bug for brazos.
 * 
 *    Rev 1.53   22 Jan 2003 15:53:06   velusws
 * SCR(s) 5284 :
 * removed conditional compilation for RTOS=NOOS. 
 * 
 *    Rev 1.52   22 Jan 2003 15:39:12   velusws
 * SCR(s) 5284 :
 * removed use of hw timers. Tested on Colorado/Watchtv
 * 
 *    Rev 1.51   22 Jan 2003 15:36:48   velusws
 * SCR(s) 5284 :
 * Fixed up iBank in IICIsr
 * 
 *    Rev 1.50   22 Jan 2003 13:50:22   velusws
 * SCR(s) 5284 :
 * Fixed build errors about iBank.
 * 
 *    Rev 1.49   22 Jan 2003 12:44:14   velusws
 * SCR(s) 5284 :
 * Wrapped hwtimer_delete for Colorado. Timers are used only for colorado (old code)
 * 
 *    Rev 1.48   22 Jan 2003 12:41:20   velusws
 * SCR(s) 5284 :
 * int_enable INT_I2C1 for brazos (Dave W). Remove use of hwtimers(Dave W). removed use of pointer to access config data. 
 * 
 *    Rev 1.47   18 Dec 2002 12:48:46   velusws
 * SCR(s) 5190 :
 * typecast pointers to remove build errors.
 * 
 *    Rev 1.46   17 Dec 2002 16:10:24   velusws
 * SCR(s) 5067 :
 * removed // comments, modifications to use multiple IIC banks (Brazos and after)
 * 
 *    Rev 1.45   10 Dec 2002 15:22:00   dawilson
 * SCR(s) 5091 :
 * Removed INCL_IIC definition from the top of the file. This prevents the 
 * interrupt controller definitions in the chip header from being processed and
 * causes HWLIB.H to fall over since it now uses these values to construct the 
 * KAL interrupt IDs.
 * 
 *    Rev 1.44   04 Dec 2002 13:40:06   dawilson
 * SCR(s) 5059 :
 * Renamed I2C_BUS_x labels to use indices 0 and 1 rather than 1 and 2 so that
 * the software definition matches the spec and app note.
 * 
 *    Rev 1.43   27 Nov 2002 09:54:50   dawilson
 * SCR(s) 4902 :
 * Replaced IICBUS type with u_int32 rather than the previous enum. The enum
 * caused warnings under SDT builds when code used the config file labels to 
 * indicate the bus type rather than the old, hardcoded IIC/DDC values.
 * 
 *    Rev 1.42   26 Nov 2002 16:55:56   dawilson
 * SCR(s) 4902 :
 * Changed from using hardcoded IIC to I2C_BUS_xxx labels from config file
 * 
 *    Rev 1.41   10 Sep 2002 16:53:46   velusws
 * SCR(s) 4571 :
 * Deleted one line too many. Restored the Interrupt clear line in IICIsr.
 * 
 *    Rev 1.40   10 Sep 2002 16:17:42   velusws
 * SCR(s) 4571 :
 * Bug fix - Was passing variable value instead of address to macro, in check for IIC bus hang. Fixed this. Added debug statements.
 * 
 *    Rev 1.39   05 Sep 2002 16:05:58   velusws
 * SCR(s) 4307 :
 * Added workaround to bring IIC bus out of a Read Hang. 
 * 
 *    Rev 1.38   03 Sep 2002 15:12:32   velusws
 * SCR(s) 4502 :
 * Removed warnings about unused vars, default var types
 * 
 *    Rev 1.37   30 Aug 2002 18:45:34   velusws
 * SCR(s) 4502 :
 * Changes for using the new IIC interface
 * 
 * 
 *    Rev 1.36   Jul 29 2002 17:02:02   allevalc
 * SCR(s) 4298 :
 * iicTransaction returns if bus is not IIC.
 * If address to write is 0, it sets pTrans->dwError to IIC_ERROR_INVALIDADDR
 * 
 *    Rev 1.35   09 May 2002 16:58:28   dryd
 * SCR(s) 3733 :
 * Eradicate IIC bitfield usage, Part 2.  The containing #ifdef
 * code is removed along with the remnant bitfield code.
 * 
 *    Rev 1.34   08 May 2002 14:29:34   dryd
 * SCR(s) 3726 :
 * Compiler header file includes should use angle brackets <>, rather than quotes "".
 * 
 *    Rev 1.33   07 May 2002 17:00:02   dryd
 * SCR(s) 3726 :
 * Eradicate bitfield use from I2C, step 1
 * 
 *
 ****************************************************************************/ 


