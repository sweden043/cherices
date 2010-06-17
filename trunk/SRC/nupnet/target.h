/***************************************************************************
*
*      Copyright (c) 1993 - 2001 by Accelerated Technology, Inc.
*
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject
* matter of this material.  All manufacturing, reproduction, use and sales
* rights pertaining to this subject matter are governed by the license
* agreement.  The recipient of this software implicity accepts the terms
* of the license.
*
****************************************************************************
****************************************************************************
*
* FILENAME
*
*       TARGET.H
*
* DESCRIPTION
*
*       This file will hold all of those defines and setups used by the
*       TCP/IP code which are processor/configuration dependent.
*
* DATA STRUCTURES
*
*  None.
*
* FUNCTIONS
*
*  None.
*
* DEPENDENCIES
*
*  None.
*
****************************************************************************/
#ifndef TARGET_H
#define TARGET_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nucleus.h"
#include "isnmp.h"

/* These definitions control which version of NET the build should be
   compatible with. This should allow new versions of NET to be shipped but
   remain compatible with applications or drivers designed for previous
   versions. */
#define     NET_4_0         1       /* NET 4.0 */
#define     NET_4_2         2       /* NET 4.2 */
#define     NET_4_3         3       /* NET 4.3 */
#define     NET_4_4         4       /* NET 4.4 */

#define NET_VERSION_COMP    NET_4_4        /* The version for which compatibilty
                                              is desired. */



/********************* Nucleus NET Porting Section **************************/

/* This is the number of Nucleus PLUS timer ticks that occur per second. */
/* DAVEM #ifndef NU_PLUS_Ticks_Per_Second
#define TICKS_PER_SECOND        ((UINT32)100)
#else
#define TICKS_PER_SECOND        ((UINT32)NU_PLUS_Ticks_Per_Second)
#endif
*/

#define TICKS_PER_SECOND        ((UINT32)500) /* DAVEM System timer is set for 2ms ticks in NUP_INIC.C */

/*
*  Setup the current limits and size type defs for the defined processor.
*  This will define constants for the sizes of integral types, with
*  minimums and maximum values allowed for the define.
*  Note: mins and maxs will be different for different processors.
*                               Size (Bytes)    Alignment (Bytes)
*/

#ifndef PLUS_VERSION_COMP

typedef char           INT8;    /*  1                   1   */
typedef unsigned char  UINT8;   /*  1                   1   */
typedef signed short   INT16;   /*  2                   2   */
typedef unsigned short UINT16;  /*  2                   2   */
typedef signed long    INT32;   /*  4                   4   */
typedef unsigned long  UINT32;  /*  4                   4   */

#endif /* PLUS_VERSION_COMP */

/* These macroes are used for the versions of Nucleus NET that run in real-mode
   on the PC.  For all architectures other than rel-mode PC these should be
   defined to be nothing.
*/
#undef   FAR
#undef   HUGE
#define  FAR
#define  HUGE

/* The following definitions specify which routines have been implemented
 * in assembly language. A C implementation of each can be found in the
 * file TOOLS.C. An assembly implementation of tcpcheck may especially
 * increase performance. A NU_TRUE indicates an assembly implementation exists.
 */
#define IPCHECK_ASM     	NU_TRUE
#define BUFCHECK_ASM     NU_TRUE
#define TCPCHECK_ASM    	NU_TRUE
#define LONGSWAP_ASM    NU_FALSE
#define INTSWAP_ASM     	NU_FALSE
#define COMPAREN_ASM    NU_FALSE

/*added by sam */
#define PUT32_ASM     NU_FALSE
#define GET32_ASM     NU_FALSE
#define PUT16_ASM    NU_FALSE
#define GET16_ASM    NU_FALSE

/* Define the two macros that point to the NET swapping routines. These
   routines are endian proof. Meaning that no matter what the endianness
   of the target hardware is they will return data in the correct
   byte order. Since no swapping is required on big endian platforms
   these macros could be defined as nothing for a performance improvement.

   ex: #define LONGSWAP

   This will replace the LONGSWAPs in the code with nothing, thus using
   the value of the variable.
*/

#if !(LONGSWAP_ASM)
#define LONGSWAP    TLS_Longswap
#else
extern   unsigned int  TLS_Longswap_ASM(unsigned int number);
#define LONGSWAP    TLS_Longswap_ASM
#endif

#if !(INTSWAP_ASM)
#define INTSWAP	   TLS_Intswap
#else
extern   unsigned short TLS_Intswap_ASM(unsigned short  number);
#define INTSWAP	   TLS_Intswap_ASM
#endif


