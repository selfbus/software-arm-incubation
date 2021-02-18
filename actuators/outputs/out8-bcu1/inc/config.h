/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>

/*
 * #define symbols "moved" to
 * eclipse menu "Project properties->C/C++ Build->Build Variables"
 *
 * set the resulting build name (*.hex, *.axf, *.bin) under
 * eclipse menu "Project properties->C/C++ Build->Settings->[TAB]Build Artifact->Artifact name"
 * e.g. ${Proj_name}-${num_outputs}-BCU${bcu_type}-${controller_type}-${relay_type}-${inverted}-${hand_actuation}-${bus_fail}_${sw_version}
 *
 *
 * build variables available / default value:
 * bcu_type = 10
 * controller_type = TE4
 * hand_actuation = HAND_ACTUATION
 * inverted = INVERT
 * num_outputs = OUT8
 * relay_type = BI_STABLE
 * bus_fail = BUSFAIL
 * sw_version = 0.5
 *
 */


/*
 * general application & relays configuration:
 *      channelnumber
 *      bi-stable relays
 *      zero-detect
 *      hand actuation
 *      bus fail detection
 */

// #define IO_TEST // can be set, to perform a little test of all relays and hand actuation LED's
// #define DEBUG_SERIAL // can be set, to send some debug messages over the serial port

// #define BI_STABLE // replaced by build-variable ${relay_type}
// #define HAND_ACTUATION // replaced by build-variable ${hand_actuation}

// #define ZERO_DETECT // no more supported? at least it's mentioned in a comment from 2014
                       // https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/Bin%C3%A4rausgang_8x230_16A_4TE
                       // PIO_SDA is used for zero-detect

/*
 *  hand actuation pin configuration
 */
#ifdef HAND_ACTUATION
#   define NO_OF_HAND_PINS 8
#   define READBACK_PIN PIN_LT9
#   define BLINK_TIME 500
    // LedTaster_4TE_3.54 : links oben = Kanal 1 nach unten aufsteigende Kanalnummer ; rechts oben = Kanal 5 ; nach unten aufsteigende Kanalnummer
    const unsigned int handPins[NO_OF_HAND_PINS] = { PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT5, PIN_LT6, PIN_LT7, PIN_LT8 };

    // for weatherstation-bim112 see commit 870c9801bea5aa616c22fde29c8219e4cb5f28b6
    // const unsigned int handPins[8] = { PIN_LT5, PIN_LT6, PIN_LT1, PIN_LT2, PIN_LT3, PIN_LT4, PIN_LT7, PIN_LT8 };
#endif

/*
 *  bus power-failure configuration
 *  Use 1% tolerance or better resistors for the voltage divider R3 & R12 on 4TE controller & TS_ARM
 *  TODO change resistors in schematic to 1%
 */
#ifdef BUSFAIL
#   define VBUS_AD_PIN PIN_VBUS
#   define VBUS_AD_CHANNEL AD7
#   define VBUS_VOLTAGE_FAILTIME_MS 20
#   define VBUS_VOLTAGE_RETURNTIME_MS 1500
#   define VBUS_ADC_SAMPLE_FREQ 10000 // ADC sampling frequency in Hz

#   ifndef BI_STABLE
#      define VBUS_THRESHOLD_FAILED 24000 // millivoltage for normal relays
#      define VBUS_THRESHOLD_RETURN 26000

       // TODO test these values with 1% resistors for the voltage divider R3 & R12
// #      define VBUS_THRESHOLD_FAILED 23500 // millivoltage for normal relays
// #      define VBUS_THRESHOLD_RETURN 24500
#   else
#      define VBUS_THRESHOLD_FAILED 25500 // some relays don't work reliable below 24V@15ms,
                                          // datasheet states 19.2V @ 50ms should work for Hongfa HFE20-1 24-1HSD-L2(359), but not with 15ms!
#      define VBUS_THRESHOLD_RETURN 27500

       // TODO test these values with 1% resistors for the voltage divider R3 & R12
// #      define VBUS_THRESHOLD_FAILED 24500 // some relays don't work reliable below 24V@15ms,
                                          // datasheet states 19.2V @ 50ms should work for Hongfa HFE20-1 24-1HSD-L2(359), but not with 15ms!
// #      define VBUS_THRESHOLD_RETURN 25500
#   endif
#endif

/*
 *  AD-pin used for "random" app start-up delay
 */
#define FLOATING_AD_PIN PIN_IO12
#define FLOATING_AD_CHANNEL AD5



/*
 * below defines normally shouldn't need any changes
 */
#ifndef DEBUG   // make sure this wont be in a release
#   undef IO_TEST
#   undef DEBUG_SERIAL
#endif

#ifdef OUT8
#   define NO_OF_CHANNELS 8
#else
#   error "symbol OUT8 not defined!" // may be where is a use-case sometimes for an OUT4/OUT2/OUT1
#   define NO_OF_CHANNELS 8
#endif

#ifdef ZERO_DETECT
#   warning "ZERO-DETECT is untested."
#endif

#if defined(ZERO_DETECT) && defined(BI_STABLE)
#   error "ZERO-DETECT and BI_STABLE can not be used together.")
#endif

#ifdef BI_STABLE
#   define NO_OF_OUTPUTS (NO_OF_CHANNELS * 2)
#   define BETWEEN_CHANNEL_DELAY_MS 100 // pause in ms between to channels relais switching, to avoid bus drainage
#else
#   define NO_OF_OUTPUTS (NO_OF_CHANNELS)
#   define BETWEEN_CHANNEL_DELAY_MS 100 // pause in ms between to channels relais switching, to avoid bus drainage
#endif

/*
 *  output pins configuration
 */

