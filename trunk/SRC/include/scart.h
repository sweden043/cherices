
/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       SCART.H                                                  */
/*                                                                          */
/* Description:    SCART control definitions allowing a version of the      */
/*                 OpenTV 1.2 driver to be used on previous OpenTV releases */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
#ifndef _SCART_H_
#define _SCART_H_
       
/***********************************************/
/* Typedefs copied from OpenTV 1.2 header file */
/***********************************************/
typedef struct {
        unsigned char   mode;
} o_scart_mode; 

#define SCARTS_OFF		0
#define SCARTS_ON		1
#define SCARTS_STANDBY		2

typedef struct {
        unsigned char   scart;
        unsigned char   pin8_state;
        unsigned char   pin16_state;
} o_scart_state;

#define TV_SCART                0 
#define VCR_SCART               1 
#define AUX_SCART               2 

#define SCART_PIN_DISABLED	0
#define SCART_PIN_ENABLED	1

typedef struct {
        unsigned char   scart;
        unsigned char   pin8_enabled_voltage;
	unsigned char   pin16_enabled_voltage;
} o_scart_attributes; 

#define SCART_PIN_VOLTAGE_LOW	0
#define SCART_PIN_VOLTAGE_MID	1
#define SCART_PIN_VOLTAGE_HIGH	2

#define MODE_DATA		0x01
#define STATE_DATA		0x02
#define ATTRIBUTES_DATA		0x04

/* Function Prototypes */
bool init_avpro_scart(void);
bool scart_sys_get(int request, voidF data);
bool scart_sys_set(int request, voidF data);

#endif