/*
 *  Copyright (c) 2016 Oliver Stefan <os252@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef params_h
#define params_h

//#include <sblib/mem_mapper.h>
#include "ext_eeprom.h"

// Eeprom address: Start of Function parameters
#define EE_FUNCTIONS_PARAMS_BASE         0x01f6	//D:502

// Eeprom address: start of timing parameters
#define EE_TIMING_PARAMS_BASE  0x01f7 //D503


// parameters in address 502 of VD
#define TEMPERATURE_FUNCTION_ACTIVE	0x80
#define AIR_QUALITY_ACTIVE			0x40
#define AIR_HUMIDITY_ACTIVE			0x20
#define DISPLAY_WINDOW_OPEN			0x10
#define DISPLAY_AIR_VENTILATION		0x08
#define EXTERNAL_TEMP_SENS			0x04

// Eeprom address: Send  actual temperature cyclically (not send = 0 or value: 1,2,3,4,5,10,15,20,30,40,50,60 min)
// size: 8 bit
#define EE_ACTUAL_TEMP_SEND_CYCLIC 361

// Eeprom address: Send actual temperature value after change of (not send = 0, steps 0,1K, range: 0,1K - 2.0K)
#define EE_ACTUAL_TEMP_SEND_CHANGE 363

// Eeprom address: Send setpoint at change (no = 0, yes = 1)
#define EE_SETPOINT_TEMP_SEND 407

enum timed_values {
	SW1_LONG_PRESS,
	SW2_LONG_PRESS,
	TEMPERATURES,
	AIR_QUALITY,
	AIR_HUMIDITY,
	LCD_HOME_SCREEN, 	//timeout to get back to home screen
	LCD_BACKLIGHT,		//timeout to shut backlight off
	RETURN_TO_AUTO,		//timeout to get back to automatic (extern) temperature control
	NUM_TIMED_VALUES
};

extern Timeout timeout[NUM_TIMED_VALUES];

// Enumeration of all COM Objects
enum comObjects {
	Dummy0,
	Dummy1,
	Dummy2,
	Dummy3,
	Dummy4,
	Dummy5,
	DUmmy6,
	Dummy7,
	SEND_INTERN_TEMP,
	SEND_SET_TEMP,
	REC_EXT_SET_TEMP,
	SEND_AIR_QUALITY,
	SEND_AIR_HUMIDITY,
	REC_WINDOW_STATE,
	REC_VENTILATION_LEVEL,
	REC_EXT_TEMP
};

//extern MemMapper memMapper;
extern ExtEeprom extEeprom;

// User Flash (=UF) Settings for extern EEPROM
// defined is the start address of the memory space

#define UF_TEMP_AUTO_RESET_TIME		0	// unsigned int
#define UF_TEMP_SOLL_INTERN			4	// unsigned int
#define UF_TEMP_SOLL_INTERN_LUFT	8	// unsigned int
#define UF_TEMP_SOLL_EXTERN			12	// unsigned int
#define UF_TEMP_SOLL_TEMP_FLAG		16	// unsigned int
#define UF_LCD_BRIGHTNESS			20	// unsigned int
#define UF_INITIALIZED				24	// byte

#endif
