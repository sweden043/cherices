//请参考《GX1001P软件包说明-V3.5.doc》
//Please refer to <GX1001 Software Developer Kit User's Manual_V3.5>
/*
Abbreviation
    GX		--	GUOXIN 
    IF		--	intermediate frequency
    RF		--  radiate frequency
    SNR		--	signal to noise ratio
    OSC		--	oscillate
    SPEC	--	spectrum
    FREQ	--	frequency
*/

#include "stbcfg.h"
#include "retcodes.h"
#include "GX1001_V35.h"
#include "basetype.h"
#include "iic.h"
#include "kal.H"

int  GX_CHIP_ADDRESS	= 0x18;    /*GX1001 chip address*/

int  GX_OSCILLATE_FREQ	= 28800;	//(oscillate frequency) ( Unit: KHz ) 
int  GX_IF_FREQUENCY	= 36000;	//(tuner carrier center frequency) ( Unit: KHz ) 

int  GX_TS_OUTPUT_MODE  = 1;        // 1: Parallel output,  0: Serial output

/*========================================================================================================================*/
/*========================================================================================================================*/
/*========================= User-defined GX1001 software interface begin =================================================*/
/*========================================================================================================================*/

/*
Function:	Delay N ms
Input:		
        ms_value - delay time (ms)
*/
void GX_Delay_N_ms(int ms_value)
{
	//User must add the delay function
	task_time_sleep(ms_value);
}

#ifndef I2C_BUS_CABLE_GX1001
#define I2C_BUS_CABLE_GX1001      (I2C_BUS_0)
#endif

/*
Function:	IIC BUS Write or Read function
Input:	
        WR_flag		-- 1: Write, 0: Read
		ChipAddress	-- Chip Address
        RegAddress  -- Write or Read register address
		*data		-- the pointer of write or read data
		data_number -- the data number to be readed or written //always be 1 eric
Output:	
        SUCCESS : return 1  , *data = Read_value ( only available when read operation )
        FAILURE : return -1
*/
GX_STATE GX_I2cReadWrite(int WR_flag, unsigned char ChipAddress,unsigned char RegAddress,unsigned char *data, int data_number)
{

       IICTRANS    iicTransBuf;
       u_int8      ui8Data[20];
       u_int8      ui8Cmd[20];
       u_int8 i;
          
     /* setup data and commands for i2c transaction */
       
  if(WR_flag==1)
  {
       ui8Data[0] = ChipAddress;                  /* i2c slave address - WR */
       ui8Data[1] = RegAddress;   /* the register address */
       memcpy(&ui8Data[2],data,data_number);  /* the data to write */
       ui8Cmd[0]  = IIC_START;
       ui8Cmd[1]  = IIC_DATA;
      for(i=2;i<2+data_number;i++)
       {
             ui8Cmd[i]  = IIC_DATA;
       } 
/*       ui8Cmd[2]  = IIC_DATA;
      ui8Cmd[3]  = IIC_STOP;*/
       ui8Cmd[2+data_number]  = IIC_STOP;
//   iicTransBuf.dwCount = 4;
     iicTransBuf.dwCount = 3+data_number;
       
       iicTransBuf.pData   = ui8Data;
        iicTransBuf.pCmd    = ui8Cmd;
       if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_GX1001) != TRUE )
       {  /* if the i2c transaction is failed, tell me the reason. */
 //           trace("write reg fail....\n");
            return (FAILURE);
       }
       else
      {
//            trace("write reg success....\n");
            return (SUCCESS);
      }
  }
  else if(WR_flag==0)
  {
       ui8Data[0] = ChipAddress;                  /* i2c slave address - WR */
       ui8Data[1] = RegAddress;   /* the register address */
       
       ui8Cmd[0]  = IIC_START;
       ui8Cmd[1]  = IIC_DATA;
       ui8Cmd[2]  = IIC_STOP;
       iicTransBuf.dwCount = 3;
        iicTransBuf.pData   = ui8Data;
       iicTransBuf.pCmd    = ui8Cmd;
       if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_GX1001) != TRUE )
       {  /* if the i2c transaction is failed, tell me the reason. */
  //          trace("read reg fail step one....\n");
            return (FAILURE);
        }
   
         /* phase 2 */
       ui8Data[0] = ChipAddress + 1;  /* i2c slave address - RD */
       ui8Cmd[0]  = IIC_START;
       ui8Cmd[1]  = IIC_DATA;
       ui8Cmd[2]  = IIC_STOP;
       iicTransBuf.dwCount = 3;
       iicTransBuf.pData   = ui8Data;
       iicTransBuf.pCmd    = ui8Cmd;
       if ( iicTransaction(&iicTransBuf, I2C_BUS_CABLE_GX1001) != TRUE )
       {  /* if the i2c transaction is failed, tell me the reason. */
  //            trace("read reg fail step two....\n");
             return (FAILURE);
       }
       else
       {  /* if the i2c transaction is successful, copy data into register map */
             memcpy(data,&ui8Data[1],data_number);
  //            trace("read reg success step two....\n");
             return (SUCCESS);
        }
   }
      

}

