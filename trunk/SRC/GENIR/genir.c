 /****************************************************************************/ 
 /*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
 /*                       SOFTWARE FILE/MODULE HEADER                        */
 /*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
 /*                              Austin, TX                                  */
 /*                         All Rights Reserved                              */
 /****************************************************************************/
 /*
  * Filename:    GENIR.C
  *
  *
  * Description: Generic Infra Red Remote Control APIs 
  *
  *
  * Author:      Anzhi Chen
  *
  ****************************************************************************/
 /* $Header: genir.c, 17, 2/13/03 11:50:18 AM, Matt Korte$
  ****************************************************************************/ 

#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "genir.h"

IRRX_INFO gFtInfo;
IRRX_INFO gBkInfo;
IRRX_DATAINFO gFtDataInfo;
IRRX_DATAINFO gBkDataInfo;
PFNISR pfnChain1 = (PFNISR)0;
PFNISR pfnChain2 = (PFNISR)0;
bool gbFrontIRPortInited = FALSE;
bool gbBackIRPortInited = FALSE;

void IRRX_Task(void * pinst_info);
int FRONTIR_int_handler(u_int32 uIntID, bool bFIQ, PFNISR *pfnChain);
int BACKIR_int_handler(u_int32 uIntID, bool bFIQ, PFNISR *pfnChain);

IRRX_STATUS cnxt_irrx_init(const IRRX_PORT port, PFNIRINIT pfninit)
{
   if (port == IRRX_TYPE_FRONT)
   {
      pfninit(PORT_FRONT);
      gbFrontIRPortInited = TRUE;
   }
   else if (port == IRRX_TYPE_BACK)
   {
      pfninit(PORT_BACK);
      gbBackIRPortInited = TRUE;
   }
   else
   {
      trace_new(IRD_ERROR_MSG, "IR:Error: Tried to init unknown port.\n");
      return IRRX_ERROR;
   }     
   return IRRX_OK;   
}

IRRX_STATUS cnxt_irrx_swinit(const IRRX_PORTID portid, PFNIRSWINIT pfnswinit, void ** ppinstdecode)
{
   return pfnswinit(portid, ppinstdecode);
   
}

IRRX_STATUS cnxt_irrx_register(IRRX_PORTID portid, PFNDECODE pfndecode, void * pinstdecode,
                              PFNNOTIFY pfnnotify, void * pinstnotify)
{
   if (portid == PORT_UNKNOWN)
   {      
      trace_new(IRD_ERROR_MSG, "IR: port not initialized!\n");
      return IRRX_ERROR;
   }

   if ( (portid == PORT_FRONT) && (gbFrontIRPortInited == FALSE) )
   {      
      trace_new(IRD_ERROR_MSG, "IR: port not initialized!\n");
      return IRRX_ERROR;
   }
   
   if ( (portid == PORT_BACK) && (gbBackIRPortInited == FALSE) )
   {      
      trace_new(IRD_ERROR_MSG, "IR: port not initialized!\n");
      return IRRX_ERROR;
   }
     
   if (portid == PORT_FRONT)
   {  
      gFtInfo.portid = portid;
      gFtInfo.pfndecode = pfndecode;
      gFtInfo.pinst_decode = pinstdecode;
      gFtInfo.pfnnotify = pfnnotify;  
      gFtInfo.pinst_notify = pinstnotify; 
      gFtDataInfo.iread = 0;
      gFtDataInfo.iwrite = 0;
      /*******************************/
      /* Initalize Front IR software */
      /*******************************/
      if (int_register_isr(INT_GIR, (PFNISR)FRONTIR_int_handler, FALSE, FALSE, (PFNISR *)&pfnChain1) != RC_OK)
      {
         trace_new(IRD_ERROR_MSG, "IR:Error: init_register_isr failed\n");
         return IRRX_ERROR;
      }
      if( (gFtDataInfo.sem_id = sem_create(0, "FTIR")) == 0)    
      {
      	trace_new(IRD_ERROR_MSG, "InfraRed: front rxir semaphore create failed\n");
      	return IRRX_ERROR;
      }
      if (task_create(IRRX_Task, (void *)&gFtInfo, (void *)NULL, FTIR_TASK_STACK_SIZE, FTIR_TASK_PRIORITY, FTIR_TASK_NAME) == 0)
      {       
         trace_new(IRD_ERROR_MSG, "IR:front ir task_create failed\n");
         return IRRX_ERROR;
      }
   }
   else
   {
      gBkInfo.portid = portid;
      gBkInfo.pfndecode = pfndecode;
      gBkInfo.pinst_decode = pinstdecode;
      gBkInfo.pfnnotify = pfnnotify;  
      gBkInfo.pinst_notify = pinstnotify; 
      gBkDataInfo.iread = 0;
      gBkDataInfo.iwrite = 0;
      if (int_register_isr(INT_PLSTIMER, (PFNISR)BACKIR_int_handler, FALSE, FALSE, (PFNISR *)&pfnChain2) != RC_OK)   
      {   
	      trace_new(IRD_ERROR_MSG, "IR:Error: init_register_isr failed\n");   
         return IRRX_ERROR;   
	   }   
      if( (gBkDataInfo.sem_id = sem_create(0, "BKIR")) == 0)       
      {   
      	trace_new(IRD_ERROR_MSG, "InfraRed: back rxir semaphore create failed\n");   
      	return IRRX_ERROR;   
      }   
      if (task_create(IRRX_Task, (void *)&gBkInfo, (void *)NULL, BKIR_TASK_STACK_SIZE, BKIR_TASK_PRIORITY, BKIR_TASK_NAME) == 0)   
      {          
         trace_new(IRD_ERROR_MSG, "IR:back ir task_create failed\n");   
         return IRRX_ERROR;   
      }   
   }
   
   return IRRX_OK;
}

