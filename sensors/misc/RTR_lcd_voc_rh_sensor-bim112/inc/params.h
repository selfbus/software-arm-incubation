/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef params_h
#define params_h

//#include <sblib/mem_mapper.h>
#include <sblib/timeout.h>
#include <sblib/mem_mapper.h>

// Eeprom address: Start of Function parameters
#define EE_FUNCTIONS_PARAMS_BASE         		0x4500
#define EE_TARGET_TEMP_FUNCTIONS_BASE			0x4501
#define EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE		0x4502

// Eeprom address: start of timing parameters
#define EE_SENDING_PARAMS_BASE  				0x4503

// 1 Byte Cyclic Faktor and Cyclic Unit + 2 Byte DPT9 Change Value
#define EE_SIZE_OF_EACH_SENDING_PARAM			0x3

// sequence of parameters for sending conditions (cyclic and after change)
#define EE_INTERNAL_TEMP_SENDING_PARAM			0
#define EE_VOC_SENDING_PARAM					1
#define EE_HUMIDITY_SENDING_PARAM				2
#define EE_EXTERNAL_TEMP_SENDING_PARAM			3
#define EE_TARGET_TEMP_SENDING_PARAM			4


// Eeprom address: switch debounce time [ms]
#define EE_SWITCH_DEBOUNCE_TIME 				0x4510


// parameters in address 4500 of knxprod
#define SEND_INTERNAL_TEMPERATURE_CYCLIC		0x80
#define SEND_INTERNAL_TEMPERATURE_AFTER_CHANGE 	0x40
#define SEND_VOC_CYCLIC							0x20
#define SEND_VOC_AFTER_CHANGE 					0x10
#define SEND_HUMIDITY_CYCLIC					0x08
#define SEND_HUMIDITY_AFTER_CHANGE 				0x04
#define SEND_EXTERNAL_TEMPERATURE_CYCLIC		0x02
#define SEND_EXTERNAL_TEMPERATURE_AFTER_CHANGE 	0x01


// parameters in address 4501 of knxprod
#define SEND_TARGET_TEMPERATURE_CYCLIC			0x80
#define SEND_TARGET_TEMPERATURE_AFTER_CHANGE 	0x40
#define USE_EXTERNAL_TARGET_TEMPERATURE			0x20

// parameters in address 4502 of VD
#define EXT_TEMP_SENSOR_IS_CONNECTED			0x80
#define CONN_EXT_TEMP_SENSOR					0x40	//if the external Temp Sensor is connected to the board or the value is provided via KNX
#define DISPLAY_WINDOW_OPEN						0x20
#define DISPLAY_AIR_VENTILATION					0x10
#define DISPLAY_HEATING_STATE					0x08



enum timed_values {
	SW1_LONG_PRESS,
	SW2_LONG_PRESS,
	TEMPERATURES_KO,
	TARGET_TEMPERATURE_KO,
	EXTERNAL_TEMPERATURE_KO,
	TEMPERATURES_LCD,
	AIR_QUALITY_KO,
	AIR_QUALITY_LCD,
	AIR_HUMIDITY_KO,
	AIR_HUMIDITY_LCD,
	LCD_HOME_SCREEN, 	//timeout to get back to home screen
	LCD_BACKLIGHT,		//timeout to shut backlight off
	RETURN_TO_AUTO,		//timeout to get back to automatic (extern) temperature control
	NUM_TIMED_VALUES
};

extern Timeout timeout[NUM_TIMED_VALUES];

// Enumeration of all COM Objects
enum comObjects {
	SEND_INTERN_TEMP,
	SEND_SET_TEMP,
	REC_EXT_SET_TEMP,
	SEND_AIR_QUALITY,
	SEND_AIR_HUMIDITY,
	REC_WINDOW_STATE,
	REC_VENTILATION_LEVEL,
	SEND_REC_EXT_TEMP
};

extern MemMapper memMapper;

// User Flash (=UF) Settings for extern EEPROM
// defined is the start address of the memory space
#define UF_BASE_ADDRESS             0xfd00
#define UF_SIZE                     0x100
#define UF_TEMP_AUTO_RESET_TIME     (UF_BASE_ADDRESS + 0)  // unsigned int
#define UF_TEMP_SOLL_INTERN         (UF_BASE_ADDRESS + 4)  // unsigned int
#define UF_TEMP_SOLL_INTERN_LUFT    (UF_BASE_ADDRESS + 8)  // unsigned int
#define UF_TEMP_SOLL_EXTERN         (UF_BASE_ADDRESS + 12) // unsigned int
#define UF_TEMP_SOLL_TEMP_FLAG      (UF_BASE_ADDRESS + 16) // unsigned int
#define UF_LCD_BRIGHTNESS           (UF_BASE_ADDRESS + 20) // unsigned int
#define UF_INITIALIZED              (UF_BASE_ADDRESS + 24) // byte

#endif
