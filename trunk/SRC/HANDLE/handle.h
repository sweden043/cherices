/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                        SOFTWARE FILE/MODULE HEADER                       */
/*                 Copyright Conexant Systems Inc. 1998-2004                */
/*                                Austin, TX                                */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:        handle.h
 *
 *
 * Description:     Public header file defining types and macros for handle
 *                  processing
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Id: handle.h,v 1.14, 2004-04-19 14:22:44Z, Billy Jackman$
 ****************************************************************************/

#ifndef _HANDLE_H_
#define _HANDLE_H_

/****************************************************************************/
/* Usage Notes                                                              */
/*                                                                          */
/* This header file provides the interface definition for standard handle   */
/* manipulation functions as well as standard error checking macros.        */
/*                                                                          */
/* The handle manipulation functions depend upon two key requirements:      */
/* 1) The first element in the handle instance structure must be a          */
/*    CNXT_HANDLE_PREFACE structure named Preface.                          */
/* 2) Handle instance structures are stored in lists, linked together by    */
/*    the pNext pointers in the Preface.                                    */
/*                                                                          */
/* Modules using the handle manipulation functions must set certain fields  */
/* of the Preface to the proper values at appropriate times:                */
/* 1) pSelf must be set to the address of the instance structure while the  */
/*    instance structure is valid (in the open and reopen functions) and to */
/*    a different value while the instance structure is not valid (set to   */
/*    NULL in the close function).                                          */
/* 2) Type must be set to the proper CNXT_HANDLE_TYPE for the module using  */
/*    the handle functions. This may be done once at initialization.        */
/* 3) uPhysUnit must be set to a module-dependent value indicating what     */
/*    physical unit the instance structure is associated with in the open   */
/*    and reopen functions. uPhysUnit may be different from the logical     */
/*    unit number an instance structure represents. uPhysUnit provides the  */
/*    information necessary to other modules to determine how to attach a   */
/*    handle from this module.                                              */
/*                                                                          */
/* Because the handle manipulation functions depend upon instances being    */
/* stored in lists, the client module must initialize its free pool of      */
/* instance structures as a list by chaining the structures together with   */
/* the pNext member of Preface, terminating the list with the last instance */
/* having pNext set to NULL.                                                */
/*                                                                          */
/* The error checking macros IS_DRIVER_INITED, IS_VALID_HANDLE, and         */
/* IS_NOT_NULL_POINTER could expand to include a direct return of a module  */
/* error code. They also may expand differently in DEBUG vs. non-DEBUG      */
/* environments.                                                            */
/****************************************************************************/

/********************************/
/* Symbol and Macro definitions */
/********************************/

#define INVALID_UNIT_NUMBER   (0xffffffff)

/******************************/
/* Handle manipulation macros */
/******************************/

/* CREATE_HANDLE expands to a call to cnxt_handle_dequeue to remove the first
   instance from the specified free list. */
#define CREATE_HANDLE(x,y)   cnxt_handle_dequeue((void**)(x),(void**)(y))

/* DESTROY_HANDLE expands to a call to cnxt_handle_enqueue to add the specified
   handle to the specified free list. */
#define DESTROY_HANDLE(x,y)  cnxt_handle_enqueue((void**)(x),(void*)(y))

/* ADD_HANDLE expands to a call to cnxt_handle_add to add the specified handle
   to the specified list. */
#define ADD_HANDLE(x,y)      cnxt_handle_add((void**)(x),(void*)(y))

/* REMOVE_HANDLE expands to a call to cnxt_handle_remove to remove the
   specified handle from the specified list. */
#define REMOVE_HANDLE(x,y)   cnxt_handle_remove((void**)(x),(void*)(y))


/* The error checking macros IS_VALID_HANDLE, IS_DRIVER_INITED, and
   IS_NOT_NULL_POINTER may expand differently in DEBUG mode and non-DEBUG
   mode. */