#ifdef OUT8
#    ifdef BI_STABLE
        // 4TE-ARM Controller + out8_16A (bi-stable relays)
        // PCB: https://github.com/selfbus/hardware/tree/master/Apps/out8_16A_1.0
        // Wiki: https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/8fach%20Bin%C3%A4rausgang%20230V%2016A%20f%C3%BCr%20LPC1115%20Controller
        const int outputPins[NO_OF_OUTPUTS] =
        {
            /*
             * PIO_SDA=PIO0_5 PIO0_5 is a open-drain pin & needs a pull-up resistor
             * see LPC user manual page 79 chapter 7.4.12 IOCON_PIO0_5.
             * better solution would be to use IO7 instead.
             */

            // K1 -> K4
            PIN_IO2,  PIN_IO3,  //  1,  2 => K1 reset/set
            PIN_IO5,  PIO_SDA,  //  3,  4 => K2 reset/set // PIO_SDA-> pull-up resistor!
            PIN_PWM,  PIN_APRG, //  5,  6 => K3 reset/set
            PIN_IO1,  PIN_IO4,  //  7,  8 => K4 reset/set

            // K8 -> K5
            PIN_TX,   PIN_IO11, // 15, 16 => K8 reset/set
            PIN_IO10, PIN_RX,   // 13, 14 => K7 reset/set
            PIN_IO14, PIN_IO15, // 11, 12 => K6 reset/set
            PIN_IO9,  PIN_IO13  //  9, 10 => K5 reset/set
        };

#       if 0
            // moved from app_main.cpp to resolve merge conflict from oltarion commit "added Hardware Pin descriptions, old pin assignments were not deleted 5787a7b3ed10097856df5ba29c415c7f039d5a85"
            // Pinbelegung für out8_16A-V2.3 (bistabile Relais, separate Kanäle ohne Zusammenlegung von Kontakten)
            const int outputPins[NO_OF_OUTPUTS] =
            {// RESET Pin,SET Pin
                PIN_IO5,  PIN_IO7,  // Kanal 1
                PIN_IO4,  PIN_IO3,  // Kanal 2
                PIN_PWM,  PIN_APRG, // Kanal 3
                PIN_IO2,  PIN_IO1,  // Kanal 4

                PIN_IO10, PIN_RX,   // Kanal 5
                PIN_TX,   PIN_IO11, // Kanal 6
                PIN_IO14, PIN_IO15, // Kanal 7
                PIN_IO9,  PIN_IO13  // Kanal 8
            };
#       endif

#       if 0
            // moved from app_main.cpp to resolve merge conflict from oltarion commit "added Hardware Pin descriptions, old pin assignments were not deleted 5787a7b3ed10097856df5ba29c415c7f039d5a85"
            // Pinbelegung für out8_16A-V2.2 (bistabile Relais, Eingänge von jeweils 2 Kanälen zusammen)
            const int outputPins[NO_OF_OUTPUTS] =
            {// RESET Pin,SET Pin
                PIN_IO1,  PIN_IO4,  // Kanal 1
                PIN_PWM,  PIN_APRG, // Kanal 2
                PIN_IO5,  PIN_IO7,  // Kanal 3
                PIN_IO2,  PIN_IO3,  // Kanal 4
                PIN_TX,   PIN_IO11, // Kanal 5
                PIN_IO10, PIN_RX,   // Kanal 6
                PIN_IO14, PIN_IO15, // Kanal 7
                PIN_IO9,  PIN_IO13  // Kanal 8
            };
#       endif

        /*
         * DONE check which bi-stable configuration this should be?
         * looks like a try to implement INVERT functionality
         * which isn't really needed, cause invert can be done via userEeprom[APP_CLOSER_MODE] (ETS)
         * and initialization of relays void Outputs::begin (unsigned int initial, unsigned int inverted)
         */
#       if 0
            const int outputPins[NO_OF_OUTPUTS] =
            {
                PIN_IO3,  PIN_IO2,  //  1,  2 K1 inverted
                PIN_IO12, PIN_IO5,  //  3,  4 K2 inverted, but PIN_IO12 goes to nowhere, maybe it should be PIO_SDA ?
                PIN_APRG, PIN_PWM,  //  5,  6 K3 inverted
                PIN_IO4,  PIN_IO1,  //  7,  8 K4 inverted

                PIN_IO13, PIN_IO9,  //  9, 10 K5 inverted
                PIN_IO15, PIN_IO14, // 11, 12 K6 inverted
                PIN_RX,   PIN_IO10, // 13, 14 K7 inverted
                PIN_IO11, PIN_TX    // 15, 16 K8 inverted
            };
#       endif // INVERT
#   else
        // 4TE-ARM Controller + out8_16A (non-stable relays)
        // PCB: https://github.com/selfbus/hardware/tree/master/Apps/relais8_4te
        // Wiki: https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/Bin%C3%A4rausgang_8x230_4TE
        const int outputPins[NO_OF_OUTPUTS] =
        {
            PIN_IO1, // REL1
            PIN_IO2, // REL2
            PIN_IO3, // REL3
            PIN_IO4, // REL4
            PIN_IO5, // REL5
            PIN_IO6, // REL6
            PIN_IO7, // REL7
            PIN_IO8  // REL8
        };
#   endif // BI_STABLE

#else
#   error "symbol OUT8 not defined!" // maybe where is a use-case sometimes for an OUT4/OUT2/OUT1
#endif // OUT8


/*
 * KNX/ETS-specific defines
 */
#define MANUFACTURER 0x04   // We are a "Jung 2138.10" device, version 0.1
#define DEVICETYPE 0x2060
#define APPVERSION 1

#endif /* CONFIG_H_ */
