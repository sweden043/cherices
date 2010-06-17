/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       toolchain.h
 *
 *
 * Description:    Toolchain-specific macro definitions
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: toolchain.h, 1, 12/8/03 3:44:47 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _TOOLCHAIN_H_
#define _TOOLCHAIN_H_

/*******************************************************************************/
/* Structure packing control                                                   */
/*                                                                             */
/* Different toolchains control packed structure generation in many different  */
/* ways. We define macros that are used in header files to prevent the need to */
/* edit the header if and when new toolchains are supported.                   */
/*******************************************************************************/
#if (ARM_TOOLKIT == ADS) || (ARM_TOOLKIT == SDT)

#define PACKED_STRUCT             typedef __packed struct
#define PACKED_MEMBER(type, name) type name;
#define PACKED_STRUCT_NAME(name)  name;

#endif /* ARM toolchains */

#if (ARM_TOOLKIT == WRGCC) || (ARM_TOOLKIT == GNUGCC)

#define PACKED_STRUCT             typedef struct
#define PACKED_MEMBER(type, name) type name __attribute__ ((packed));
#define PACKED_STRUCT_NAME(name)  name __attribute__ ((packed));

#endif /* GNU toolchains */

#endif /* _TOOLCHAIN_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         12/8/03 3:44:47 PM     Dave Wilson     CR(s) 
 *        8117 : Header file containing macro definitions designed to 
 *        encapsulate compiler-specific syntax for various pragmas, etc.
 *        
 * $
 *
 ****************************************************************************/

