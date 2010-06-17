/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2003                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       fpleds.c
 *
 *
 * Description:    Front panel LED and LCD matrix driver
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Id: fpleds.c,v 1.23, 2003-11-14 13:02:13Z, Ganesh Banghar$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#include "basetype.h"
#include "globals.h"
#include "iic.h"
#include "kal.h"
#include "fpleds.h"
#include "retcodes.h"
#include "lcdmatrix.h"

/********************/
/* Global Variables */
/********************/
static bool bInitialised = FALSE;
static char *pszRtosArray[] = {
    "NOOS",
    "PSOS",
    "VXWK",
    "NUP ",
    "UCOS",
    "UCOS2"
}; /* note that if the RTOS config changes, change this as well */

static char *pszToolkitArray[] = {
    "      ",
    "SDT   ",
    "ADS   ",
    "WRGCC ",
    "GNUGCC"
}; /* note that if the ARM_TOOLKIT config changes, change this as well */

/* length of internal workspace for displaying numbers */
#define MAX_LED_STRING    10

extern u_int32 ArmClkFreq32, MemClkFreq32;
extern int RAMSize, RAMWidth;
extern unsigned long ChipID, ChipRevID;

/*******************/
/* Local Functions */
/*******************/
static bool MatrixClearDisplay(void);
static bool MatrixStringAt(char *string, int x, int y);

/******************************************************************************/
/* Function Name: LEDInit                                                     */
/*                                                                            */
/* Description  : Determine which board we are running on and, hence, the     */
/*                type of display that is supported (4 digit or 20x4 array)   */
/*                                                                            */
/* Parameters   : None                                                        */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/******************************************************************************/
bool LEDInit(void)
{
  bool bRetcode = FALSE;

  trace_new(TRACE_GEN|TRACE_LEVEL_3, "Initialising LED display driver\n");
    
  /* First look for the 20x4 matrix display (display of choice) */

  #if (I2C_BUS_LCD_MATRIX != I2C_BUS_NONE)
  bRetcode = iicAddressTest(I2C_ADDR_LCD_MATRIX, I2C_BUS_LCD_MATRIX, FALSE);
  #else
  bRetcode = FALSE;
  #endif
  
  if(bRetcode)
  {
    trace_new(TRACE_GEN|TRACE_LEVEL_3,"Found 20x4 character matrix display\n");
    bRetcode = TRUE;
  }  
  else
  {
    trace_new(TRACE_GEN|TRACE_LEVEL_3,"ERROR - No LCD display found!\n");
    error_log(MOD_GEN|ERROR_WARNING);
    bRetcode = FALSE;
  }
  
  bInitialised = bRetcode;
  
  if(bRetcode)
    LEDClearDisplay();
  
  return(bRetcode);  
}

/******************************************************************************/
/* Function Name: LEDIsMatrixDisplay                                          */
/*                                                                            */
/* Description  : Tell the caller whether or not this board is equipped with  */
/*                a 20x4 LCD matrix display.                                  */
/*                                                                            */
/* Parameters   : None                                                        */
/*                                                                            */
/* Returns      : TRUE id matrix is available, FALSE otherwise                */
/*                                                                            */
/******************************************************************************/
bool LEDIsMatrixDisplay(void)
{
  if(!bInitialised)
    LEDInit();
  
    return(TRUE);
}

/******************************************************************************/
/* Function Name: LEDClearDisplay                                             */
/*                                                                            */
/* Description  : Clear the display on whichever device we are using          */
/*                                                                            */
/* Parameters   : None                                                        */
/*                                                                            */
/* Returns      : Nothing                                                     */
/*                                                                            */
/******************************************************************************/
void LEDClearDisplay(void)
{
    MatrixClearDisplay();
}

/******************************************************************************/
/* Function Name: LEDGetDisplaySize                                           */
/*                                                                            */
/* Description  : Return the dimensions of the LED screen in characters       */
/*                                                                            */
/* Parameters   : width  - pointer to storage for screen width                */
/*                height - pointer to storage for screen height               */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/******************************************************************************/
bool LEDGetDisplaySize(int *width, int *height)
{
    /*
     * Initialise if not done already
     */
    if(!bInitialised)
    {
        LEDInit();
    }
  
    /*
     * Check for valid pointers
     */
    if(!width || !height)
    {
        return(FALSE);
    }

    /*
     * Return the appropriate dimensions  
     */
    *width  = 20;
    *height = 4;

    return(TRUE);
}