void IRRX_Task(void * pinst_info)
{
   PIRRX_INFO pInstInfo;
   PFNDECODE fndecode;
   PFNNOTIFY fnnotify;
   void * pinst_decode;
   void * pinst_notify;
   LPREG            lpEnable;
   LPREG         lpPTfcr;
   IRRX_KEYINFO keyinfo;
  
   pInstInfo = (PIRRX_INFO)pinst_info;
   fndecode = pInstInfo -> pfndecode;
   fnnotify = pInstInfo -> pfnnotify;
   pinst_decode = pInstInfo -> pinst_decode;   
   pinst_notify = pInstInfo -> pinst_notify;   
   
   lpEnable = (LPREG)IRD_INT_ENABLE_REG;
   lpPTfcr = (LPREG)PLS_CONTROL_REG;
   
   if (pInstInfo -> portid == PORT_FRONT)
   {
   	if (int_enable(INT_GIR) != RC_OK)
	   {        
	      trace_new(IRD_ERROR_MSG, "IR int enable failed\n");
   	   sem_delete(gFtDataInfo.sem_id);
	      task_terminate();
   	}
	}
   else
   {
	   if (int_enable(INT_PLSTIMER) != RC_OK)
   	  {        
	       trace_new(IRD_ERROR_MSG, "Pulse Timer int enable failed\n");
	       sem_delete(gBkDataInfo.sem_id);
   	    task_terminate();
	   }
   }
   
   while (1)
   {
      keyinfo.device_type = IRRX_UNKNOWN;  
      if (pInstInfo -> portid == PORT_FRONT)
      {
         sem_get(gFtDataInfo.sem_id, KAL_WAIT_FOREVER);
         fndecode(pinst_decode, &gFtDataInfo, &keyinfo);
         if (keyinfo.device_type != IRRX_UNKNOWN)
         {  
            keyinfo.device_type |= IRRX_FRONT;
            fnnotify(pinst_notify, &keyinfo);
            /* reset keyinfo structure */
            keyinfo.device_type = IRRX_UNKNOWN;
         }
         /* Re-enable interrupt in case it was disable in ISR */
         if (CNXT_GET_VAL(lpEnable, IRD_INT_ENABLE_RXFIFO_MASK) == 0)
            CNXT_SET_VAL(lpEnable, IRD_INT_ENABLE_RXFIFO_MASK, 1);
      }
      else
      {
         sem_get(gBkDataInfo.sem_id, KAL_WAIT_FOREVER);
         fndecode(pinst_decode, &gBkDataInfo, &keyinfo);
         if (keyinfo.device_type != IRRX_UNKNOWN)
         { 
            keyinfo.device_type |= IRRX_BACK;
            fnnotify(pinst_notify, &keyinfo);
            keyinfo.device_type = IRRX_UNKNOWN;
         }
         if (CNXT_GET_VAL(lpPTfcr, PLS_CONTROL_FIFO_INT_ENABLE_MASK) == 0)
         {
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_ENABLE_MASK, 0); //Disable PT before changing any configuration
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_TIMEOUT_INT_ENABLE_MASK, 0);
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_OVERRUN_INT_ENABLE_MASK, 0);
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_FIFO_INT_ENABLE_MASK, 1);
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_EDGE_CNTL_MASK, PLS_EDGE_EITHER);
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_FIFO_INT_CNTL_MASK, PLS_FIFO_INT_NOT_EMPTY);
	    CNXT_SET_VAL(lpPTfcr, PLS_CONTROL_ENABLE_MASK, 1);
         }     
      }
   }
}

