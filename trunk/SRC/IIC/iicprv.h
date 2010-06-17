/*****************************************************************************/
/* File: iicprv.h                                                            */
/*                                                                           */
/* Module: I2C Driver                                                        */
/*                                                                           */
/* Description: Private include file.                                        */
/*****************************************************************************/
/*****************************************************************************
$Header: iicprv.h, 10, 1/30/03 9:24:28 PM, Billy Jackman$
*****************************************************************************/
#ifndef _IICPRV_H_
#define _IICPRV_H_

#ifdef _DEBUG
#define DEBUG
#endif /* _DEBUG */

#ifndef NULL
#define NULL 0
#endif /* NULL */

#ifndef BYTE
#define BYTE   u_int8
#endif /* BYTE */
#ifndef DWORD
#define DWORD  u_int32
#endif /* DWORD */

/* Debug messaging functions */
/* @@@SG commented out - dprintf is now a function which calls trace_new */
/*#ifdef DEBUG */
/*#define dprintf trace */
/*#else */ /* DEBUG */
/*#define dprintf if (0) ((int (*)(char *, ...)) 0) */
/*#endif */ /* DEBUG */

#define IIC_ERROR_TIMEOUT              100000  /* Time in uSec for bus timeout */

#if IIC_TYPE == IIC_TYPE_COLORADO

#if (EMULATION_LEVEL != FINAL_HARDWARE)
#define IIC_WRITE_COMMAND (I2C_MASTER_CTLW_TRANSMIT | I2C_MASTER_CTLW_INTENABLE | I2C_MASTER_CTLW_MODE400KHZ)
#define IIC_READ_COMMAND  (I2C_MASTER_CTLW_RECEIVE | I2C_MASTER_CTLW_INTENABLE)
#define IIC_START_COMMAND (I2C_MASTER_CTLW_STARTTRANSMIT | I2C_MASTER_CTLW_INTENABLE)
#define IIC_STOP_COMMAND  (I2C_MASTER_CTLW_STOPTRANSMIT | I2C_MASTER_CTLW_INTENABLE)
#define IIC_ERROR_STOP_COMMAND (I2C_MASTER_CTLW_STOPTRANSMIT)
#else
#define IIC_WRITE_COMMAND (I2C_MASTER_CTLW_TRANSMIT | I2C_MASTER_CTLW_INTENABLE)
#define IIC_READ_COMMAND  (I2C_MASTER_CTLW_RECEIVE | I2C_MASTER_CTLW_INTENABLE)
#define IIC_START_COMMAND (I2C_MASTER_CTLW_STARTTRANSMIT | I2C_MASTER_CTLW_INTENABLE)
#define IIC_STOP_COMMAND  (I2C_MASTER_CTLW_STOPTRANSMIT | I2C_MASTER_CTLW_INTENABLE)
#define IIC_ERROR_STOP_COMMAND (I2C_MASTER_CTLW_STOPTRANSMIT)
#endif /* EMULATION_LEVEL */

#else /* IIC_TYPE == IIC_TYPE_COLORADO */

#define CNXT_IIC_BASE_CLK_FREQUENCY    54000000 /* ASX Clk - 54 MHz  */
#define CNXT_IIC_CLK_DIVIDER_100KHZ    540      /* Standard IIC Mode */
#define CNXT_IIC_CLK_DIVIDER_400KHZ    135      /* Fast IIC Mode     */
#define CNXT_IIC_CLK_DIVIDER_1000KHZ   54       /* High speed Mode   */
#define CNXT_IIC_CLK_DIVIDER_3400KHZ   16       /* Upper limit of High Speed 
                                                   Mode: 400-3400KHz */

/* Register access macros */
#define     CNXT_IIC_MODE_SCL_OVERRIDE(val, pDest)                             \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_SCL_OVRIDE_MASK) |  \
                         ((val<<CNXT_IIC_MODE_SCL_OVRIDE_SHIFT) &              \
                          CNXT_IIC_MODE_SCL_OVRIDE_MASK)))
#define     CNXT_IIC_MODE_SDA_OVERRIDE(val, pDest)                             \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_SDA_OVRIDE_MASK) |  \
                         ((val<<CNXT_IIC_MODE_SDA_OVRIDE_SHIFT) &              \
                          CNXT_IIC_MODE_SDA_OVRIDE_MASK)))
