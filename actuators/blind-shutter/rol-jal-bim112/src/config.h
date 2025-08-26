/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/types.h>
#include <sblib/io_pin_names.h>

// we are a MDT shutter/blind actuator, version 2.8
#define MANUFACTURER 131
#define APPVERSION   0x28

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short baseAddress; //!> Base address of the config parameters
    byte hardwareVersion[6];    //!> The hardware identification number
} HardwareVersion;

extern const HardwareVersion * currentVersion;


//#define HAND_ACTUATION

//#define USE_DEV_LEDS
//#define MEM_TEST

/*
 *  bus power-failure configuration
 *  Use 1% tolerance or better resistors for the voltage divider R3 & R12 on 4TE controller & TS_ARM
 *  TODO change resistors R3 & R12 tolerance in schematic to 1%
 */
#ifdef BUSFAIL
#   define VBUS_AD_PIN PIN_VBUS
#   define VBUS_AD_CHANNEL AD7
#   define VBUS_VOLTAGE_FAILTIME_MS 20
#   define VBUS_VOLTAGE_RETURNTIME_MS 1500
#   define VBUS_ADC_SAMPLE_FREQ 10000 // ADC sampling frequency in Hz

#   define VBUS_THRESHOLD_FAILED 24000 // millivoltage for normal relays
#   define VBUS_THRESHOLD_RETURN 26000
#endif


/*
 *  hand actuation pin configuration
 */
#ifdef HAND_ACTUATION
#   define NO_OF_HAND_PINS 8
#   define READBACK_PIN PIN_LT9
#   define BLINK_TIME 500
    // LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    const unsigned int handPins[NO_OF_HAND_PINS] = { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };
#endif

#endif /* CONFIG_H_ */