#ifdef DEBUG

   #define IS_VALID_HANDLE(x,y,z)                                     \
   {                                                                  \
      if((y)==NULL)                                                   \
      {                                                               \
         return CNXT_##x##_BAD_HANDLE;                                \
      }                                                               \
      if((y)->Preface.pSelf != (CNXT_HANDLE_PREFACE *)(y))            \
      {                                                               \
         return CNXT_##x##_BAD_HANDLE;                                \
      }                                                               \
      if(offsetof(CNXT_##x##_INST, Preface) != 0)                     \
      {                                                               \
         return CNXT_##x##_BAD_HANDLE;                                \
      }                                                               \
      if ((z) && (cnxt_handle_exist((void**)(z),(void*)(y))==FALSE))  \
      {                                                               \
         return CNXT_##x##_BAD_HANDLE;                                \
      }                                                               \
   }

   #define IS_DRIVER_INITED(x,y)         \
   {                                     \
      if ((y) == FALSE)                  \
      {                                  \
         return CNXT_##x##_NOT_INIT;     \
      }                                  \
   }

   #define IS_NOT_NULL_POINTER(x,y)          \
   {                                         \
      if ((y) == NULL)                       \
      {                                      \
         return CNXT_##x##_BAD_PARAMETER;    \
      }                                      \
   }

#else

   /* In non-DEBUG builds, error checking macros are expanded to check for NULL
      pointer only */

   #define IS_VALID_HANDLE(x,y,z)        \
   {                                     \
      if((y)==NULL)                      \
      {									 \
         return CNXT_##x##_BAD_HANDLE;   \
      }                                  \
   }

   #define IS_DRIVER_INITED(Name,y)      ((void)(y))

   #define IS_NOT_NULL_POINTER(x,y)          \
   {                                         \
      if ((y) == NULL)                       \
      {                                      \
         return CNXT_##x##_BAD_PARAMETER;    \
      }                                      \
   }

#endif

/*****************/
/* Data Types    */
/*****************/
typedef void *CNXT_HANDLE;

typedef enum
{
   CNXT_HANDLE_TYPE_TVENC = 1,
   CNXT_HANDLE_TYPE_ATVTUN,
   CNXT_HANDLE_TYPE_ACAP,
   CNXT_HANDLE_TYPE_AVID,
   CNXT_HANDLE_TYPE_FIREWIRE,
   CNXT_HANDLE_TYPE_OC_OOBFE,
   CNXT_HANDLE_TYPE_PODHI,
   CNXT_HANDLE_TYPE_RFMOD,
   CNXT_HANDLE_TYPE_SCART,
   CNXT_HANDLE_TYPE_SMC,
   CNXT_HANDLE_TYPE_SWDL,
   CNXT_HANDLE_TYPE_CCHANGE,
   CNXT_HANDLE_TYPE_POWER,
   CNXT_HANDLE_TYPE_DVBLIB,
   CNXT_HANDLE_TYPE_BUTTONS
   #if (defined DRIVER_INCL_MIA_TEMPLATE) || (defined DRIVER_INCL_MIA_THREAD_TEMPLATE)
   , CNXT_HANDLE_TYPE_YOURDRIVER
   #endif
} CNXT_HANDLE_TYPE;

typedef struct cnxt_handle_preface
{
   struct cnxt_handle_preface  *pSelf;
   struct cnxt_handle_preface  *pNext;
   CNXT_HANDLE_TYPE            Type;
   u_int32                     uPhysUnit;
} CNXT_HANDLE_PREFACE;

/**********************/
/* Utility Prototypes */
/**********************/
bool cnxt_handle_add ( void **ppFirstInst, void *pInst );
bool cnxt_handle_enqueue ( void **ppFirstInst, void *pInst );
bool cnxt_handle_remove ( void **ppFirstInst, void *pInst );
bool cnxt_handle_dequeue ( void **ppFirstInst, void **ppInst );
#ifdef DEBUG
bool cnxt_handle_exist ( void **ppFirstInst, void *pInst );
#endif
bool cnxt_handle_get_type ( CNXT_HANDLE hHandle, CNXT_HANDLE_TYPE *pType );
bool cnxt_handle_get_physunit ( CNXT_HANDLE hHandle, u_int32 *puPhysUnit );

#endif   /* _HANDLE_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  15   mpeg      1.14        4/19/04 9:22:44 AM     Billy Jackman   CR(s) 
 *        8883 8884 : Added check of self-reference pointer to IS_VALID_HANDLE 
 *        macro.
 *        Modified keywords for expansion to StarTeam format.
 *  14   mpeg      1.13        4/7/04 3:34:06 PM      Dave Wilson     CR(s) 
 *        8803 : Added new handle for BUTTONS driver (directory PANEL_BUTTONS)
 *  13   mpeg      1.12        3/22/04 9:37:29 AM     Dave Wilson     CR(s) 
 *        8602 : Added conditional to ensure that MIA_THREAD_TEMPLATE could be 
 *        used in addition to MIA_TEMPLATE and still have the appropriate 
 *        YOURDRIVER handle type included.
 *  12   mpeg      1.11        3/4/04 9:14:41 PM      Matt Korte      CR(s) 
 *        8519 : Fix warning
 *  11   mpeg      1.10        3/1/04 2:23:12 PM      Dave Wilson     CR(s) 
 *        8484 : Added handle type for YOURDRIVER to allow the MIA_TEMPLATE 
 *        module to build without modification.
 *  10   mpeg      1.9         9/23/03 4:24:06 PM     Craig Dry       SCR(s) 
 *        7532 :
 *        Add Power Driver Handle Type
 *        
 *  9    mpeg      1.8         9/23/03 3:13:58 PM     Billy Jackman   SCR(s) 
 *        7527 :
 *        Expand macro DESTROY_HANDLE to a call to cnxt_handle_enqueue instead 
 *        of to
 *        cnxt_handle_add. Since DESTROY_HANDLE adds the handle back onto the 
 *        free list,
 *        cnxt_handle_add was failing due to its internal checks for list 
 *        consistency.
 *        cnxt_handle_enqueue does not check the pSelf pointer that was causing
 *         the error.
 *        
 *  8    mpeg      1.7         9/18/03 12:23:46 PM    Billy Jackman   SCR(s) 
 *        7482 :
 *        Change the non-DEBUG version of the IS_VALID_HANDLE macro to use the 
 *        same
 *        number of parameters as the debug version.
 *        
 *  7    mpeg      1.6         9/18/03 10:10:02 AM    Billy Jackman   SCR(s) 
 *        7482 :
 *        Added module CCHANGE to CNXT_HANDLE_TYPE.
 *        
 *  6    mpeg      1.5         9/17/03 2:01:08 PM     Billy Jackman   SCR(s) 
 *        7482 :
 *        Updated for new Multi Instance Driver Guidelines
 *        
 *        
 *  5    mpeg      1.4         7/31/03 6:32:22 PM     Angela Swartz   SCR(s) 
 *        5994 :
 *        check NULL pointer for CHECK_VALID_HANDLE and CHECK_NULL_POINTER in 
 *        release build
 *        
 *  4    mpeg      1.3         5/23/03 5:03:10 PM     Tim Ross        SCR(s) 
 *        6580 6581 :
 *        Added INVALID_UNIT_NUMBER definition.
 *        
 *  3    mpeg      1.2         2/11/03 9:20:36 AM     Larry Wang      SCR(s) 
 *        5462 :
 *        move prototypes of cnxt_handle_add() and cnxt_handle_remove() out of 
 *        #ifdef DEBUG block.
 *        
 *  2    mpeg      1.1         1/30/03 8:15:36 AM     Larry Wang      SCR(s) 
 *        5323 :
 *        Add $Log section.
 *        
 *  1    mpeg      1.0         1/27/03 12:32:28 PM    Larry Wang      
 * $
 *
 ****************************************************************************/