#define     CNXT_IIC_MODE_HWCTRL_ENABLE(val, pDest)                            \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_HWCTRL_EN_MASK) |   \
                         ((val<<CNXT_IIC_MODE_HWCTRL_EN_SHIFT) &               \
                          CNXT_IIC_MODE_HWCTRL_EN_MASK)))
#define     CNXT_IIC_MODE_WAITST_ENABLE(val, pDest)                            \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_WAITST_EN_MASK) |   \
                         ((val<<CNXT_IIC_MODE_WAITST_EN_SHIFT) &               \
                          CNXT_IIC_MODE_WAITST_EN_MASK)))
#define     CNXT_IIC_MODE_MULTIMAST_ENABLE(val, pDest)                         \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_MULTIMAST_EN_MASK) |\
                         ((val<<CNXT_IIC_MODE_MULTIMAST_EN_SHIFT) &            \
                          CNXT_IIC_MODE_MULTIMAST_EN_MASK)))
 #if IIC_TYPE == IIC_TYPE_BRAZOS
#define     CNXT_IIC_MODE_BYTEORDER_LI_ENABLE(val, pDest)                      \
                  (*(LPREG)(pDest) =                                           \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_MODE_BYTEORDER_LI_MASK) |\
                         ((val<<CNXT_IIC_MODE_BYTEORDER_LI_SHIFT) &            \
                          CNXT_IIC_MODE_BYTEORDER_LI_MASK)))
 #endif /* IIC_TYPE == IIC_TYPE_BRAZOS */
#define     CNXT_IIC_MODE_CLKDIV_SET(val, pDest)                               \
                   (*(LPREG)(pDest) |= ((val<<CNXT_IIC_MODE_CLKDIV_VALUE_SHIFT)&\
                                       CNXT_IIC_MODE_CLKDIV_VALUE_MASK))
#define     CNXT_IIC_MODE_CLKDIV_GET()                                         \
                  ((*(LPREG)(CNXT_IIC_MODE_REG) &                              \
                    CNXT_IIC_MODE_CLKDIV_VALUE_MASK) >>                        \
                   CNXT_IIC_MODE_CLKDIV_VALUE_SHIFT)

#define     CNXT_IIC_CTRL_NUMBYTES_SET(val, pDest)                             \
                  (*(LPREG)(pDest) |= (((val-1)<<CNXT_IIC_CTRL_NUMBYTES_SHIFT) \
                                      & CNXT_IIC_CTRL_NUMBYTES_MASK))
#define     CNXT_IIC_CTRL_READACK_ENABLE(val, pDest)                           \
                   (*(LPREG)(pDest) |=                                         \
                        ((*(LPREG)(pDest) & ~CNXT_IIC_CTRL_READACK_MASK) |     \
                          ((val<<CNXT_IIC_CTRL_READACK_SHIFT) &                \
                           CNXT_IIC_CTRL_READACK_MASK)))
#define     CNXT_IIC_CTRL_START_ENABLE(val, pDest)                             \
                  (*(LPREG)(pDest) |= ((*(LPREG)(pDest) &                      \
                                    ~CNXT_IIC_CTRL_START_MASK)                 \
                                     | ((val<<CNXT_IIC_CTRL_START_SHIFT) &     \
                                       CNXT_IIC_CTRL_START_MASK)))
#define     CNXT_IIC_CTRL_STOP_ENABLE(val, pDest)                              \
                  (*(LPREG)(pDest) |= ((*(LPREG)(pDest)&~CNXT_IIC_CTRL_STOP_MASK)\
                                     | ((val<<CNXT_IIC_CTRL_STOP_SHIFT) &      \
                                        CNXT_IIC_CTRL_STOP_MASK)))
#define     CNXT_IIC_CTRL_WRITEDATA_MASK(idx)                                  \
                  (CNXT_IIC_CTRL_WRITEDATA_INST_MASK <<                        \
                   (CNXT_IIC_CTRL_WRITEDATA_INST_WIDTH *                       \
                    ((idx%CNXT_IIC_CTRL_WRITEDATA_NUM_INST)+1)))
#define     CNXT_IIC_CTRL_WRITEDATA_SHIFT(idx)                                 \
                  (CNXT_IIC_CTRL_WRITEDATA_INST_WIDTH *                        \
                   ((idx%CNXT_IIC_CTRL_WRITEDATA_NUM_INST)+1))
