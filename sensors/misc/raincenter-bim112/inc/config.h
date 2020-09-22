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

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short baseAddress; //!> Base address of the config parameters
    byte hardwareVersion[6];    //!> The hardware identification number
} HardwareVersion;

extern const HardwareVersion * currentVersion;

// Hardware version. Must match the product_serial_number in the VD's table hw_product
const HardwareVersion hardwareVersion[] =
{ {4, 0x4578, { 0, 0, 0, 0, 0x0, 0x29 }}
, {8, 0x46B8, { 0, 0, 0, 0, 0x0, 0x28 }}
};

#define MANUFACTURER 0x83       // MDT
#define HARDWARE_ID 0

#define LED_STARTUP_DISPLAY 1000

#define RAINCENTER_BAUDRATE 2400
#define SENDPERIODIC_INTERVAL_MS 60000 // TODO send period every 60s can't be the final solution, needs to be configured via ETS
#define POLL_INTERVAL_MS 5000
#define REPOLL_INTERVAL_MS 1500


#define RAINCENTER_TX_DELAY_SEND_MS 100 // time in ms the raincenter doesnt react to serial commands after it send a response
#define TX_FLASH_MS 50 // time in ms TX-Led is flashing
#define RX_FLASH_MS 50 // time in ms RX-Led is flashing
#define RX_FLASH_OK_MS RX_FLASH_MS + 1000 // time in ms RX-Led is active to display correct received values


/* possible IOs for serial communication
    PIO1_6 ISP_RX
    PIO1_7 ISP_TX

    PIO2_7 EXT10 RX
    PIO2_8 EXT9 TX

    PIO3_0 IO6 TX
    PIO3_1 IO7 RX
*/

#if defined(DEMOBOARD_OM13087)
    // LEDs
    #define BLUE_LED PIO0_9  // DEMOBOARD_OM13087 Onboard Blue-Led (Pin5)
    #define RX_LED PIO0_8    // DEMOBOARD_OM13087 Onboard Green-Led (Pin6)
    #define TX_LED PIO0_7    // DEMOBOARD_OM13087 Onboard Red-Led (Pin11)
    //serial port
    #define RC_TX_PIN PIN_IO14 // DEMOBOARD_OM13087 Pin9 (PIO1_7)
    #define RC_RX_PIN PIN_IO15 // DEMOBOARD_OM13087 Pin10 (PIO1_6)
#elif defined(TS_ARM)
    // LEDs
    #define RX_LED PIN_IO1 // TS_ARM IO1 PAD (PIO2_2)
    #define TX_LED PIN_IO7 // TS_ARM IO2 PAD (PIO0_9)
    // serial port
    #define RC_TX_PIN PIN_TX // TS_ARM IO6 PAD (PIO3_0)
    #define RC_RX_PIN PIN_RX // TS_ARM IO7 PAD (PIO3_1)
#else // for 4TE ARM Controller
    // LEDs
    #define RX_LED PIN_INFO    // PIO2_6
    #define TX_LED PIN_RUN     // PIO3_3
    // serial port
    #define RC_TX_PIN PIN_IO14 // PIO1_7
    #define RC_RX_PIN PIN_IO15 // PIO1_6
#endif

#if defined(DEMOBOARD_OM13087)
    #define LED_ON 0
    #define LED_OFF 1
#elif defined(TS_ARM)
    #define LED_ON 0
    #define LED_OFF 1
#else
    #define LED_ON 1
    #define LED_OFF 0
#endif

#endif /* CONFIG_H_ */