/******************************************************************************/
/* Function Name: LEDLongString                                               */
/*                                                                            */
/* Description  : Display a string longer than 4 characters on the LEDs (for  */
/*                20x4 display only). String will be displayed at top left    */
/*                and word-wrapped as required.                               */
/*                                                                            */
/* Parameters   : string - pointer to string to display                       */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/******************************************************************************/
bool LEDLongString(char *string)
{
  if(!bInitialised)
    LEDInit();
  
  return(LEDStringAt(string, 0, 0));  
}

/******************************************************************************/
/* Function Name: LEDLongIntAt                                                */
/*                                                                            */
/* Description  : Display an integer in hex or decimal format on the matrix   */
/*                display at the coordinates provided.                        */
/*                                                                            */
/* Parameters   : number - number to display                                  */
/*                hex    - TRUE for hexadecimal format, FALSE for decimal     */
/*                x      - starting column for displayed string               */
/*                y      - starting row for displayed string                  */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/******************************************************************************/
#if RTOS != NOOS
bool LEDLongIntAt(int number, bool hex, int x, int y)
{
  char string[12];
  
  if(!bInitialised)
    LEDInit();
  
  if(hex)
    sprintf(string, "0x%x", number);
  else
    sprintf(string, "%d", number);
    
  return(LEDStringAt(string, x, y));  
}
#endif

/******************************************************************************/
/* Function Name: LEDStringAt                                                 */
/*                                                                            */
/* Description  : Display a string at a given (x,y) position on the screen    */
/*                (20x4 display only). String will be word-wrapped as         */
/*                required.                                                   */
/*                                                                            */
/* Parameters   : string - pointer to string to display                       */
/*                x      - column in which to start displaying string         */
/*                y      - row in which to start displaying string            */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/******************************************************************************/
bool LEDStringAt(char *string, int x, int y)
{
  if(!bInitialised)
    LEDInit();
  
  return(MatrixStringAt(string, x, y));
    
}

/******************************************************************************/
/* Function Name: LEDSegs                                                     */
/*                                                                            */
/* Description  : Sets the LEDS to the segment values specified               */
/*                                                                            */
/* Parameters   : b0-b3    bytes to put in LED Segment registers              */
/*                         b0 in leftmost, b3 in rightmost                    */
/*                                                                            */
/* Returns      : Nothing                                                     */
/*                                                                            */
/******************************************************************************/
void LEDSegs(BYTE b0, BYTE b1, BYTE b2, BYTE b3)
{
}

/******************************************************************************/
/* Function Name: LEDString                                                   */
/*                                                                            */
/* Description  : Displays the passed string in the LED display               */
/*                Currently limited to displaying the first 4 chars           */
/*                Could be enhanced to scroll through the string > 4 chars    */
/*                For < 4 chars, left align, pad with blank                   */
/*                                                                            */
/* Parameters   : pszString    Pointer to string of which first 4 chars       */
/*                             are displayed to best of ability               */
/*                                                                            */
/* Returns      : Nothing                                                     */
/*                                                                            */
/******************************************************************************/
void LEDString(char *pszString)
{
   if(!bInitialised)
      LEDInit();
   
   /************************/
   /* Check string present */  
   /************************/
   if (pszString)
   {
      /***************************************************************/
      /* If we have a large screen attached, direct the output there */
      /***************************************************************/
      LEDStringAt(pszString, 0, 0);
      return;
   }
      
} /* LEDString */