/* This macro specifies the required byte alignment for Nucleus NET packet
   buffers. Generally this value is dictated by the DMA requirements. For
   Example the DMA may only work to/from addresses on 8 byte boundaries. A
   value less than 4 should not be specified. */
#define REQ_ALIGNMENT           16

/********************* End Nucleus NET Porting Section **********************/

/************************ Protocol Configuration ****************************

    The following macros control which network protocols will be included
    in the build of the Nucleus NET stack. Keep in mind that some protocols
    make use of other protocols, therefore it is not always possible to
    remove certain protocols from the build. One example would be UDP. UDP is
    used by DHCP and some other protocols, so if you want to remove UDP you
    must also remove all the protocols that use UDP. You can look at the
    #error macros below to see which protocols are dependant on other
    protocols.

*/

/* Should support for UDP be included. */
#define INCLUDE_UDP                     NU_TRUE

/* Should support for TCP be included. */
#define INCLUDE_TCP                     NU_TRUE

/* Should support for RAW IP be included. */
//#define INCLUDE_IP_RAW                  NU_FALSE
#define INCLUDE_IP_RAW                  NU_TRUE

/* Should the loopback device be included and initialized in the 
   build of NET. This is NU_TRUE by default. */
#define INCLUDE_LOOPBACK_DEVICE         NU_TRUE

/* This macro controls whether the code to forward IP packets will be included
   in the library.
*/
//#define INCLUDE_IP_FORWARDING           NU_TRUE
#define INCLUDE_IP_FORWARDING           NU_FALSE

/* By default RARP is not included in the Nucleus NET build.  To include RARP
   change the NU_FALSE to a NU_TRUE. See the Nucleus NET reference manual
   for more information on RARP.
*/
#define INCLUDE_RARP                    NU_FALSE

/* By default DNS is included in the Nucleus NET build.  To exclude it change
   the NU_TRUE below to a NU_FALSE.  See the Nucleus NET reference Manual
   for more information on DNS.
*/
#define INCLUDE_DNS NU_TRUE

/* Is this possible? */
#if ((INCLUDE_UDP == NU_FALSE) && (INCLUDE_DNS == NU_TRUE))
    #error UDP must be included in order to use DNS
#endif

/* By default DHCP client is included in the Nucleus NET build.  To exclude
   it change the NU_TRUE below to a NU_FALSE. DHCP validate callback and
   vendor options callback can be enabled in a like manner. See the
   Nucleus NET reference Manual for more information on DHCP.
*/
#define INCLUDE_DHCP                    NU_TRUE
//#define INCLUDE_DHCP                    NU_FALSE
#define DHCP_VALIDATE_CALLBACK          NU_FALSE

/* Is it ok for DHCP? */
#if ((INCLUDE_UDP == NU_FALSE) && (INCLUDE_DHCP == NU_TRUE))
    #error UDP must be included in order to use DHCP
#endif

/* By default BOOTP client is not included in the Nucleus Net Build. To include
   it change the NU_FALSE below to a NU_TRUE.  See the Nucleus Net
   reference Manual for more information on BOOTP.
*/
#define INCLUDE_BOOTP                   NU_FALSE

/* Is this possible? */
#if ((INCLUDE_UDP == NU_FALSE) && (INCLUDE_BOOTP == NU_TRUE))
    #error UDP must be included in order to use BOOTP
#endif

/* By default IP reassembly is included in the Nucleus NET library. Change
   this definition to a NU_FALSE to exclude IP reassembly. */
#define INCLUDE_IP_REASSEMBLY           NU_TRUE

/* By default IP fragmentation is included in the Nucleus NET library. Change
   this definition to a NU_FALSE to exclude IP fragmentation. */
#define INCLUDE_IP_FRAGMENT             NU_TRUE

/* By default IP Multicasting is included in the Nucleus NET library. Change
   this definition to a NU_FALSE to exclude IP Multicasting. */
#define INCLUDE_IP_MULTICASTING         NU_TRUE

/* By default NAT is not included in the Nucleus NET library.  Change this 
   definition to NU_TRUE to include NAT. */
#define INCLUDE_NAT                     NU_FALSE

/* Make sure that the above configuration is ok for protocols
   that make use of UDP. */

/* Is it ok for SNMP? */
#if ((INCLUDE_UDP == NU_FALSE) && (INCLUDE_SNMP == NU_TRUE))
    #error UDP must be included in order to use SNMP
#endif


/******************** End Protocol Configuration ****************************/

/******************* Nucleus NET Stack Tunning Section **********************

    In this section can be found various timeouts and size limitations used
    by the NET stack.

*/

/* The PACKET definition controls how packets are sent.  If PACKET is defined
 * then each packet is transmited as soon as it is ready.  If PACKET is
 * undefined the packets are placed into a transmit queue when they are
 * ready. */