#define     CNXT_IIC_CTRL_WRITEDATA_SET(NumBytes, pSource, pDest)              \
                  {                                                            \
                     LPREG pStore = (LPREG)pDest;                              \
                     char *pSrc = (char *)pSource;                             \
                     int num_bytes = (int)NumBytes;                            \
                     if((pStore != (LPREG)NULL) && (pSrc != (char *)NULL))     \
                     {                                                         \
                        while(num_bytes--)                                     \
                        {                                                      \
                           *(LPREG)pStore |= (*(char *)pSrc++ <<               \
                              ((CNXT_IIC_CTRL_WRITEDATA_NUM_INST-num_bytes) *  \
                                    CNXT_IIC_CTRL_WRITEDATA_INST_WIDTH));      \
                        }                                                      \
                     }                                                         \
                     else                                                      \
                     {                                                         \
                        printf(                                                \
                        "!NULL Pointer passed to CNXT_IIC_WRITEDATA_SET!\n");  \
                     }                                                         \
                  }

#define     CNXT_IIC_STAT_INT_IS_SET(src)                                      \
               (((*(LPREG)(src)) & CNXT_IIC_STAT_INT_MASK) >>                  \
                CNXT_IIC_STAT_INT_SHIFT)
#define     CNXT_IIC_STAT_WRITEACK_IS_SET(src)                                 \
               (((*(LPREG)(src)) & CNXT_IIC_STAT_WRITEACK_MASK) >>             \
                CNXT_IIC_STAT_WRITEACK_SHIFT)
#define     CNXT_IIC_STAT_SCL_IS_SET(src)                                      \
               (((*(LPREG)(src)) & CNXT_IIC_STAT_SCL_MASK) >>                  \
                CNXT_IIC_STAT_SCL_SHIFT)
#define     CNXT_IIC_STAT_SDA_IS_SET(src)                                      \
               (((*(LPREG)(src)) & CNXT_IIC_STAT_SDA_MASK) >>                  \
                CNXT_IIC_STAT_SDA_SHIFT)
#define     CNXT_IIC_STAT_CLEAR(src) ((*(LPREG)(src)) = 0)
#define     CNXT_IIC_READDATA_MASK(idx)                                        \
                  (CNXT_IIC_READDATA_INST_MASK <<                              \
                   (CNXT_IIC_READDATA_INST_WIDTH *                             \
                    (idx%CNXT_IIC_READDATA_NUM_INST)))
#define     CNXT_IIC_READDATA_SHIFT(idx)                                       \
                  (CNXT_IIC_READDATA_INST_WIDTH *                              \
                   (idx%CNXT_IIC_READDATA_NUM_INST))
#define     CNXT_IIC_READDATA_GET(pReg, pData, NumBytes)                             \
                  {                                                            \
                     u_int32 rdata_value = *(LPREG)(pReg);    \
                     char *pStore = (char *)pData;                             \
                     int num_bytes = (int)NumBytes;                            \
                     if(pStore != (char *)NULL)                                \
                     {                                                         \
                        while(num_bytes--)                                     \
                        {                                                      \
                           *(char *)pStore++ =                                 \
                                       (rdata_value &                          \
                                        CNXT_IIC_READDATA_MASK(num_bytes))>>   \
                                          CNXT_IIC_READDATA_SHIFT(num_bytes);  \
                        }                                                      \
                     }                                                         \
                     else                                                      \
                     {                                                         \
                        printf(                                                \
                           "!NULL Pointer passed to CNXT_IIC_READDATA_GET!\n");\
                     }                                                         \
                  }
 #if IIC_TYPE == IIC_TYPE_BRAZOS
#define     CNXT_IIC_READDATA_LI_GET(pData, NumBytes)                          \
                  {                                                            \
                     u_int32 rdata_value = *(LPREG)(CNXT_IIC_READDATA_REG);    \
                     char *pStore = (char *)pData;                             \
                     int num_bytes = 0;                                        \
                     if(pStore != (char *)NULL)                                \
                     {                                                         \
                        for(;num_bytes<NumBytes;num_bytes++)                   \
                        {                                                      \
                           *(char *)pStore++ =                                 \
                                       (rdata_value &                          \
                                        CNXT_IIC_READDATA_MASK(num_bytes))>>   \
                                          CNXT_IIC_READDATA_SHIFT(num_bytes);  \
                        }                                                      \
                     }                                                         \
                     else                                                      \
                     {                                                         \
                        printf(                                                \
                        "!NULL Pointer passed to CNXT_IIC_READDATA_LI_GET!\n");\
                     }                                                         \
                  }
 #endif /* IIC_TYPE == IIC_TYPE_BRAZOS */

