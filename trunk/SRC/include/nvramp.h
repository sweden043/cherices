/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       nvramp.h                                                 */
/*                                                                          */
/* Description:    NVRAM Private Interface Header file                      */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           02/09/00                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _NVRAMP_H_
#define _NVRAMP_H_

/*****************/
/* Include Files */
/*****************/

/***********/
/* Aliases */
/***********/
#define NV_PRIV_SIZE        4096    // Private data size
/* Placed NV at the Top of EE */
#if EEPROM_TYPE == EEPROM_32KB
#define NVPRIV_BASE_ADDR   ( 32768 - NV_PRIV_SIZE ) 
#else /* EEPROM_TYPE == EEPROM_32KB */
#define NVPRIV_BASE_ADDR   ( 16384 - NV_PRIV_SIZE )
#endif /* EEPROM_TYPE == EEPROM_32KB */

/***********************/
/* Function prototypes */
/***********************/
bool nvp_init(void);
int nvram_private_get_size(void);
int nvram_private_write(unsigned short offset, unsigned short size, const 
                                                unsigned char *data);
int nvram_private_read(unsigned short offset, unsigned short size, 
                                                unsigned char *data);

#endif // _NVRAMP_H_

