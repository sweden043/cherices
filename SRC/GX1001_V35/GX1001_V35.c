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
#include "GX1001_V35.h"

extern int  GX_CHIP_ADDRESS;    //GX1001 chip address
extern int  GX_OSCILLATE_FREQ;	//oscillate frequency  
extern int  GX_IF_FREQUENCY;	//tuner IF center frequency

extern int  GX_TS_OUTPUT_MODE;  //TS output mode
/* 
Function: Write one byte to chip
Input:
        RegAdddress -- The register address
        WriteValue  -- The write value
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Write_one_Byte(int RegAddress,int WriteValue)
{
	int UCtmp=FAILURE;
	unsigned char data[2];
	unsigned int ChipAddress ;
	
	ChipAddress  = GX_CHIP_ADDRESS;
	
	data[0] = (unsigned char)RegAddress;
	data[1] = (unsigned char)WriteValue; 
	
	UCtmp = GX_I2cReadWrite( WRITE, ChipAddress,data[0],&data[1], 1 );
	
	if (SUCCESS == UCtmp)//ok
	{
		if ((WriteValue&0xff) == (GX_Read_one_Byte(RegAddress)&0xff))
			return SUCCESS;
	}
	return FAILURE;
}


/* 
Function: Write one byte to chip with no read test 
Input:
        RegAdddress -- The register address
        WriteValue  -- The write value
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Write_one_Byte_NoReadTest(int RegAddress,int WriteValue)
{
	int UCtmp=FAILURE;
	unsigned char data[2];
	unsigned int ChipAddress ;
	
	ChipAddress  = GX_CHIP_ADDRESS;
	
	data[0] = (unsigned char)RegAddress;
	data[1] = (unsigned char)WriteValue; 
	
	UCtmp = GX_I2cReadWrite( WRITE, ChipAddress,data[0],&data[1], 1 );

	return UCtmp;
}



/* 
Function: Read one byte from chip
Input:
        RegAdddress -- The register address
Output:
        success: Read value
        FAILURE:  FAILURE 
*/
int GX_Read_one_Byte(int RegAddress)
{
	int UCtmp=FAILURE;
	unsigned int ChipAddress;
	unsigned char Temp_RegAddress=0;
    unsigned char Temp_Read_data =0;

	Temp_RegAddress = (unsigned char)RegAddress;
	ChipAddress=GX_CHIP_ADDRESS;

	UCtmp = GX_I2cReadWrite(READ, ChipAddress,Temp_RegAddress,&Temp_Read_data, 1 );

	if (SUCCESS == UCtmp)//ok
		return (Temp_Read_data&0xff) ;
	else
		return FAILURE;
}





/*
Function:  Init the GX1001p 
Input:  
        TS_OutputMode -- 1: Parallel output
				         0: Serial output
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Init_Chip(void)
{ 
    int UCtmp=FAILURE;

	UCtmp = GX_Set_AGC_Parameter(); //Set AGC parameter
	
	if (SUCCESS == UCtmp)
	{
		UCtmp = GX_SetOSCFreq();                        /* set crystal frequency */  

		if (SUCCESS == UCtmp)
		{
			UCtmp = GX_Set_OutputMode(GX_TS_OUTPUT_MODE);    /* set the TS output mode */
		}
	}
	return UCtmp;
}




