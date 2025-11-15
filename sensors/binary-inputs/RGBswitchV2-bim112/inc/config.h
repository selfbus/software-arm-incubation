/*
 * config.h
 *
 *  Created on: 27.01.2018
 *      Author: uwe223
 *
 *  Modified for RGB Switch V2 
 *  Copyright (c) 2025 Oliver Stefan <o.stefan252@gmail.com>
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <app_RGBswitchV2.h>
#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>

#define MANUFACTURER 0x83       // MDT

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short deviceType;  //!> bcu.begin devicetype
    unsigned short appVersion;  //!> application version
    unsigned short baseAddress; //!> Base address of the config parameters (0x4400 + Offset first param.(debounce-Time))
    unsigned short logicBaseAddress; //!> Base address of the logic parameters
    byte hardwareVersion[6];    //!> The hardware identification number (see selfbus wiki "Verwendung der sblib")
    const char *version;
} HardwareVersion;

extern const HardwareVersion * currentVersion;

const HardwareVersion hardwareVersion[1] =
{
	{ 6, 0x0031, 0x20, 0x44D4, 0x4650, { 0, 0, 0, 0, 0x01, 0x1E }, "RGB Switch 6f" }
};

#define EE_LED_PARAMS_BASE 0x46A9

#define HARDWARE_ID 0
#define INVERT

// Input pins
const int inputPins[] =
{
	PIO1_7,  //  A
	PIO1_10, //  B
	PIO0_1,  //  C
	PIO2_4,  //  D
	PIO2_8,  //  E
	PIO2_10, //  F

};

#endif /* CONFIG_H_ */