/*******************************************************************************
*   FRONTIR_int_handler: interrupt handler for front panel IR
*
*   This interrupt handler reads datum from receive IR FIFO til FIFO is empty 
*
*******************************************************************************/
int FRONTIR_int_handler(u_int32 uIntID, bool bFIQ, PFNISR *pfnChain)
{
   int count_vacancy;
   int i;
   bool bFifoEmpty;
   
   LPREG        lpIRControl;
   LPREG        lpIRStatus;
   LPREG        lpIRDatar;
   LPREG        lpEnable; 

   lpEnable    = (LPREG)IRD_INT_ENABLE_REG;
   lpIRControl = (LPREG)IRD_CONTROL_REG;
   lpIRStatus  = (LPREG)IRD_STATUS_REG;
   lpIRDatar = (LPREG)IRD_DATA_BASE;
   bFifoEmpty = FALSE;
   
   if(gFtDataInfo.iwrite >= gFtDataInfo.iread)
   	count_vacancy = IR_BUFFER_SIZE  - (gFtDataInfo.iwrite - gFtDataInfo.iread);
   else
	   count_vacancy = gFtDataInfo.iread - gFtDataInfo.iwrite;

   if((CNXT_GET_VAL(lpIRStatus, IRD_STATUS_RXOVERRUN_MASK)) == TRUE)
   {
   	CNXT_SET_VAL(lpIRControl, IRD_CONTROL_ENBL_RXFIFO_MASK, 0);  //flush Rx Fifo if overrun
   	CNXT_SET_VAL(lpIRControl, IRD_CONTROL_ENBL_RXFIFO_MASK, 1);  //reenable the Rx. Fifo
   	isr_trace_new(IRD_ERROR_MSG, "InfraRed receive fifo overflowed.\n",0,0);
   }       
   else
   {
	   if(count_vacancy > 0)
   	{
      	for(i=0; i<count_vacancy; i++)
		   {
            gFtDataInfo.data[gFtDataInfo.iwrite] = ((*lpIRDatar) & (IRD_DATA_MASK|IRD_BIT_LEVEL|IRD_DATA_AVAIL)) ^ IR_LEVEL_MASK;  
		      /* check to see if this is the end of transmission */
            #ifdef COLORADO_IR_INVERT_BUG
            /* The colorado chip has a bug relating to ir input
             * see the Colorado Errata document for details
             */
   		   if((gFtDataInfo.data[gFtDataInfo.iwrite] & 0x0ffff) == 0x0ffff)
            {
               //Logic level was NOTed.  For end of transmission, reset logic value to 0. Bug in Colorado.
               gFtDataInfo.data[gFtDataInfo.iwrite] = 0x0ffff; 
            }
            #endif
		      gFtDataInfo.iwrite++;
   		   gFtDataInfo.iwrite &= IR_INDEX_MASK;
	   	   // Check status register and the control register to see if FIFO empty 
		   if( ((CNXT_GET_VAL(lpIRStatus, IRD_STATUS_RXFIFO_REQ_MASK)) == 0) && 
		       ((CNXT_GET_VAL(lpIRControl, IRD_CONTROL_RXINT_CTRL_MASK)) == 1) )
		     {
		       bFifoEmpty = TRUE;
		       break;
		     }    
		   }
   	   if(!bFifoEmpty)
		   {
		      /* Disable receive interrup if no more space in ring buffer */
   		   /* but receive FIFO is still not empty.  This prevents FIFO */
	   	   /* overrun later.                                           */
		     if( ((CNXT_GET_VAL(lpIRStatus, IRD_STATUS_RXFIFO_REQ_MASK)) == 1) && //if FIFO is still NOT empty
			 ((CNXT_GET_VAL(lpIRControl, IRD_CONTROL_RXINT_CTRL_MASK)) == 1) )
		       {
			 CNXT_SET_VAL(lpEnable, IRD_INT_ENABLE_RXFIFO_MASK, 0);
			 isr_trace_new(IRD_ISR_MSG, "InfraRed receive interrupt disabled.\n", 0, 0);
		       }
		   }
         //Signal semaphore whenever there is data in buffer.
	      //sem_get(gFtDataInfo.sem_id, 0);  //signal semaphore if it hasn't beent done so
	      sem_put(gFtDataInfo.sem_id);
	   }       
   }

   return(RC_ISR_HANDLED);
}