/*
Function:	Search signal with setted parameters
Input:
	    Symbol_Rate_1   --  Used first symbol rate value (range: 450 -- 9000)     (Unit: kHz)
	    Symbol_Rate_2	--  Used second symbol rate value. Please set 0 if no use	(Unit: kHz)
	
	    Spec_Mode	    --	0：search only positive spectrum
	    			    	1：search only negative spectrum
	    			    	2：first positive and then negative spectrum
	    			    	3：first negative and then positive spectrum
	                    
	    Qam_Size	    --  0-2 = reserved;
	    			    	3 = 16QAM;
	    			    	4 = 32QAM; 
	    			    	5 = 64QAM; 
	    			    	6 = 128QAM;
	    			    	7 = 256QAM.
	                    
	    RF_Freq		    --  The RF frequency (KHz)
                        
	    Wait_OK_X_ms    --  The waiting time before give up one search ( Unit: ms )
	    			        (Range: 250ms -- 2000ms, Recommend: 700ms)


Output:
		SUCCESS --  Have signal
		FAILURE	--  No signal
*/
GX_STATE GX_Search_Signal(  unsigned long Symbol_Rate_1,
                            unsigned long Symbol_Rate_2,
                            int Spec_Mode,
                            int Qam_Size,
                            unsigned long RF_Freq,
                            int Wait_OK_X_ms)
{
	int After_EQU_OK_Delay	= 35;	//35 ms,  
	int spec_invert_enable	= 0;	//spec invert enable flag
	int spec_invert_value	= 0;	//next spec invert value
	int symbol_2_enable		= 0;	//Symbol_Rate_2 enable flag

    int wait_ok_x_ms_temp	= 0;    //for save Wait_OK_X_ms
    //-----------------------------------------------------------------------------
    trace("symbol is %d ,frenq is %d,qam is %d\n",Symbol_Rate_1,RF_Freq,Qam_Size);
   if (FAILURE == GX_CoolReset_CHIP()) return FAILURE;
   if (FAILURE == GX_Init_Chip()) return FAILURE;

	//-----------------------------------------------------------------------------
	wait_ok_x_ms_temp = Wait_OK_X_ms/10;	//as 700 ms = 70 * 10ms_Delay

	if (FAILURE == GX_Select_DVB_QAM_Size(Qam_Size)) return FAILURE;	//Set QAM size

	if (FAILURE == GX_SetSymbolRate(Symbol_Rate_1)) return FAILURE;	//Set Symbol rate value

	if (FAILURE == GX_Set_RFFrequency(RF_Freq))	return FAILURE;		//Set tuner frequency

	if (Symbol_Rate_2 >= 4500) symbol_2_enable = 1;	//Symbol_Rate_2 enable

    if (Symbol_Rate_1<25000) After_EQU_OK_Delay = 100;   //100ms   (if  <2.5M  = 100ms)

	//-----------------------------------------------------------------------------

SYMBOL_2_SEARCH:
	switch (Spec_Mode)
	{
	case 3:	// first negative and then positive
		{
			spec_invert_enable = 1;
			spec_invert_value  = 0;	//next spec invert value
		}
	case 1:	//negative
		{
			GX_SetSpecInvert(1);
		}
		break;
	case 2:// first positive and then negative
		{
			spec_invert_enable = 1;
			spec_invert_value  = 1;	//next spec invert value
		}
	default://positive
		{
			GX_SetSpecInvert(0);
		}
		break;
	}
	//-----------------------------------------------------------------------------

SPEC_INVERT_SEARCH:	
	if (FAILURE == GX_HotReset_CHIP()) return FAILURE;

    wait_ok_x_ms_temp = Wait_OK_X_ms/10;	//as 700 ms = 70 * 10ms_Delay

	while ((FAILURE == GX_Read_EQU_OK()) && (wait_ok_x_ms_temp))
	{
		wait_ok_x_ms_temp --;
		GX_Delay_N_ms(10);		//Delay 10 ms
	}

	if ( 0 == wait_ok_x_ms_temp) return FAILURE;	//Read EQU time over

	GX_Delay_N_ms(After_EQU_OK_Delay);		//Delay After_EQU_OK_Delay ms

	if (SUCCESS == GX_Read_ALL_OK())	//All ok
	{
		return SUCCESS;
	}
	else
	{
		if (spec_invert_enable)
		{
			spec_invert_enable = 0;				//disable spec invert
			if (FAILURE == GX_SetSpecInvert(spec_invert_value))  return FAILURE;	//spec invert
			else
				goto SPEC_INVERT_SEARCH;
		}
		else
		{
			if (symbol_2_enable)
			{
				symbol_2_enable = 0;

                if (Symbol_Rate_2<25000) 
                    After_EQU_OK_Delay = 100;   //100ms
                else
                    After_EQU_OK_Delay = 35;   //35ms

				if (FAILURE == GX_SetSymbolRate(Symbol_Rate_2)) 
                    return FAILURE;	//Set Symbol rate value
				else
					goto SYMBOL_2_SEARCH;
			}
			else
			{
				return FAILURE;
			}
		}
	}

}


//========================================================================================================================
//========================================================================================================================
//========================================================================================================================
//========================================================================================================================



/*
Funtion:	Set chip wake up
Input:
		Sleep_Enable    --    1: Sleep
					          0: Working
Output:
		SUCCESS or FAILURE
*/
GX_STATE GX_Set_Sleep(int Sleep_Enable)
{
	
	int temp=0;
	int UCtmp = FAILURE;
	temp=GX_Read_one_Byte(GX_MAN_PARA);	/*0x10 - bit2*/
	if (temp != FAILURE)
	{
		temp &=0xfb;
		
		if (Sleep_Enable) //Sleep
			temp |= 0x04;

		UCtmp = GX_Write_one_Byte(GX_MAN_PARA,temp);

		if (SUCCESS == UCtmp)
		{
			if (Sleep_Enable == 0)
			{
				UCtmp =GX_HotReset_CHIP();
			}
		}
	}
	return UCtmp;
}




