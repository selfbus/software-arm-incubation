/*
 *  config.h
 *
 *  Created on: 11.09.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/io_pin_names.h>
#include <sblib/digital_pin.h>

/* possible IOs for serial communication
    PIO1_6 ISP_RX
    PIO1_7 ISP_TX

    PIO2_7 EXT10 RX
    PIO2_8 EXT9 TX

    PIO3_0 IO6 TX
    PIO3_1 IO7 RX
*/

#ifdef DEMOBOARD_OM13087
    // LEDs
    #define BLUE_LED PIO0_9  // DEMOBOARD_OM13087 Onboard Blue-Led (Pin5)
    #define RX_LED PIO0_8    // DEMOBOARD_OM13087 Onboard Green-Led (Pin6)
    #define TX_LED PIO0_7    // DEMOBOARD_OM13087 Onboard Red-Led (Pin11)
    //serial port
    // #define RC_TX_PIN PIN_IO14 // DEMOBOARD_OM13087 Pin9 (PIO1_7)
    // #define RC_RX_PIN PIN_IO15 // DEMOBOARD_OM13087 Pin10 (PIO1_6)
#else
    #ifdef TS_ARM
        // LEDs
        #define RX_LED PIN_IO1 // TS_ARM IO1 PAD (PIO2_2)
        #define TX_LED PIN_IO7 // TS_ARM IO2 PAD (PIO0_9)
        // serial port
        #define RC_TX_PIN PIN_TX // TS_ARM IO6 PAD (PIO3_0)
        #define RC_RX_PIN PIN_RX // TS_ARM IO7 PAD (PIO3_1)
    #else
        // LEDs
        #define RX_LED PIN_INFO    // PIO2_6
        #define TX_LED PIN_RUN     // PIO3_3
        // serial port
        #define RC_TX_PIN PIN_IO14 // PIO1_7
        #define RC_RX_PIN PIN_IO15 // PIO1_6
    #endif
#endif

#define TX_FLASH_MS 100
#define RX_FLASH_MS 200

#define LED_ON 0
#define LED_OFF 1

#define POLL_INTERVAL_MS 3000
#define RAINCENTER_BAUDRATE 2400


#endif /* CONFIG_H_ */