#if RTOS != NOOS
/******************************************************************************/
/* Function Name: LEDVal                                                      */
/*                                                                            */
/* Description  : Displays and integer in decimal or Hex on LEDS, from 0-9999 */
/*                or 0-FFFF                                                   */
/*                                                                            */
/* Parameters   : i       Value to display. If > limit, displays limit.       */
/*                digits  Number of digits to display.                        */
/*                        If 0, just display number                           */
/*                        If >0, displays number with preceding 0's           */
/*                        example: LEDInt(45,0) displays '  45'               */
/*                        example: LEDInt(45,3) displays ' 045'               */
/*                        example: LEDInt(45,4) displays '0045'               */
/*                bHex    TRUE for hex display, FALSE for decimal                                                            */
/*                                                                            */
/* Returns      : Nothing                                                     */
/*                                                                            */
/******************************************************************************/
void LEDVal(unsigned int i, int digits, int bHex)
{
   char string[MAX_LED_STRING];
   int  shift;
   
   if(!bInitialised)
     LEDInit();
   
   /******************************************/
   /* Make sure no overflows beyond 4 digits */   
   /******************************************/
   if (bHex)
   {
      if (i > 0xFFFF) {
         i = 0xFFFF;
      }
   } else {
      if (i > 9999) {
         i = 9999;
      }
   } /* endif */
   
   /*********************************************/
   /* Make sure no more than 4 digits specified */      
   /*********************************************/
   if (digits > 4) {
      digits=4;
   }
   
   /******************************************************************/
   /* Format the number with the requisite quantity of leading zeros */   
   /******************************************************************/
   if (bHex)
   {
      /*****************/
      /* Format in Hex */   
      /*****************/
      switch (digits)
      {
         case 0:  sprintf(string,"%X",i);      break;
         case 1:  sprintf(string,"%1.1X",i);   break;
         case 2:  sprintf(string,"%2.2X",i);   break;
         case 3:  sprintf(string,"%3.3X",i);   break;
         case 4:  sprintf(string,"%4.4X",i);   break;
      
      } 
   } else {
      /*********************/
      /* Format in Decimal */   
      /*********************/
      switch (digits)
      {
         case 0:  sprintf(string,"%d",i);      break;
         case 1:  sprintf(string,"%1.1d",i);   break;
         case 2:  sprintf(string,"%2.2d",i);   break;
         case 3:  sprintf(string,"%3.3d",i);   break;
         case 4:  sprintf(string,"%4.4d",i);   break;
      
      } 
   } /* endif hex or decimal */
   
   
   /************************************************************************/
   /* Right justify the result - calculate number of places to shift right */   
   /************************************************************************/
   shift = 3;           /* if only one character, right shift by 3 places */
   if (string[1])
   {
      shift--;          /* at least 2 chars, right shift by 2 places */
      if (string[2])
      {
         shift--;       /* at least 3 chars, shift right by 1 place */
         if (string[3])
         {
            shift--;    /* 4 chars, shift should now be 0 */
         } 
      } 
   } 
   switch (shift)
   {
      case 0: break;
      case 1: 
         string[3] = string[2];
         string[2] = string[1];
         string[1] = string[0];
         string[0] = ' ';
         break;
      case 2:
         string[3] = string[1];
         string[2] = string[0];
         string[1] = ' ';
         string[0] = ' ';
         break;
      case 3:
         string[3] = string[0];
         string[2] = ' ';
         string[1] = ' ';
         string[0] = ' ';
         break;
   } /* endswitch */
   
   LEDString(string);
   
   return;
} /* LEDVal */
#endif /* OS != NOOS */

/***********************************************************/
/* Functions Specific to the 20x4 character matrix display */
/***********************************************************/

/******************************************************************************/
/* Function Name: MatrixClearDisplay                                          */
/*                                                                            */
/* Description  : Clear the display of the LCD matrix device                  */
/*                                                                            */
/* Parameters   : None                                                        */
/*                                                                            */
/* Returns      : Nothing                                                     */
/*                                                                            */
/******************************************************************************/
static bool MatrixClearDisplay(void)
{
   #if I2C_BUS_LCD_MATRIX != I2C_BUS_NONE
   
   IICTRANS iicTransBuf;
   BYTE byData[3];
   BYTE byCmd[4];
   bool bRetcode;
    
   // Clear the screen
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_CLEAR_DISPLAY;  // Command code

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);

   // Move the cursor back to the top left
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_GOTO_HOME;      // Command code

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
   
   // Turn the cursor off
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_CURSOR_OFF;     // Command code

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
   
   // Turn blink off
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_BLINK_OFF;      // Command code

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
   
   // Turn word wrapping on 
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_AUTO_WRAP_ON;   // Command code

   iicTransBuf.dwCount = 4;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
   
   return(bRetcode);
   #else
   return(FALSE);
   #endif /* I2C_BUS_LCD_MATRIX != I2C_BUS_NONE */
}