/*
Function: Set TS output mode
Input:
		0 - Serial
		1 - Parallel
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_Set_OutputMode(int mode)
{
	int temp=0;
	int UCtmp = FAILURE;
	temp=GX_Read_one_Byte(GX_OUT_FORMAT);	/*0x90 - bit6*/

	if (temp != FAILURE)
	{
		temp &= 0xbf;
		if (mode) temp+=0x40;

		UCtmp = GX_Write_one_Byte(GX_OUT_FORMAT,temp);
	}
	return UCtmp;
}



/* 
Function: Select QAM size (4 - 256), only for DVB.
Input:
        size  --  0-2 = reserved;
			        3 = 16QAM;
			        4 = 32QAM; 
			        5 = 64QAM; 
			        6 = 128QAM;
			        7 = 256QAM.
utput:
        SUCCESS or FAILURE
*/
GX_STATE GX_Select_DVB_QAM_Size(int size)
{
    int r,temp=0;
	int UCtmp = FAILURE;

	if ((size>7)||(size<=2)) size = 5;
    size<<=5;

	temp=GX_Read_one_Byte(GX_MODE_AGC);

	if (temp != FAILURE)
	{
		temp &= 0x1f;
		temp += size;
		UCtmp = GX_Write_one_Byte(GX_MODE_AGC,temp);  /*0x20 - bit7:5   */
		r=GX_Read_one_Byte(GX_MODE_AGC);
		trace("QAM_Size is %d\n",r);
	}
	return UCtmp;
}



/* 
Function: Set symbol rate 
Input:
        Symbol_Rate_Value :  The range is from 450 to 9000	(Unit: kHz)
        
Output:
        SUCCESS or FAILURE
*/
GX_STATE GX_SetSymbolRate(unsigned long Symbol_Rate_Value)
{
	int r,UCtmp = FAILURE;
    unsigned long temp_value=0;

    temp_value = Symbol_Rate_Value*1000;        

	UCtmp =	GX_Write_one_Byte(GX_SYMB_RATE_H,((int)((temp_value>>16)&0xff)));	/*0x43*/
   
	if (SUCCESS == UCtmp)
	{
		UCtmp = GX_Write_one_Byte(GX_SYMB_RATE_M,((int)((temp_value>>8)&0xff)));	/*0x44*/

		if (SUCCESS == UCtmp)
		{
			UCtmp = GX_Write_one_Byte(GX_SYMB_RATE_L,((int)( temp_value&0xff))); /*0x45*/
			r=GX_Read_one_Byte(GX_SYMB_RATE_L);
		       trace("GX_SYMB_RATE_L is %d\n",r);
		}
	}
	return UCtmp;
}


/*
Function: Set oscillate frequancy 
Output:
        SUCCESS or FAILURE 
*/
GX_STATE GX_SetOSCFreq(void)
{
	int r,UCtmp = FAILURE;
	unsigned long temp=0;
    unsigned long OSC_frequancy_Value =0; 

    OSC_frequancy_Value = GX_OSCILLATE_FREQ;       // KHz
        
    temp=OSC_frequancy_Value*250;  

	UCtmp =GX_Write_one_Byte(GX_FSAMPLE_H,((int)((temp>>16)&0xff)));       //0x40
    r=GX_Read_one_Byte(GX_FSAMPLE_H);
		       trace("GX_FSAMPLE_H  is %x\n",r);
	if (SUCCESS == UCtmp)
	{
		UCtmp = GX_Write_one_Byte(GX_FSAMPLE_M,((int)((temp>>8)&0xff)));   //0x41
              r=GX_Read_one_Byte(GX_FSAMPLE_M);
		       trace("GX_FSAMPLE_m  is %x\n",r);
		if (SUCCESS == UCtmp)
		{
			UCtmp = GX_Write_one_Byte(GX_FSAMPLE_L,((int)( temp&0xff)));   //0x42
			r=GX_Read_one_Byte(GX_FSAMPLE_L);
		       trace("GX_FSAMPLE_L  is %x\n",r);
		}
	}
	return UCtmp;
}



