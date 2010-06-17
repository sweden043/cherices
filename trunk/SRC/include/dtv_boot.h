/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       dtv_boot.h
 *
 *
 * Description:    Function Prototypes for dtv_boot.c
 *
 * Author:         Dave Moore
 *
 ****************************************************************************/
/* $Header: dtv_boot.h, 4, 5/2/03 12:05:10 PM, Dave Moore$
 ****************************************************************************/

#define NO_STATE 0
#define BOOT_OBJ_ACQUIRED 1
#define MARKER_OBJ_ACQUIRED 2
#define TIMEOUT_MARKER_OBJ 4 
#define TIMEOUT_BOOT_OBJ 8 
#define UPDATE_LIST_OBJ_ACQUIRED 16
#define ALL_CHANNELS_ACQUIRED 32

#define WEST_119 3
#define WEST_101 0

genfilter_mode ChannelObjNotify( u_int8 *pChannelObj, u_int32 uObjLength );
genfilter_mode MarkerObjNotify( u_int8 *pMarkerObj, u_int32 uObjLength );
genfilter_mode UpdateListObjNotify( u_int8 *pUpdateListObj, u_int32 uObjLength );
genfilter_mode BootObjNotify( u_int8 *pBootObj, u_int32 uObjLength );
void DtvBoot( void );
char* map_servicetype_to_string( u_int8 uServiceType );
char* map_streamtype_to_string( u_int8 uStreamType );

/* Code to do strncpy plus remove 'special' characters */
#define StrnCpyRemoveUndefinedCharacters(desStr, souStr,length,count)       \
{                                                                           \
           for (count=0;count<length;count++)                               \
           {                                                                \
               if ( (souStr[count] <= 0x1F) || (souStr[count] >= 0x7F) )    \
               {  desStr[count] = 0x20; } /* Convert to space character */  \
               else { desStr[count] = souStr[count]; }                      \
           }                                                                \
           desStr[count] = '\0';                                            \
}      

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         5/2/03 12:05:10 PM     Dave Moore      SCR(s) 
 *        5972 :
 *        Changes to support integration with WatchTV
 *        
 *        
 *  3    mpeg      1.2         3/28/03 12:16:34 PM    Angela Swartz   SCR(s) 
 *        5826 :
 *        moved some #defines from dtv_boot.c here, so si.c can use it
 *        
 *  2    mpeg      1.1         3/19/03 9:42:08 AM     Dave Moore      SCR(s) 
 *        5805 :
 *        changed most function returns to genfilter_mode
 *        
 *        
 *  1    mpeg      1.0         3/18/03 4:04:52 PM     Dave Moore      
 * $
 * 
 *    Rev 1.3   02 May 2003 11:05:10   mooreda
 * SCR(s) 5972 :
 * Changes to support integration with WatchTV
 * 
 * 
 *    Rev 1.2   28 Mar 2003 12:16:34   swartzwg
 * SCR(s) 5826 :
 * moved some #defines from dtv_boot.c here, so si.c can use it
 * 
 *    Rev 1.1   19 Mar 2003 09:42:08   mooreda
 * SCR(s) 5805 :
 * changed most function returns to genfilter_mode
 * 
 * 
 *    Rev 1.0   18 Mar 2003 16:04:52   mooreda
 * SCR(s) 5805 :
 * Header File for DIRECTV dtv_boot.c Boot Code
 * 
 *
 ****************************************************************************/

