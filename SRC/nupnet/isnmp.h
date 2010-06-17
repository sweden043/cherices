/***********************************************************************
*                                                                       
*        Copyright (c) 1993 - 2001 Accelerated Technology, Inc.         
*                                                                       
* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      
* subject matter of this material.  All manufacturing, reproduction,    
* use, and sales rights pertaining to this subject matter are governed  
* by the license agreement.  The recipient of this software implicitly  
* accepts the terms of the license.                                     
*                                                                       
***********************************************************************/

/***********************************************************************
*                                                                       
*   FILE NAME                                       VERSION                       
*                                                                               
*       isnmp.h                                       4.4                        
*                                                                               
*   COMPONENT                                                             
*                                                                       
*       Configuration                                                    
*                                                                       
*   DESCRIPTION                                                           
*                                                                       
*       The only purpose of this file is to control the inclusion of     
*       Nucleus SNMP code into the Nucleus NET library.                  
*                                                                       
*   DATA STRUCTURES                                                       
*                                                                       
*       None
*                                                                       
*   DEPENDENCIES                                                          
*                                                                       
*      None
*                                                                       
***********************************************************************/

#ifndef ISNMP_H
#define ISNMP_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++  */
#endif /* _cplusplus */

/* This macro specifies if SNMP update calls will be included in the
   build of the Nucleus NET library. To build in SNMP support change the 
   NU_FALSE to a NU_TRUE. */

#ifndef INCLUDE_SNMP
#define INCLUDE_SNMP                    NU_FALSE
#endif

#if (INCLUDE_SNMP == NU_TRUE)


/* This macro specifies the SNMP glue header file to include. The first 
   (snmp_g.h) is for Nucleus SNMP. The second (sr_snmp_g.h) is for SNMP 
   Research SNMP*/

#define SNMP_GLUE  "snmp_g.h"

/* #define SNMP_GLUE  "snmp\src\include\sr_snmpg.h" */

#else

/* If no SNMP support is desired then define the SNMP macros to be null strings. */
#define NU_SNMP_Initialize()
#define SR_SNMP_Initialize()
#define SNMP_sysDescr(string)
#define SNMP_sysObjectID(string)
#define SNMP_sysUpTime(value)
#define SNMP_sysUpTime_Inc
#define SNMP_sysContact(string)
#define SNMP_sysName(string)
#define SNMP_sysLocation(string)
#define SNMP_sysServices(value)
#define SNMP_atTableUpdate(command, index, phys_addr, net_addr)
#define SNMP_ipInReceives_Inc
#define SNMP_ipInHdrErrors_Inc
#define SNMP_ipInAddrErrors_Inc
#define SNMP_ipForwarding
#define SNMP_ipForwDatagrams_Inc
#define SNMP_ipInUnknownProtos_Inc
#define SNMP_ipInDiscards_Inc
#define SNMP_ipInDelivers_Inc
#define SNMP_ipOutRequests_Inc
#define SNMP_ipOutDiscards_Inc
#define SNMP_ipOutNoRoutes_Inc
#define SNMP_ipReasmTimeout(value)
#define SNMP_ipReasmReqds_Inc
#define SNMP_ipReasmOKs_Inc
#define SNMP_ipReasmFails_Inc
#define SNMP_ipFragOKs_Inc
#define SNMP_ipFragFails_Inc
#define SNMP_ipFragCreates_Inc
#define SNMP_ipAdEntUpdate(command, index, ip_addr, mask, bcast, reasm_size)
#define SNMP_ipNetToMediaTableUpdate(command, index, phys_addr, net_addr, type)
#define SNMP_ipRoutingDiscards_Inc
#define SNMP_icmpInMsgs_Inc
#define SNMP_icmpInErrors_Inc
#define SNMP_icmpInDestUnreachs_Inc
#define SNMP_icmpInTimeExcds_Inc
#define SNMP_icmpInParmProbs_Inc
#define SNMP_icmpInSrcQuenchs_Inc
#define SNMP_icmpInRedirects_Inc
#define SNMP_icmpInEchos_Inc
#define SNMP_icmpInEchoReps_Inc
#define SNMP_icmpInTimeStamps_Inc
#define SNMP_icmpInTimeStampReps_Inc
#define SNMP_icmpInAddrMasks_Inc
#define SNMP_icmpInAddrMaskReps_Inc
#define SNMP_icmpOutMsgs_Inc
#define SNMP_icmpOutErrors_Inc
#define SNMP_icmpOutDestUnreachs_Inc
#define SNMP_icmpOutTimeExcds_Inc
#define SNMP_icmpOutParmProbs_Inc
#define SNMP_icmpOutSrcQuenchs_Inc
#define SNMP_icmpOutRedirects_Inc
#define SNMP_icmpOutEchos_Inc
#define SNMP_icmpOutEchoReps_Inc
#define SNMP_icmpOutTimestamps_Inc
#define SNMP_icmpOutTimestampReps_Inc
#define SNMP_icmpOutAddrMasks_Inc
#define SNMP_icmpOutAddrMaskReps_Inc
#define SNMP_tcpRtoAlgorithm(value)
#define SNMP_tcpRtoMin(value)
#define SNMP_tcpRtoMax(value)
#define SNMP_tcpMaxCon(value)
#define SNMP_tcpActiveOpens_Inc
#define SNMP_tcpPassiveOpens_Inc
#define SNMP_tcpAttemptFails_Inc
#define SNMP_tcpEstabResets_Inc
#define SNMP_tcpInSegs_Inc
#define SNMP_tcpOutSegs_Inc
#define SNMP_tcpRetransSegs_Inc
#define SNMP_tcpInErrs_Inc
#define SNMP_tcpOutRsts_Inc
#define SNMP_udpInDatagrams_Inc
#define SNMP_udpNoPorts_Inc
#define SNMP_udpInErrors_Inc
#define SNMP_udpoutDatagrams_Inc
#define SNMP_ifTotalInterfaces(value)
#define SNMP_ifCreate(index)
#define SNMP_ifNumber(value)
#define SNMP_ifDescr(index, string)
#define SNMP_ifType(index, value)
#define SNMP_ifMtu(index, value)
#define SNMP_ifSpeed(index, value)
#define SNMP_ifPhysAddress(index, addr)
#define SNMP_ifAdminStatus(index, status)
#define SNMP_ifOperStatus(index, status)
#define SNMP_ifLastChange(index, time)
#define SNMP_ifInOctets(index, value)
#define SNMP_ifInUcastPkts_Inc(index)
#define SNMP_ifInNUcastPkts_Inc(index)
#define SNMP_ifInDiscards_Inc(index)
#define SNMP_ifInErrors_Inc(index)
#define SNMP_ifInUnknownProtos_Inc(index)
#define SNMP_ifOutOctets(index, value)
#define SNMP_ifOutUcastPkts_Inc(index)
#define SNMP_ifOutNUcastPkts_Inc(index)
#define SNMP_ifOutDiscards_Inc(index)
#define SNMP_ifOutErrors_Inc(index)
#define SNMP_ifOutQLen_Inc(index)
#define SNMP_ifOutQLen_Dec(index)
#define SNMP_ifSpecific(index, string)
#define SNMP_ifIndex(index)

#endif /* INCLUDE_SNMP */

#ifdef          __cplusplus
}
#endif /* _cplusplus */

#endif /* ifndef ISNMP_H */