/* 
Function: Hot reset the Chip 
Output:
        SUCCESS or FAILURE 
*/
GX_STATE GX_HotReset_CHIP(void)
{
	int UCtmp = FAILURE;
	int temp;

	temp=GX_Read_one_Byte(GX_MAN_PARA);

	if (temp != FAILURE)
	{
		temp|=0x02;
		UCtmp = GX_Write_one_Byte_NoReadTest(GX_MAN_PARA,temp);
	}

	return UCtmp;
}



/* 
Function: Cool reset the Chip 
Output:
        SUCCESS or FAILURE 
*/
GX_STATE GX_CoolReset_CHIP(void)
{
	int UCtmp = FAILURE;
	int temp;

	temp=GX_Read_one_Byte(GX_MAN_PARA);

	if (temp != FAILURE)
	{
		temp|=0x08;
		UCtmp = GX_Write_one_Byte_NoReadTest(GX_MAN_PARA,temp);
	}

	return UCtmp;
}



/* 
Function: Read EQU OK
Output:
        SUCCESS - EQU OK, FAILURE - EQU Fail
*/
GX_STATE GX_Read_EQU_OK(void)
{
	int Read_temp=0;

	Read_temp=GX_Read_one_Byte(GX_STATE_IND);         /*0x13*/

	if (Read_temp != FAILURE)
	{
	    //   trace("GX_Read_EQU_OK  is ....%x\n",Read_temp);
		if ((Read_temp&0xe0)==0xe0)                     
			return SUCCESS;
	}
	return FAILURE;
}

/* 
Function: Read ALL OK
Output:
        SUCCESS - all ok, FAILURE - not all ok 
*/
GX_STATE GX_Read_ALL_OK(void)
{
	int Read_temp=0;

	Read_temp=GX_Read_one_Byte(GX_STATE_IND);         /*0x13*/

	if (Read_temp != FAILURE)
	{
	  //     trace("GX_Read_ALL_OK  is ....%x\n",Read_temp);
		if ((Read_temp&0xf1)==0xf1)                 /*DVB-C : 0xF1*/
			return SUCCESS;
	}
	return FAILURE;
}


/* Function: Enable/Disable the Tunner repeater
Input:	
        1 - On
		0 - Off
Output:
        SUCCESS or FAILURE 
*/
GX_STATE GX_Set_Tunner_Repeater_Enable(int OnOff)
{
	int UCtmp = FAILURE;
	int Read_temp;

	Read_temp=GX_Read_one_Byte(GX_MAN_PARA);

	if (Read_temp != FAILURE)
	{
		if(OnOff)
		{
			Read_temp|=0x40;        /*Open*/
		}
		else
		{
			Read_temp&=0xbf;        /*Close*/
		}
	
		UCtmp = GX_Write_one_Byte(GX_MAN_PARA,Read_temp);
	}

	return UCtmp;
}



//==============================================================================================

/*
Function:   convert a integer to percentage ranging form 0% to 100%  
Input:
        value - integer
        low   - lower limit of input,corresponding to 0%  .if value <= low ,return 0
        high  - upper limit of input,corresponding to 100%.if value >= high,return 100
Output:
        0~100 - percentage
*/
unsigned char GX_Change2percent(int value,int low,int high)
{
    unsigned char temp=0;
    if (value<=low) return 0;
    if (value>=high) return 100;
    temp = (unsigned char)((value-low)*100/(high-low));
    return temp;
}


/* 
Function:   100LogN calculating function 
Output:
        = 100LogN
*/
int GX_100Log(int iNumber_N)
{
	int iLeftMoveCount_M=0;
	int iChangeN_Y=0;
	int iBuMaY_X=0;
	int iReturn_value=0;
	long iTemp=0,iResult=0,k=0;

	iChangeN_Y=iNumber_N;
	
	for (iLeftMoveCount_M=0;iLeftMoveCount_M<16;iLeftMoveCount_M++)
	{
		if ((iChangeN_Y&0x8000)==0x8000)
			break;
		else
		{
			iChangeN_Y=iNumber_N<<iLeftMoveCount_M;
		}
	}

	iBuMaY_X=0x10000-iChangeN_Y;	//get 2's complement

	k=(long)iBuMaY_X*10000/65536;

	//iTemp= k+(1/2)*(k*k)+(1/3)*(k*k*k)+(1/4)*(k*k*k*k)
    iTemp = k + (k*k)/20000 + ((k*k/10000)*(k*33/100))/10000 + ((k*k/100000)*(k*k/100000))/400;

	//iResult=4.816480-(iTemp/2.302585);
	iResult=48165-(iTemp*10000/23025);	//4.8165 = lg2^16

	k=iResult-3010*(iLeftMoveCount_M-1);
	
	iReturn_value=(k/100);   //magnify logN by 100 times
	
	return iReturn_value;
}       
        
        
        
