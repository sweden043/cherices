/*****************************************************************************/
/* File: gfxisr.c                                                            */
/* Module: gfxlib - CX2249X GFX Library                                      */
/* Description: Interrupt service routine for the OSD driver.                */
/*****************************************************************************/
/* Functions:                                                                */
/*    GXAIsrInit()                                                           */
/*    GXAIsr()                                                               */
/*****************************************************************************/
/***************************************************************************
$Header: gxaisr.c, 2, 2/13/03 11:57:26 AM, Matt Korte$
****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#define INCL_GXA
#define INCL_MPG
#include "kal.h"
#include "globals.h"
#include "retcodes.h"
#include "trace.h"
PFNISR gpfnGxaChain = NULL;

sem_id_t qmark_sem;

int GXAIsr(u_int32, bool, PFNISR *); 

/* Function: GXAIsrInit
 * Parameters: void
 * Returns: void
 * Description: Intializes the GXA interrupt service routine. This
 *              is called from the GFXinit function.
 */

void GXAIsrInit(void) {
   int nRC;
   bool ksPrev;


   // Create a semaphore for the GXA interrupt signal
   qmark_sem = sem_create(0,"GXA2");

   // JQR Register the GXA interrupt so you can see when the operation completes
   nRC = int_register_isr(INT_GXA, (PFNISR)GXAIsr, FALSE, FALSE,
       &gpfnGxaChain);
   if (RC_OK == nRC)
   {
      nRC = int_enable(INT_GXA);
      if (RC_OK != nRC)
      {
         trace_new(TRACE_GCP, "int_enable(INT_GXA) returned ");
         switch (nRC)
         {
         case RC_KAL_INVALID:
            trace_new(TRACE_GCP, "RC_KAL_INVALID\n");
            break;
         case RC_KAL_NOTHOOKED:
            trace_new(TRACE_GCP, "RC_KAL_NOTHOOKED\n");
            break;
         }
      }
      else
      {
         // Enable GXA qmark and hsx bus error interrupts
         ksPrev = critical_section_begin();
         *glpIntMask |= (GXA_STAT_QMARK | GXA_STAT_HSXERR); 
         critical_section_end(ksPrev);
      }
   }
   else
   {
      trace_new(TRACE_GCP, "int_register_isr(INT_GXA, ...) returned ");
      switch (nRC)
      {
      case RC_KAL_INVALID:
         trace_new(TRACE_GCP, "RC_KAL_INVALID\n");
         break;
      case RC_KAL_BADPTR:
         trace_new(TRACE_GCP, "RC_KAL_BADPTR\n");
         break;
      case RC_KAL_PRIVATE:
         trace_new(TRACE_GCP, "RC_KAL_PRIVATE\n");
         break;
      }
   }
}

/*
 * Function GXAISr()
 * Parameters:   u_int32 dwIntID    - Interrupt that the ISR is being
 *                                    asked to service.
 *               bool    bFIQ       - If TRUE, the routine is running as a 
 *                                    result of a FIQ, else an IRQ.
 *               PFNISR* pfnChain   - A pointer to storage for any ISR to
 *                                    be called after this function completes.
 * Returns:  int - RC_ISR_HANDLED - Interrupt fully handled by this routine.
 *                                  Do not chain.
 *               - RC_ISR_NOTHANDLED - Interrupt not handled by this function
 *                                     KAL should chain to the function whose
 *                                     pointer is stored in pfnChain
 * Description:  Interrupt service routine for the GXA driver
 */

int GXAIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain) {
   int nRet = RC_ISR_HANDLED;
   u_int32 nStatus;
   volatile u_int32 *gxaCfg2Reg = (u_int32 *) GXA_CFG2_REG;

   isr_trace_new(TRACE_GCP,"->GXAIsr\n",0,0);

   switch(dwIntID){
      case INT_GXA: // JQR handle GXA interrupts
          nStatus = *gxaCfg2Reg;
          if (nStatus & GXA_STAT_HSXERR){
             //Not sure what we need to do to handle this...
             isr_trace_new(TRACE_GCP,"GXA: Bus ERROR!\n",0,0);
             //Write this bit to reset it
             *gxaCfg2Reg &= *gxaCfg2Reg & GXA_STAT_HSXERR;
          }
          else if(nStatus & GXA_STAT_QMARK){
             sem_put(qmark_sem);
             isr_trace_new(TRACE_GCP,"GXA:Put qmark_sem.\n",0,0);
             //Write this bit to reset it.
             *gxaCfg2Reg &= *gxaCfg2Reg & GXA_STAT_QMARK;
          } 
          else if((*(LPREG)glpIntStatus) && ((*pfnChain =gpfnGxaChain)!=0))
              nRet = RC_ISR_NOTHANDLED;
          break;
      default:
          nRet = RC_ISR_NOTHANDLED;
    }

    return nRet;
}
/***************************************************************************
                     PVCS Version Control Information
$Log: 
 2    mpeg      1.1         2/13/03 11:57:26 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 1    mpeg      1.0         8/24/01 3:56:26 PM     Quillian Rutherford 
$
 * 
 *    Rev 1.1   13 Feb 2003 11:57:26   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
****************************************************************************/
