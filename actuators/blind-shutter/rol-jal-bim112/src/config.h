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

/**
 * @def HAND_ACTUATION
 * Define to compile with hand actuation support.
 *
 * @note Currently set by build variable ${hand_actuation} in eclipse menu "Project properties->C/C++ Build->Build Variables"
 * @warning Hand actuation is currently only supported for 4-channel builds.
 */
// #define HAND_ACTUATION

/**
 * @def CHANNEL_COUNT
 * Define to set the number of blind/fold channels (1, 2, 4 or 8)
 *
 * @warning Currently only 4 or 8 channels are supported (8 Channels experimental).
 */
#define CHANNEL_COUNT 4

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short baseAddress; //!> Base address of the config parameters
    byte hardwareVersion[6];    //!> The hardware identification number. Must match the product_serial_number in the VD's table hw_product
} HardwareVersion;


#define NO_OF_OUTPUTS (CHANNEL_COUNT * 2)

#if CHANNEL_COUNT == 4
    // JAL-0410.01 Shutter Actuator 4-fold, 4TE, 230VAC, 10A
    const HardwareVersion currentVersion = {4, 0x4578, { 0, 0, 0, 0, 0x0, 0x29 }};
    const int outputPins[NO_OF_OUTPUTS] =
        { PIN_IO1, PIN_IO2, PIN_IO3, PIN_IO4, PIN_IO5, PIN_IO6, PIN_IO7, PIN_IO8 };
#elif CHANNEL_COUNT == 8
#   warning "8 channel build is experimental!"
    // JAL-0810.01 Shutter Actuator 8-fold, 8TE, 230VAC,10A
    const HardwareVersion currentVersion = {8, 0x46B8, { 0, 0, 0, 0, 0x0, 0x28 }};
    const int outputPins[NO_OF_OUTPUTS] =
        {
          PIN_IO1, PIN_IO2, PIN_IO3, PIN_IO4, PIN_IO5, PIN_IO6, PIN_IO7, PIN_IO8,
          PIN_TX, PIN_RX, PIN_IO9, PIN_IO10, PIN_IO12, PIN_IO13, PIN_IO14, PIN_IO15
        };
#else
#   error "CHANNEL_COUNT must be defined"
#endif

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
#   if CHANNEL_COUNT != 4
#       error "HAND_ACTUATION only supported for CHANNEL_COUNT == 4"
#   endif
#   define NO_OF_HAND_PINS 8
#   define READBACK_PIN PIN_LT9
#   define BLINK_TIME 500
    // LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    const unsigned int handPins[NO_OF_HAND_PINS] = { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };
#endif

#endif /* CONFIG_H_ */