#undef PACKET

/* The PRINT_ERROR_MSG define controls whether a error message is printed to
 * the console when NU_Tcp_Log_Error is called.  The error is logged
 * regardless of PRINT_ERROR_MSG define.  However, an error message will only
 * be printed if PRINT_ERROR_MSG is defined.   */
//#undef PRINT_ERROR_MSG
#define PRINT_ERROR_MSG

#define MAXRTO        ((UINT32)(240 * SCK_Ticks_Per_Second))  /* Maximum retransmit timeout.*/
#define MINRTO        ((UINT32)(SCK_Ticks_Per_Second >> 2))   /* Min. retransmit timeout. */
#define ARPTO         ((UINT32)(1 * SCK_Ticks_Per_Second))    /* ARP retransmit timeout. */
#define CACHETO       ((UINT32)(400 * SCK_Ticks_Per_Second))  /* ARP cache timeout. */
#define WAITTIME      ((UINT32)(2 * SCK_Ticks_Per_Second))    /* Length of time to wait before reusing a port. */

#define ARP_CACHE_LENGTH   10               /* Size of the ARP cache. */
#define CREDIT             4096

#if (INCLUDE_TCP == NU_TRUE)
 #define TCP_MAX_PORTS      30              /* Maximum number of TCP ports.     */
#else
 #define TCP_MAX_PORTS      0
#endif

#if (INCLUDE_UDP == NU_TRUE)
 #define UDP_MAX_PORTS      30              /* Maximum number of UDP ports.     */
#else
 #define UDP_MAX_PORTS      0
#endif

#if (INCLUDE_IP_RAW == NU_TRUE)
 #define IPR_MAX_PORTS      30              /* Maximum number of RAW IP ports.  */
#else
 #define IPR_MAX_PORTS      0
#endif

/* Total number of socket descriptors. This should be
   TCP_MAX_PORTS + UDP_MAX_PORTS + IPR_MAX_PORTS */
#define NSOCKETS        (TCP_MAX_PORTS + UDP_MAX_PORTS + IPR_MAX_PORTS)


#define MAX_BUFFERS     300             /* The max number of internal NET   */
                                        /* buffers. These are used for TX   */
                                        /* and RX of data packets.          */

#define MAX_RETRANSMITS 7               /* The max number of times to       */
                                        /* retransmit a packet before       */
                                        /* giving up.                       */

#define WINDOW_SIZE     16000           /* Size of buffers for TCP in/out   */
                                        /* windows.                         */

#define UMAX_DGRAMS     5               /* Maximum number UDP data grams    */
                                        /* that can be buffered for a       */
                                        /* single port.                     */

#define IMAX_DGRAMS     10              /* Maximum number RAW IP data grams */
                                        /* that can be buffered for a       */
                                        /* single port.                     */


/* NOTE: by default, we are now setup to use DHCP to config Nucleus NET       */
/*       for an IRD. The HOSTNAME, IPADDRESS, etc. will get set automatically */
/*       when using DHCP.                                                     */

/* This is the local host's name.  It can be a maximum of 32 charaters long. */
/* If you don't use DHCP, it's up to you to set this to an unused value..    */
#define HOSTNAME   "sabine10"

/* This is the local host's ip address.  It can be a maximum of 32 charaters long. */
/* If you don't use DHCP, it's up to you to set this to an unused value..    */
#define IPADDRESS "157.152.157.180"

/*   - Austin Design Center IRD Address Pool -

  sabine01 = 157.152.157.191     sabine06 = 157.152.157.196
        02 =            .192           07 =            .197
        03 =            .193           08 =            .198
		04 =            .194           09 =            .199
		05 =            .195           10 =            .180
*/


/* Nucleus NET relies on a couple of tasks to perform its duties. The priority
 * of each is defiend below. */
#define EV_PRIORITY   3   /* The Events Dispather priority. */
#define TM_PRIORITY   3   /* The Timer Task priority. */

/* SWSOVERIDE is the amount of time to wait before overriding the Nagle
   algorithm.  The Nagle algorithm is aimed at preventing the transmission of
   lots of tiny packets.  However, we only want to delay a packet for a short
   period of time.  RFC 1122 recommends a delay of 0.1 to 1.0 seconds.  We
   default to a delay of a 1/4 second. */
#define SWSOVERRIDE       (TICKS_PER_SECOND >> 2)  /* Delay of a 1/4 second */

/*  PROBETIMEOUT  is the delay before a window probe is sent.  */
#define  PROBETIMEOUT     (TICKS_PER_SECOND << 1)    /*  Delay of 2 seconds. */