/*========================================================================================================================*/
/*==============================  User-defined GX1001 software interface end =============================================*/
/*========================================================================================================================*/
/*========================================================================================================================*/





/*
Function:	Set RF frequency (Unit: KHz)
Input:
        fvalue -- RF frequency (Unit: KHz)
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Set_RFFrequency(unsigned long fvalue)
{
		int UCtmp = FAILURE;
        unsigned char data[5];
        unsigned long freq;

        freq=(fvalue+GX_IF_FREQUENCY)*10/625;              /*freq=(fvalue+GX_IF_FREQUENCY)*/
        data[0] = 0xc0;	                                /*Tunner Address*/
        data[1] =(unsigned char)((freq>>8)&0xff);	
        data[2] =(unsigned char)(freq&0xff);	
        data[3] = 0x86;	/*62.5KHz*/
        data[4] = 0x08;
     
        if (SUCCESS == GX_Set_Tunner_Repeater_Enable(1))	/*open the chip repeater */
		{
			if (SUCCESS == GX_I2cReadWrite( WRITE, data[0],data[1],&data[2], 3 ))
			{
				GX_Delay_N_ms(50);
				UCtmp = GX_Set_Tunner_Repeater_Enable(0);	/*close the chip repeater*/
			}
		}

		if (SUCCESS == UCtmp)
		{
            GX_Delay_N_ms(50);
			UCtmp = GX_HotReset_CHIP();
		}
		return UCtmp;
}



/*
Function:   Set AGC parameter
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Set_AGC_Parameter(void)
{
       int r;
	return (GX_Write_one_Byte(GX_AGC2_THRES,0x2d));		/* set the AGC2 parameter */
	
}


/*
Function:   get the signal intensity expressed in percentage
Output:
        The signal Strength value  ( Range is [0,100] )
*/
unsigned char GX_Get_Signal_Strength(void)
{
    int iAGC1_word=300,iAGC2_word=300,Amp_Value;
	int read_i=0, agc1_temp=0,agc2_temp=0;
    
    //the following parameters are specific for certain tuner
    int C0=130;
    int C1=61,  A1=-7;
    int C2=108, A2=-27;
    int C3=14,  A3=9;
    int C4=119, A4=-6;
    //-----------------------------------------------
        
	int i=0;

	while (i<40)
	{
		agc1_temp =GX_Read_one_Byte(GX_AGC1_CTRL);
		agc2_temp =GX_Read_one_Byte(GX_AGC2_CTRL);

		
		if ((agc1_temp>0)&&(agc2_temp>0))
		{

			if ((abs(agc1_temp - iAGC1_word)<5)&&(abs(agc2_temp - iAGC2_word)<5))
			{
				break;
			}
			
			iAGC1_word = agc1_temp;
			iAGC2_word = agc2_temp;
		}

		GX_Delay_N_ms(10);
		i++;

	}

	if (i>=40) 
	{
		iAGC1_word =GX_Read_one_Byte(GX_AGC1_CTRL);
		iAGC2_word =GX_Read_one_Byte(GX_AGC2_CTRL);
	}

	if (iAGC1_word > 0xdf) iAGC1_word = 0xdf;
	Amp_Value = C0 - ((iAGC1_word-C1)*(A1-A2))/(C2-C1) - ((iAGC2_word-C3)*(A3-A4))/(C4-C3);
	return GX_Change2percent(Amp_Value,0,100);

}         
