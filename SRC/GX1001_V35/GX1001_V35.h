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

typedef int GX_STATE;

#define WRITE	1
#define READ  	0

#define SUCCESS 1
#define FAILURE	-1	//don't set 0 


/*-- Register Address Defination begin ---------------*/

#define GX_CHIP_ID                 0x00
#define GX_MAN_PARA                0x10
#define GX_INT_PO1_SEL             0x11
#define GX_SYSOK_PO2_SEL           0x12
#define GX_STATE_IND               0x13
#define GX_TST_SEL                 0x14
#define GX_I2C_RST                 0x15
#define GX_MAN_RST                 0x16
#define GX_BIST                    0x18
#define GX_MODE_AGC                0x20
#define GX_AGC_PARA                0x21
#define GX_AGC2_THRES              0x22
#define GX_AGC12_RATIO             0x23
#define GX_AGC_STD                 0x24
#define GX_SCAN_TIME               0x25
#define GX_DCO_CENTER_H            0x26
#define GX_DCO_CENTER_L            0x27
#define GX_BBC_TST_SEL             0x28
#define GX_AGC_ERR_MEAN            0x2B
#define GX_FREQ_OFFSET_H           0x2C
#define GX_FREQ_OFFSET_L           0x2D
#define GX_AGC1_CTRL               0x2E
#define GX_AGC2_CTRL               0x2F
#define GX_FSAMPLE_H               0x40
#define GX_FSAMPLE_M               0x41
#define GX_FSAMPLE_L               0x42
#define GX_SYMB_RATE_H             0x43
#define GX_SYMB_RATE_M             0x44
#define GX_SYMB_RATE_L             0x45
#define GX_TIM_LOOP_CTRL_L         0x46
#define GX_TIM_LOOP_CTRL_H         0x47
#define GX_TIM_LOOP_BW             0x48
#define GX_EQU_CTRL                0x50
#define GX_SUM_ERR_POW_L           0x51
#define GX_SUM_ERR_POW_H           0x52
#define GX_EQU_BYPASS              0x53
#define GX_EQU_TST_SEL             0x54
#define GX_EQU_COEF_L              0x55
#define GX_EQU_COEF_M              0x56
#define GX_EQU_COEF_H              0x57
#define GX_EQU_IND                 0x58
#define GX_RSD_CONFIG              0x80
#define GX_ERR_SUM_1               0x81
#define GX_ERR_SUM_2               0x82
#define GX_ERR_SUM_3               0x83
#define GX_ERR_SUM_4               0x84
#define GX_RSD_DEFAULT             0x85
#define GX_OUT_FORMAT              0x90

/*-- Register Address Defination end ---------------*/


/* -------------------------- User-defined GX1001 software interface begin  ---------------------*/

void GX_Delay_N_ms(int ms_value);

GX_STATE GX_I2cReadWrite(int WR_flag, unsigned char ChipAddress,unsigned char RegAddress,unsigned char *data, int data_number);

/*-------------------------- User-defined GX1001 software interface end -------------------------*/

GX_STATE GX_Set_RFFrequency(unsigned long fvalue);

GX_STATE GX_Set_AGC_Parameter(void);


/*-------------------------- Control set begin ------------------------*/

GX_STATE GX_Write_one_Byte(int RegAddress,int WriteValue);

GX_STATE GX_Write_one_Byte_NoReadTest(int RegAddress,int WriteValue);

int GX_Read_one_Byte(int RegAddress);

GX_STATE GX_Set_Sleep(int Sleep_Enable);

GX_STATE GX_Init_Chip(void);

GX_STATE GX_HotReset_CHIP(void);

GX_STATE GX_CoolReset_CHIP(void);

GX_STATE GX_Set_OutputMode(int mode);

GX_STATE GX_Select_DVB_QAM_Size(int size);

GX_STATE GX_Read_ALL_OK(void);

GX_STATE GX_Read_EQU_OK(void);

GX_STATE GX_Set_Tunner_Repeater_Enable(int OnOff);

/*-------------------------- Control set end ---------------------------*/


/*--------------- Symbol recovery begin --------------------------------*/

GX_STATE GX_SetSymbolRate(unsigned long Symbol_Rate_Value);
GX_STATE GX_SetOSCFreq(void);

/*--------------- Symbol recovery end ----------------------------------*/


//===========================================================================================================================================

unsigned char GX_Change2percent(int value,int low,int high);
int GX_100Log(int);
unsigned char GX_Get_SNR(void);
unsigned char GX_Get_Signal_Strength(void);
GX_STATE GX_Get_ErrorRate(int *E_param);


GX_STATE GX_SetSpecInvert(int Spec_invert);
GX_STATE GX_Search_Signal(unsigned long Symbol_1,unsigned long Symbol_2,int Spec_Mode,int Qam_Size,unsigned long RF_Freq,int Wait_OK_X_ms);


//=======================================================================================================================================