/* CFG_NETMASK is the mask used by the protocol stack to decide if a node is
 * on the local network.  If a value of 0 is used, then the protocol stack
 * chooses a mask based on the local host's IP address.  If any other value is
 * used, that value will become the network mask.  Allowing the protocol stack
 * to choose the network mask is recommended.
 */
#define CFG_NETMASK  0x0L

/***************** End Nucleus NET Stack Tunning Section ********************/


/* These typedef's are not target specific, but it is useful to have them here. */
typedef struct _RTAB_Route          RTAB_ROUTE;
typedef struct SCK_IP_ADDR_STRUCT   SCK_IP_ADDR;
typedef struct _DV_DEVICE_ENTRY     DV_DEVICE_ENTRY;
typedef struct _DEV_DEVICE          DEV_DEVICE;
typedef struct _DV_REQ              DV_REQ;
typedef struct _IP_MULTI            IP_MULTI;
typedef struct _IP_MULTI_OPTIONS    IP_MULTI_OPTIONS;
typedef struct sock_struct          SOCKET;
typedef struct SCK_SOCKADDR_IP_STRUCT SCK_SOCKADDR_IP;

#if !(GET32_ASM)
#define GET32(bufferP, offset)   \
	TLS_Get32((UINT8 *)bufferP, offset)
#else
extern   unsigned int TLS_Get32_ASM( UINT8 *bufferP, UINT32 offset);
#define GET32(bufferP, offset)   \
	TLS_Get32_ASM((UINT8 *)bufferP, offset)
#endif

#if !(PUT32_ASM)
#define PUT32(bufferP, offset, value)   \
	TLS_Put32((UINT8 *)bufferP, offset, (value))
#else
extern   void TLS_Put32_ASM(UINT8 *bufferP, UINT32 offset,UINT32 value);
#define PUT32(bufferP, offset, value)   \
	TLS_Put32_ASM((UINT8 *)bufferP, offset, value)
#endif

#if !(GET16_ASM)
#define GET16(bufferP, offset)    \
	TLS_Get16((UINT8 *)bufferP, offset)
#else
extern   unsigned int TLS_Get16_ASM(UINT8 *bufferP, UINT32 offset);
#define GET16(bufferP, offset)    \
	TLS_Get16_ASM((UINT8 *)bufferP, offset)
#endif

#if !(PUT16_ASM)
#define PUT16(bufferP, offset, value)   \
	TLS_Put16((UINT8 *)bufferP, offset, value)
#else
extern   void TLS_Put16_ASM(UINT8 *bufferP, UINT32 offset,UINT16 value);
#define PUT16(bufferP, offset, value)   \
	TLS_Put16_ASM((UINT8 *)bufferP, offset, value)
#endif

#define GET8(bufferP, offset)    (((UINT8 *)(bufferP))[offset])

#define PUT8(bufferP, offset, value)    (((unsigned char *)(bufferP))[offset]) = (value)

#define PUT_STRING(dest, offset, src, size) \
   TLS_Put_String((unsigned char *)(dest), (offset), \
                    (unsigned char *)(src), (size))

#define GET_STRING(src, offset, dest, size) \
   TLS_Get_String((unsigned char *)(src), (offset), \
                    (unsigned char *)(dest), (size))

#define EQ_STRING(packet, offset, local, size) \
   TLS_Eq_String((unsigned char *)(packet), (offset), \
                   (unsigned char *)(local), (size))

/* Map the 'C' library macros used by Nucleus NET and other networking
   protocol products to the actual functions supplied by Nucleus NET.
   If needed, these mappings can be changed to use a different set
   of functions, possible from a customer/tool supplied 'C' library.
*/
#define NU_STRICMP      NCL_Stricmp
#define NU_ITOA         NCL_Itoa
#define NU_ULTOA        NCL_Ultoa
#define NU_ATOI         NCL_Atoi
#define NU_ATOL         NCL_Atol
#define NU_TOUPPER      NCL_To_Upper
#define NU_BLOCK_COPY   memcpy

/* This macro is used to remove warnings. */
#define UNUSED_PARAMETER(x)     TLS_Unused_Parameter = ((UINT32)(x))

/* Define Supervisor and User mode functions */
#if (!defined(NU_SUPERV_USER_VARIABLES))
#define NU_IS_SUPERVISOR_MODE() (NU_TRUE)
#define NU_SUPERVISOR_MODE() ((void) 0)
#define NU_USER_MODE() ((void) 0)
#define NU_SUPERV_USER_VARIABLES    /* Not a Supervisor/User kernel */
#endif /* NU_SUPERV_USER_MODE */

/* added by sunbey */
extern NU_MEMORY_POOL gSystemMemory;
#define System_Memory gSystemMemory

#endif  /* TARGET_H */