/******************************************************************************/
/* Function Name: MatrixStringAt                                              */
/*                                                                            */
/* Description  : Display a string at a given (x,y) position on the screen    */
/*                (20x4 display only). String will be word-wrapped as         */
/*                required.                                                   */
/*                                                                            */
/* Parameters   : string - pointer to string to display                       */
/*                x      - column in which to start displaying string         */
/*                y      - row in which to start displaying string            */
/*                                                                            */
/* Returns      : TRUE on success, FALSE on failure                           */
/*                                                                            */
/* Notes        : The device expects column and row numbers staring at 1. To  */
/*                maintain a 0-based coordinate system as for the screen,     */
/*                function expects 0-based x and y values and adds 1 to       */
/*                convert to the 1-based system used by the LCD display.      */
/*                                                                            */
/******************************************************************************/
#define MAX_CHARS_PER_BUFFER 30
static bool MatrixStringAt(char *string, int x, int y)
{
   #if I2C_BUS_LCD_MATRIX != I2C_BUS_NONE
   
   IICTRANS iicTransBuf;
   BYTE byData[MAX_CHARS_PER_BUFFER+1];
   BYTE byCmd[MAX_CHARS_PER_BUFFER+2];
   char *pCurrent;
   bool bFinished = FALSE;
   bool bRetcode;
   int  iNumChars;

   if ((x > 19) || (x < 0) || (y > 3) || (y < 0))
   {
     trace_new(TRACE_GEN|TRACE_LEVEL_3, "Illegal coordinates passed to MatrixStringAt (%d, %d)\n", x, y);
     return(FALSE);
   }  
   
   // Convert to 1-based coordinates
   x++;
   y++;
   
   // Move cursor to the requested position
   byCmd[0] = IIC_START | IIC_NO_DEATH;
   byCmd[1] = IIC_DATA;
   byCmd[2] = IIC_DATA;
   byCmd[3] = IIC_DATA;
   byCmd[4] = IIC_DATA;
   byCmd[5] = IIC_STOP;

   byData[0] = I2C_ADDR_LCD_MATRIX; // Address
   byData[1] = LCD_COMMAND_PREFIX; // Command prefix
   byData[2] = LCD_GOTO_XY;        // Command code
   byData[3] = (char)x;
   byData[4] = (char)y;

   iicTransBuf.dwCount = 6;
   iicTransBuf.pData = byData;
   iicTransBuf.pCmd = byCmd;

   bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
   
   /* Start dumping the string in sections */
   pCurrent = string;
   while (!bFinished)
   {
     for (iNumChars = 0; (*pCurrent != (char)0) && (iNumChars < MAX_CHARS_PER_BUFFER); iNumChars++)
     {
       byCmd[iNumChars+1]  = IIC_DATA;
       byData[iNumChars+1] = (BYTE)*pCurrent;
       pCurrent++;
     }  
   
     /* Do we have any characters to send? */
     if (iNumChars != 0)
     {
       byCmd[iNumChars+1] = IIC_STOP;
       iicTransBuf.dwCount = iNumChars+2;
       bRetcode = iicTransaction(&iicTransBuf, I2C_BUS_LCD_MATRIX);
     }  
     
     if(*pCurrent == (char)0)
      bFinished = TRUE;
   }

   return(bRetcode);
   #else
   return(FALSE);
   #endif /* I2C_BUS_LCD_MATRIX != I2C_BUS_NONE */
   
}