#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

/* Function Prototypes */
/*-------------------- */
#if IIC_TYPE == IIC_TYPE_COLORADO
bool SendByte(BYTE byData, bool bStart);
BYTE ReadByte(bool bAck);
void i2c_Start(void);
void i2c_Stop(void);
void IICDeath(u_int32 dwDeathCode);
#else /* IIC_TYPE == IIC_TYPE_COLORADO */
void iic_hw_init(IICBUS bus);
bool SendByte(BYTE byData, bool bStart, bool bStop, IICBUS bus);
BYTE ReadByte(bool bAck, bool bStop, IICBUS bus);
void i2c_SW_Stop(IICBUS bus);
void IICDeath(u_int32 dwDeathCode, IICBUS bus);
#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

int IICIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);

#endif /* _IICPRV_H_ */

/*****************************************************************************
$Log: 
 10   mpeg      1.9         1/30/03 9:24:28 PM     Billy Jackman   SCR(s) 5364 
       :
       Re-define CNXT_IIC_READDATA_GET to include a parameter that is a pointer
        to
       the correct read data register so this can be used for multiple bus 
       setups.
       
 9    mpeg      1.8         1/22/03 3:41:58 PM     Senthil Veluswamy SCR(s) 
       5284 :
       removed hw timer callback. 
       
 8    mpeg      1.7         12/17/02 4:08:04 PM    Senthil Veluswamy SCR(s) 
       5067 :
       Removed // comments, moved $Log:$ to end of file, moved IIC macros from 
       chip header to here, modifications to support multiple IIC banks (Brazos
        and after)
       
 7    mpeg      1.6         8/30/02 7:45:10 PM     Senthil Veluswamy SCR(s) 
       4502 :
       Modifications for using the new IIC interface
       
 6    mpeg      1.5         8/11/99 6:31:40 PM     Dave Wilson     Changed KAL 
       calls to use new API.
       
 5    mpeg      1.4         11/16/98 4:09:58 PM    Steve Glennon   Increased 
       timeout for final hardware to 100000. Have seen some flakiness with
       smaller timeouts. Further debug needed at timeout of 10000 to see why 
       IICDeath is occurring, reporting timeout.
       
 4    mpeg      1.3         11/14/98 1:58:34 PM    Steve Glennon   Removed 
       400KHz mode from final hardware version. Increased final hardware 
       timeout to 10000 (10ms). LED drivers do not work at 400KHz.
       
 3    mpeg      1.2         11/13/98 4:38:32 PM    Rob Tilton      Spend up the
        final hw to 400KHz.
       
 2    mpeg      1.1         10/6/98 6:34:18 PM     Steve Glennon   Made dprintf
        into a function which calls trace_new with trace_level_1 (info)
       Hence removed #define of dprintf from include file
       
 1    mpeg      1.0         7/30/98 4:13:40 PM     Rob Tilton      
$
 * 
 *    Rev 1.9   30 Jan 2003 21:24:28   jackmaw
 * SCR(s) 5364 :
 * Re-define CNXT_IIC_READDATA_GET to include a parameter that is a pointer to
 * the correct read data register so this can be used for multiple bus setups.
 * 
 *    Rev 1.8   22 Jan 2003 15:41:58   velusws
 * SCR(s) 5284 :
 * removed hw timer callback. 
 * 
 *    Rev 1.7   17 Dec 2002 16:08:04   velusws
 * SCR(s) 5067 :
 * Removed // comments, moved $Log: 
 * Removed // comments, moved  10   mpeg      1.9         1/30/03 9:24:28 PM   
 * Removed // comments, moved          Billy Jackman   SCR(s) 5364 :
 * Removed // comments, moved        Re-define CNXT_IIC_READDATA_GET to include
 * Removed // comments, moved         a parameter that is a pointer to
 * Removed // comments, moved        the correct read data register so this can
 * Removed // comments, moved         be used for multiple bus setups.
 * Removed // comments, moved        
 * Removed // comments, moved  9    mpeg      1.8         1/22/03 3:41:58 PM   
 * Removed // comments, moved          Senthil Veluswamy SCR(s) 5284 :
 * Removed // comments, moved        removed hw timer callback. 
 * Removed // comments, moved        
 * Removed // comments, moved  8    mpeg      1.7         12/17/02 4:08:04 PM  
 * Removed // comments, moved          Senthil Veluswamy SCR(s) 5067 :
 * Removed // comments, moved        Removed // comments, moved $Log:$ to end 
 * Removed // comments, moved        of file, moved IIC macros from chip header
 * Removed // comments, moved         to here, modifications to support 
 * Removed // comments, moved        multiple IIC banks (Brazos and after)
 * Removed // comments, moved        
 * Removed // comments, moved  7    mpeg      1.6         8/30/02 7:45:10 PM   
 * Removed // comments, moved          Senthil Veluswamy SCR(s) 4502 :
 * Removed // comments, moved        Modifications for using the new IIC 
 * Removed // comments, moved        interface
 * Removed // comments, moved        
 * Removed // comments, moved  6    mpeg      1.5         8/11/99 6:31:40 PM   
 * Removed // comments, moved          Dave Wilson     Changed KAL calls to use
 * Removed // comments, moved         new API.
 * Removed // comments, moved        
 * Removed // comments, moved  5    mpeg      1.4         11/16/98 4:09:58 PM  
 * Removed // comments, moved          Steve Glennon   Increased timeout for 
 * Removed // comments, moved        final hardware to 100000. Have seen some 
 * Removed // comments, moved        flakiness with
 * Removed // comments, moved        smaller timeouts. Further debug needed at 
 * Removed // comments, moved        timeout of 10000 to see why 
 * Removed // comments, moved        IICDeath is occurring, reporting timeout.
 * Removed // comments, moved        
 * Removed // comments, moved  4    mpeg      1.3         11/14/98 1:58:34 PM  
 * Removed // comments, moved          Steve Glennon   Removed 400KHz mode from
 * Removed // comments, moved         final hardware version. Increased final 
 * Removed // comments, moved        hardware 
 * Removed // comments, moved        timeout to 10000 (10ms). LED drivers do 
 * Removed // comments, moved        not work at 400KHz.
 * Removed // comments, moved        
 * Removed // comments, moved  3    mpeg      1.2         11/13/98 4:38:32 PM  
 * Removed // comments, moved          Rob Tilton      Spend up the final hw to
 * Removed // comments, moved         400KHz.
 * Removed // comments, moved        
 * Removed // comments, moved  2    mpeg      1.1         10/6/98 6:34:18 PM   
 * Removed // comments, moved          Steve Glennon   Made dprintf into a 
 * Removed // comments, moved        function which calls trace_new with 
 * Removed // comments, moved        trace_level_1 (info)
 * Removed // comments, moved        Hence removed #define of dprintf from 
 * Removed // comments, moved        include file
 * Removed // comments, moved        
 * Removed // comments, moved  1    mpeg      1.0         7/30/98 4:13:40 PM   
 * Removed // comments, moved          Rob Tilton      
 * Removed // comments, moved $ to end of file, moved IIC macros from chip header to here, modifications to support multiple IIC banks (Brazos and after)
 * 
 *    Rev 1.6   30 Aug 2002 18:45:10   velusws
 * SCR(s) 4502 :
 * Modifications for using the new IIC interface
 * 
 *    Rev 1.5   11 Aug 1999 17:31:40   dawilson
 * Changed KAL calls to use new API.
 * 
 *    Rev 1.4   16 Nov 1998 16:09:58   glennon
 * Increased timeout for final hardware to 100000. Have seen some flakiness with
 * smaller timeouts. Further debug needed at timeout of 10000 to see why 
 * IICDeath is occurring, reporting timeout.
 *
 *    Rev 1.3   14 Nov 1998 13:58:34   glennon
 * Removed 400KHz mode from final hardware version. Increased final hardware
 * timeout to 10000 (10ms). LED drivers do not work at 400KHz.
 *
 *    Rev 1.2   13 Nov 1998 16:38:32   rtilton
 * Spend up the final hw to 400KHz.
 *
 *    Rev 1.1   06 Oct 1998 17:34:18   glennon
 * Made dprintf into a function which calls trace_new with trace_level_1 (info)
 * Hence removed #define of dprintf from include file
 *
 *    Rev 1.0   30 Jul 1998 15:13:40   rtilton
 * Initial revision.
 *
*****************************************************************************/
