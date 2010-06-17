
#include "GX1001_V35.H"
#include "basetype.H"
#include "bcd.H"
#include "Confmgr.H"
#include "Mw.h"
#include "Huangpu.H"
//#include "demod_module_api.h"

u_int8 GX1001_demod_connect_flag=0;
extern sem_id_t    sem_demod_connect;
extern sem_id_t loader_sem;
extern u_int8 search_flag;
#define  TUNNING_TIME_OUT   700

#define GX_QAM16                (0x03)
#define GX_QAM32                (0x04)
#define GX_QAM64                (0x05)
#define GX_QAM128               (0x06)
#define GX_QAM256               (0x07)

typedef enum
{
   SPEC_NORMAL = 0,
   SPEC_INVERTED,
   NORMAL_INVERTED,
   INVERTED_NORMAL
} NIM_CABLE_SPECTRUM;

extern uint8 scan_flag ;
void GX1001_set_to_cable(sabine_config_data *pConfig)
{
     
      u_int32            iFrequency, iSymbolRate;
      GX_STATE  ret;
      u_int16   QamSize;
      NIM_CABLE_SPECTRUM spectrum_mode;
	
	trace("GX1001 set to cable!\n");	
 
      iFrequency  = bcdint_to_int(pConfig->frequency_hz, 8, 8, 10);
      iSymbolRate = bcdint_to_int(pConfig->symbol_rate, 8, 8, 10);
      iFrequency  = iFrequency * 100;
      iSymbolRate = iSymbolRate * 100;
      
      QamSize    = pConfig->modulation_type;//(MOD_QAMAUTO);      
      spectrum_mode = NORMAL_INVERTED;
      iSymbolRate=iSymbolRate/1000;
      iFrequency=iFrequency/1000;
      switch(QamSize)
      	{
      	      case 16:
      	           QamSize=GX_QAM16;
      	           break;
      	     case 32:
      	           QamSize=GX_QAM32;
      	           break;
      	     case 64:
      	            QamSize=GX_QAM64;
      	            break;
      	     case 128:
      	           	QamSize=GX_QAM128;
      	           	break;
      	     case 256:
      	              QamSize=GX_QAM256;
      	             break;
      	      default:
      	      	      QamSize=GX_QAM64;
      	             break;
      	           				
      	}
      GX1001_demod_connect_flag=0;
      
     ret=GX_Search_Signal(iSymbolRate,
                            0,
                            spectrum_mode,
                            QamSize,
                            iFrequency,
                            TUNNING_TIME_OUT);

     if(SUCCESS==ret)
     	{
             trace("Demod connected.\n");
             GX1001_demod_connect_flag=1;
	      if(sem_demod_connect)
	           sem_put(sem_demod_connect);
		if(search_flag==1)
	      { 
	             mwrt_send_message(MWM_NIM_LOCKED,0,0,0);
		      trace("GX_MWM_NIM_LOCKED\n");
		      task_time_sleep(20);			  
	      }
		else
		{
		     #ifdef GENERAL_LOADER
	             if(loader_sem)
		      {
		   	     sem_put(loader_sem);//eric 1115
	             }
                   #endif
	      }
   	}
     else
     	{
	      trace("Demod timeout, retry. \r\n");
             mwrt_send_message(MWM_NIM_TIMED_OUT,0,0,0);
     	}
     
}