/*********************************************************************/
/*  LEDDisplayApplication ()                                         */
/*                                                                   */
/*  PARAMETERS: char *pszAppName    Ptr to Application name string   */
/*                                                                   */
/*  DESCRIPTION: This function call the init function, then          */
/*               displays the app name and other information         */
/*               on the front panel LED.                             */
/*                                                                   */
/*               For instance, if pszAppName is "AppName", built     */
/*               with PSOS as debug with version "VersionString",    */
/*               then display would look similar to:                 */
/*                                                                   */
/*                     01234567890123456789                          */
/*                    +--------------------+                         */
/*                   0\AppName   PSOS DBG  \                         */
/*                   1\HwConfig   SwConfig \                         */
/*                   2\VersionString       \                         */
/*                   3\Date    Time  Freq  \                         */
/*                    +--------------------+                         */
/*                                                                   */
/*  RETURNS: CNXT_LED_OK if success                                  */
/*********************************************************************/
CNXT_LED_STATUS LEDDisplayApplication(char *pszAppName)
{
    extern char *szBuildVersion;
    extern char *szBuildDate;
    extern char *szBuildTime;
    extern char *szConfig;
    extern char *szSWConfig;
    bool bRetcode;
    int iDispWidth, iDispHeight;
    char cShortVersion[24];       /* For temp storing short version string */
    char szModBuildDate[16];
    char szModBuildTime[16];
    char szArmClkFreq[8];
    char szDbg[4];

  #ifdef DEBUG
    strcpy(szDbg, "DBG");
  #else
    strcpy(szDbg, "REL");
  #endif

    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ Application:       %s\n", pszAppName);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ Env/RTOS/Dbg:      %s %s %s\n",
        pszToolkitArray[ARM_TOOLKIT], pszRtosArray[RTOS], szDbg);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ HwConfig/SwConfig: %s %s\n", szConfig, szSWConfig);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ VersionString:     %s\n", szBuildVersion);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ Date/Time:         %s %s\n", szBuildDate, szBuildTime);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ ARM/MEM Freq:      %dMhz %dMhz\n",
        (ArmClkFreq32/1000000)+1, (MemClkFreq32/1000000)+1);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ RAM Size/Width:    %dMB %dbits\n",
        (RAMSize/1024/1024), RAMWidth);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "+ Chip ID/Rev:       0x%X rev %d\n", ChipID, ChipRevID);
    trace_new(TRACE_GEN|TRACE_LEVEL_ALWAYS,
        "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    bRetcode = LEDGetDisplaySize(&iDispWidth, &iDispHeight);
    if (bRetcode && (iDispHeight == 4))
    {
      LEDClearDisplay();
      LEDStringAt(pszAppName, 0, 0);             /* Line 0, col 00: app name */
      LEDStringAt(pszRtosArray[RTOS], 10, 0);    /* Line 0, col 10: RTOS */
      LEDStringAt(szDbg, 15, 0);                 /* Line 0, col 15: Dbg/Rel */

      LEDStringAt(szConfig, 0, 1);               /* Line 1, col 00: HW config */
      LEDStringAt("          ", 10, 1);          /* clear rest of line */
      LEDStringAt(szSWConfig, 11, 1);            /* Line 1, col 11: SW config */

      /* get only 20 chars from version and make sure it's null terminated */
      strncpy(cShortVersion, szBuildVersion, 20);
      cShortVersion[20] = 0;
      LEDStringAt(cShortVersion, 0, 2);          /* Line 2, col 00: Version */

      strncpy(szModBuildDate, szBuildDate, 3);
      strncpy(&(szModBuildDate[3]), &(szBuildDate[4]), 2);
      szModBuildDate[5] = '\'';
      strncpy(&(szModBuildDate[6]), &(szBuildDate[9]), 2);
      szModBuildDate[8] = 0;
      LEDStringAt(szModBuildDate, 0, 3);            /* Line 3, col 00: Bld Date */

      strncpy(szModBuildTime, szBuildTime, 5);
      szModBuildTime[5] = 0;
      LEDStringAt(szModBuildTime, 9, 3);           /* Line 3, col 9: Bld Time */

      sprintf(szArmClkFreq, "%ldMz", ((ArmClkFreq32/1000000)+1));
      LEDStringAt(szArmClkFreq, 15, 3);           /* Line 3, col 15: Arm Clk Freq */
    }
    
    return(CNXT_LED_OK);
} /* LEDDisplayApplication() */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  24   mpeg      1.23        11/14/03 7:02:13 AM    Ganesh Banghar  CR(s): 
 *        7926 Added String UCOS2 so UCOS2 can be displayed on the front Panel 
 *        when building RTOS=UCOS2.
 *  23   mpeg      1.22        10/30/03 4:20:43 PM    Tim White       CR(s): 
 *        7756 7757 Add the display of the ARM clock frequency to the LCD 
 *        panel.  Also add all the LCD
 *        information (plus some extra) to the serial trace output log.
 *        
 *  22   mpeg      1.21        9/2/03 7:11:46 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        reordered header files to eliminate warnings when building for PSOS
 *        
 *  21   mpeg      1.20        5/14/03 2:22:26 PM     Matt Korte      SCR(s) 
 *        6325 6326 :
 *        Fixed FP display and confmgr init issues
 *        
 *  20   mpeg      1.19        5/5/03 4:15:44 PM      Tim White       SCR(s) 
 *        6172 :
 *        Removed 7 segment LED support.
 *        
 *        
 *  19   mpeg      1.18        4/11/03 10:15:16 AM    Matt Korte      SCR(s) 
 *        5998 :
 *        Fix bug that caused long HW config names to mess up the SW config
 *        name. Also moved HW, SW config names into the <rtos>kal.c file wher
 *        the other bldver.h definitions were, so that this file is not rebuilt
 *        everytime.
 *        
 *        
 *  18   mpeg      1.17        3/20/03 4:12:46 PM     Matt Korte      SCR(s) 
 *        5837 :
 *        Add display routine
 *        
 *  17   mpeg      1.16        2/13/03 11:49:36 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  16   mpeg      1.15        1/24/03 9:53:38 AM     Dave Moore      SCR(s) 
 *        5305 :
 *        removed FREQ_SCREEN
 *        
 *        
 *  15   mpeg      1.14        1/22/03 1:57:28 PM     Dave Wilson     SCR(s) 
 *        5099 :
 *        Reworked to ensure that the code would not try to access the display 
 *        device
 *        if the config file explicitly defines it as unpopulated.
 *        
 *  14   mpeg      1.13        11/26/02 4:02:00 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed to use label from config file to define which I2C bus device 
 *        is on.
 *        
 *  13   mpeg      1.12        9/3/02 7:58:58 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  12   mpeg      1.11        5/11/00 11:53:32 AM    Dave Wilson     Removed a
 *         "to be completed" comment that was redundant
 *        
 *  11   mpeg      1.10        4/3/00 4:46:00 PM      QA - Roger Taylor 
 *        Eliminated recursive call to LED_init.
 *        
 *  10   mpeg      1.9         3/28/00 1:41:30 PM     Dave Wilson     Fixed bug
 *         in LEDLongIntAt which was previously displaying at (0,0) regardless
 *        
 *  9    mpeg      1.8         3/28/00 12:26:50 PM    Dave Wilson     Expanded 
 *        API to support 20x4 matrix display
 *        
 *  8    mpeg      1.7         3/3/00 1:35:40 PM      Tim Ross        
 *        Eliminated code that was OS dependent or inefficient for inclusion
 *        in a boot block using the RTOS == NOOS switch.
 *        
 *  7    mpeg      1.6         11/2/99 10:16:26 AM    Chris Chapman   Changed 
 *        code to use new variation of FREQ_SCREEN def.
 *        
 *  6    mpeg      1.5         10/29/99 5:24:58 PM    Dave Wilson     Fixed 
 *        header files to ensure that code will build under pSOS and VxWorks.
 *        
 *  5    mpeg      1.4         10/14/99 7:14:02 PM    Dave Wilson     Removed 
 *        dependence on OpenTV headers.
 *        
 *  4    mpeg      1.3         10/11/99 5:47:44 PM    Chris Chapman   Disabled 
 *        all routines in case of FREQ_SCREEN
 *        
 *  3    mpeg      1.2         9/23/99 12:25:12 PM    Dave Wilson     Fixed a 
 *        compiler warning.
 *        
 *  2    mpeg      1.1         10/20/98 5:34:22 PM    Steve Glennon   Fixed up 
 *        and made it work. Combined LEDInt and LEDHex into
 *        a single function LEDVal.
 *        
 *  1    mpeg      1.0         10/19/98 4:58:18 PM    Steve Glennon   
 * $
 *****************************************************************************/
