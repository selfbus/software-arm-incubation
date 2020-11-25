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

#define NO_OF_HAND_PINS 8 // FIXME get rid of this

// #define BI_STABLE // replaced by build-variable ${relay_type}
// #define HAND_ACTUATION // replaced by build-variable ${hand_actuation}

// #define ZERO_DETECT // no more supported? at least it's mentioned in a comment from 2014
                       // https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/Bin%C3%A4rausgang_8x230_16A_4TE

/*
 *  bus power-failure configuration
 */
#ifdef BUSFAIL
#   define VBUS_AD_CHANNEL AD7
#   define VBUS_THRESHOLD 1.94 // TODO 1.94V @ ADC-Pin of the LPC11xx, 1.94V is just selected for fast testing, needs further investigation
                               // depend's on used controller e.g.
                               // 4TE-ARM the voltage divider is R3/R12 (91K0 & 10K0)
                               // TS_ARM  the voltage divider is R3/R12 (91K0 & 10K0)
#endif





/*
 * below defines normally shouldn't need any changes
 */
#ifndef DEBUG
#   undef IO_TEST // make sure this wont be in a release
#endif

#ifdef OUT8
#   define NO_OF_CHANNELS 8
#else
#   error "symbol OUT8 not defined!" // may be where is a use-case sometimes for an OUT4/OUT2/OUT1
#   define NO_OF_CHANNELS 8
#endif

#ifdef ZERO_DETECT
#   pragma message("ZERO-DETECT is untested.")
#endif

#ifdef BI_STABLE
#   define NO_OF_OUTPUTS (NO_OF_CHANNELS * 2)
#else
#   define NO_OF_OUTPUTS (NO_OF_CHANNELS)
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
         * TODO check which bi-stable configuration this should be?
         * looks like a try to implement INVERT functionality
         * which isn't really needed, cause invert can be done via userEeprom[APP_CLOSER_MODE] (ETS)
         * and initialisation of relays void Outputs::begin (unsigned int initial, unsigned int inverted)
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

/* this is used no where. Why its here ?
extern "C" const char APP_VERSION[13] = "O08.10  1.00";

const char * getAppVersion()
{
    return APP_VERSION;
}

// volatile const char * v = getAppVersion();
*/


#endif /* CONFIG_H_ */