/*
Function : get the signal quality expressed in percentage
Output:
        The SNR value (range is [0,100])   ( 0 express SNR = 5 dB ,  100  express SNR = 35 dB )
*/
unsigned char  GX_Get_SNR(void)
{       
	int S_N_value=0,read_temp=0;
	int read_temp1=0;
	int read_temp2=0;

	if (GX_Read_ALL_OK()==SUCCESS)
	{
		read_temp1 =( GX_Read_one_Byte(GX_SUM_ERR_POW_L)&0xff);
		read_temp2 =( GX_Read_one_Byte(GX_SUM_ERR_POW_H)&0xff);
		if ((read_temp1>0)||(read_temp2>0))
		{
			read_temp = read_temp1 + (read_temp2<<8);       //SN= 49.3-10log(read_temp) 
			S_N_value = 493 - GX_100Log(read_temp);         //magnifid by 10 times
			//return GX_Change2percent(S_N_value,50,350);	
			return S_N_value/10;
		}
	}
	return 0;
}       
 
//========================================================================================================================




/*
Function: Set spectrum invert
Input:   
       Spec_invert         : 1 - Yes, 0 - No.
Output:SUCCESS or FAILURE 
*/
GX_STATE GX_SetSpecInvert(int Spec_invert)
{
    int write_value=0;
	long OSC_frequancy_Value	=	GX_OSCILLATE_FREQ;		
	long Carrier_center			=	GX_IF_FREQUENCY;       

	if (Carrier_center<OSC_frequancy_Value)
	{
	      if (Spec_invert)
	              write_value=(int)(((OSC_frequancy_Value-Carrier_center)*1000)/1024);
	      else
	              write_value=(int)((Carrier_center*1000)/1024);
	}
	else
	{
	       if (Spec_invert)
	               write_value=(int)((((2*OSC_frequancy_Value-Carrier_center)*1000)/1024));
	       else
	               write_value=(int)(((Carrier_center-OSC_frequancy_Value)*1000)/1024);
	}
	
	if (SUCCESS == GX_Write_one_Byte(GX_DCO_CENTER_H,(((write_value>>8)&0xff))))           //0x26
	{
		if (SUCCESS ==	GX_Write_one_Byte(GX_DCO_CENTER_L,(( write_value&0xff))))          //0x27
			return SUCCESS;
	}
	return FAILURE;
}


//========================================================================================================================


/*
Function: 	Get Error Rate value 
Input:		
			*E_param: for get the exponent of E
Output:
          	FAILURE:    Read Error
          	other: 		Error Rate Value
          	
Example:	if  return value = 456 and  E_param = 3 ,then here means the Error Rate value is : 4.56 E-3
*/
GX_STATE GX_Get_ErrorRate(int *E_param)
{
	int flag = 0;
	int e_value = 0;
	int return_value = 0;
	int temp=0;
	unsigned char Read_Value[4];
	unsigned long Error_count=0;
	int i=0;
	unsigned long divied = 53477376;	//(2^20 * 51)

	*E_param = 0;

	if (GX_Read_ALL_OK() == FAILURE)
	{
		*E_param = 0;
		return 1;
	}
	
	for (i=0;i<4;i++)	//Read Error Count
	{
		flag = GX_Read_one_Byte(GX_ERR_SUM_1 + i);
		if (FAILURE == flag)
		{
			*E_param = 0;
			return 1;
		}
		else
		{
			Read_Value[i] = (unsigned char)flag;
		}
	}
	Read_Value[3] &= 0x03;
	Error_count = (unsigned long)(Read_Value[0]+ (Read_Value[1]<<8) + (Read_Value[2]<<16) + (Read_Value[3]<<24));
	
	//ERROR RATE = (ERROR COUNT) /（ ( 2^(2* 5 +5))*204*8）    //bit      
	
	for (i=0;i<20;i++)
	{
		temp = Error_count/divied;

		if (temp)
		{
			return_value = Error_count/(divied/100);
			break;
		}
		else
		{
			e_value +=1;
			Error_count *=10;
		}
	}
	*E_param = e_value;
	return return_value;
}


//========================================================================================================================