/******************************************************************************
*   BACKIR_int_handler: interrupt handler for back panel IR
*
*   This interrupt handler reads datum from receive IR FIFO til FIFO is empty 
*
*******************************************************************************/
int BACKIR_int_handler(u_int32 uIntID, bool bFIQ, PFNISR *pfnChain)
{
   int count_vacancy;
   int i;
   bool bFifoEmpty;
   u_int32 buf_overrun[8];

   LPREG         lpPTControl;
   LPREG         lpPTStatus;
   LPREG lpPTDatar;

   lpPTControl = (LPREG)PLS_CONTROL_REG;
   lpPTStatus  = (LPREG)PLS_STATUS_REG;
   lpPTDatar = (LPREG)PLS_FIFO_DATA_REG;
   bFifoEmpty = FALSE;

   if(gBkDataInfo.iwrite >= gBkDataInfo.iread)
   	count_vacancy = IR_BUFFER_SIZE  - (gBkDataInfo.iwrite - gBkDataInfo.iread);
   else
	   count_vacancy = gBkDataInfo.iread - gBkDataInfo.iwrite;

   if((CNXT_GET_VAL(lpPTStatus, PLS_STATUS_FIFO_OVERRUN_MASK)) == 1)
   {
	   isr_trace_new(IRD_ERROR_MSG, "Encountered IRPT receive fifo overrun.\n",0,0);
      //Read from FIFO to flush it. 
      for (i = 0; i< 8; i++)
         buf_overrun[i++] = * lpPTDatar;
   }       
   else
   {
	   if(count_vacancy > 0)
   	{
	   	for(i=0; i<count_vacancy; i++)
		   {
		      gBkDataInfo.data[gBkDataInfo.iwrite] = ((*lpPTDatar) & (PLS_DATA_MASK|PLS_BIT_LEVEL|PLS_DATA_AVAIL)) ^ IR_LEVEL_MASK;  
   		   if((gBkDataInfo.data[gBkDataInfo.iwrite] & 0x0ffff) == 0x0ffff)
            {
               //Logic level was NOTed.  For end of transmission, reset logic value to 0. Bug in Colorado.
               gBkDataInfo.data[gBkDataInfo.iwrite] = 0x0ffff; 
   		   }
            gBkDataInfo.iwrite++;
	   	   gBkDataInfo.iwrite &= IR_INDEX_MASK;
		      // Check status register and the control register to see if FIFO empty 
   		   if( ((CNXT_GET_VAL(lpPTStatus, PLS_STATUS_FIFO_SERVICE_MASK)) == 0) && 
		       ((CNXT_GET_VAL(lpPTControl, PLS_CONTROL_FIFO_INT_CNTL_MASK)) == 1) )
	   	   {       
		         bFifoEmpty = TRUE;
		         break;
   		   }    
	   	}
         if(!bFifoEmpty)
         {
            /* Disable FIFO interrupt to prevent overrun */
            CNXT_SET_VAL(lpPTControl, PLS_CONTROL_ENABLE_MASK, 0);    //Disable PT first
            CNXT_SET_VAL(lpPTControl, PLS_CONTROL_FIFO_INT_ENABLE_MASK, 0);
 	      	isr_trace_new(IRD_ERROR_MSG, "Pulse Timer Fifo interrupt disabled.\n", 0, 0);
         }
         /* Signal semaphore whenever there is data in buffer. */
    	   sem_get(gBkDataInfo.sem_id, 0);  
 	      sem_put(gBkDataInfo.sem_id);
      }
	}     

   return(RC_ISR_HANDLED);
}

 /****************************************************************************
 * Modifications:
 * $Log: 
 *  17   mpeg      1.16        2/13/03 11:50:18 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  16   mpeg      1.15        10/15/02 4:56:12 PM    Steve Glennon   SCR(s): 
 *        4796 
 *        Added cnxt_irrx_swinit to support registration of a software 
 *        initialization
 *        function in the IR decoder. This removes the need for the GENIR user 
 *        from having
 *        to know the format of the protocol specific instance structure, have 
 *        a copy
 *        of it and initialize it correctly. That function truly belongs in the
 *         decode
 *        module itself. 
 *        cnxt_irrx_swinit should be called after cnxt_irrx_swinit and before
 *        cnxt_irrx_register.
 *        
 *        
 *  15   mpeg      1.14        5/17/02 12:14:54 PM    Craig Dry       SCR(s) 
 *        3810 :
 *        Eradicate bitfields from IRD code
 *        
 *  14   mpeg      1.13        5/14/02 12:03:24 PM    Craig Dry       SCR(s) 
 *        3779 :
 *        Eradicate bitfield use from PLS(Pulse Timer), step 2
 *        
 *  13   mpeg      1.12        5/14/02 11:43:52 AM    Craig Dry       SCR(s) 
 *        3776 :
 *        Eradicate bitfield use from PLS(Pulse Timer), step 1
 *        
 *  12   mpeg      1.11        3/11/02 11:03:10 AM    Ian Mitchell    SCR(s): 
 *        3342 
 *        used new definitions for BKIR_TASK settings when the task is created
 *        
 *  11   mpeg      1.10        11/29/01 6:06:58 PM    Quillian Rutherford 
 *        SCR(s) 2933 :
 *        Added a check for a bug in Colorado with the IR input.  
 *        See the hardware Errata for details on the bug.  
 *        
 *        
 *  10   mpeg      1.9         5/15/01 4:19:58 PM     Anzhi Chen      Changed 
 *        INT_GIR to INT_PLSTIMER when calling int_register_isr for back IR.
 *        This is the same change as for 1.8.1.0 for HONDO build.
 *        
 *  9    mpeg      1.8         4/12/01 5:13:02 PM     Anzhi Chen      Renamed 
 *        IRRX_BAD as IRRX_UNKNOWN.
 *        
 *  8    mpeg      1.7         2/21/01 4:54:34 PM     Anzhi Chen      Removed 
 *        an unused variable.
 *        
 *  7    mpeg      1.6         2/15/01 1:30:30 PM     Anzhi Chen      Removed 
 *        the sem_get call in the ISR since VxWorks does not allow sem_get from
 *        ISR.
 *        
 *  6    mpeg      1.5         2/13/01 3:47:28 PM     Anzhi Chen      Added two
 *         globals to indicate if IR port are initialized.
 *        
 *  5    mpeg      1.4         2/13/01 3:24:12 PM     Anzhi Chen      Changed 
 *        prototype of cnxt_irrx_init() so it accepts a second parameter which
 *        points to IR protocol specific initialization function.  This init 
 *        function
 *        is located in IR protocol specific driver directory (like IRSEJ).
 *        
 *  4    mpeg      1.3         2/7/01 6:37:36 PM      Anzhi Chen      Fixed a 
 *        bunch of bugs.
 *        
 *  3    mpeg      1.2         2/7/01 2:37:14 PM      Anzhi Chen      Fixed a 
 *        bug that points lpEnable to unassigned memory address.
 *        
 *  2    mpeg      1.1         2/6/01 6:48:42 PM      Anzhi Chen      Fixed 
 *        compiling errors.
 *        
 *  1    mpeg      1.0         2/6/01 3:51:00 PM      Anzhi Chen      
 * $
 * 
 *    Rev 1.16   13 Feb 2003 11:50:18   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.15   15 Oct 2002 15:56:12   glennon
 * SCR(s): 4796 
 * Added cnxt_irrx_swinit to support registration of a software initialization
 * function in the IR decoder. This removes the need for the GENIR user from having
 * to know the format of the protocol specific instance structure, have a copy
 * of it and initialize it correctly. That function truly belongs in the decode
 * module itself. 
 * cnxt_irrx_swinit should be called after cnxt_irrx_swinit and before
 * cnxt_irrx_register.
 * 
 * 
 *    Rev 1.14   17 May 2002 11:14:54   dryd
 * SCR(s) 3810 :
 * Eradicate bitfields from IRD code
 * 
 *    Rev 1.13   14 May 2002 11:03:24   dryd
 * SCR(s) 3779 :
 * Eradicate bitfield use from PLS(Pulse Timer), step 2
 * 
 *    Rev 1.12   14 May 2002 10:43:52   dryd
 * SCR(s) 3776 :
 * Eradicate bitfield use from PLS(Pulse Timer), step 1
 *
 ****************************************************************************/ 

