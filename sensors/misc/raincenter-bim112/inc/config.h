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

/*
 *
 * Raincenter objects for testing
 * JAL-410.01 Kanal A Rolladen
 * Object   Name                        Function
 * 13       Rolladen Auf/Ab             TapWaterRefill (on/off)
 * 15       Stop                        TapWaterExchangeActive (on/off)
 * 17       Status aktuelle Richtung    TapWaterRefill Status (on/off)
 *
 * 20       Status aktuelle Position    Calibrated Fill Level in m3 * 10 (43 means 4.3m³)
 * 22       Akt. Position gueltig       Alarm (on/off)
 * 23       Referenzfahrt starten       Pump active (on/off)
 * 25       Status obere Position       AutomaticTapwaterRefill Status (on/off)
 * 26       Status untere Position      ManualTapwaterRefill Status (on/off)
 *
 */

#define BLINKENLIGHTS //!< enables Tx/Rx leds

#define OBJ_TAPWATER_REFILL 13
#define OBJ_TAPWATER_REFILL_STATUS 17
#define OBJ_TAPWATER_EXCHANGE_STATUS 15
#define OBJ_FILL_LEVEL_CALIBRATED_m3 20
#define OBJ_ALARM_STATUS 22
#define OBJ_PUMP_STATUS 23
#define OBJ_TAPWATER_REFILL_AUTOMATIC_STATUS 25
#define OBJ_TAPWATER_REFILL_MANUAL_STATUS 26

#define MANUFACTURER 0x83 //!< Manufacturer MDT
#define DEVICETYPE   0x29 //!< Device Type JAL-0410.01 Shutter Actuator 4-fold, 4TE, 230VAC, 10A
#define APPVERSION   0x28 //!< Application Version
const unsigned char hardwareVersion[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x29}; //!< The hardware identification number

#define LED_STARTUP_DISPLAY 1000

#define RAINCENTER_BAUDRATE 2400
#define SENDPERIODIC_INTERVAL_MS 60000 // TODO send period every 60s can't be the final solution, needs to be configured via ETS
#define POLL_INTERVAL_MS 2000
#define REPOLL_INTERVAL_MS 1500


#define RAINCENTER_TX_DELAY_SEND_MS 100 // time in ms the raincenter doesn't react to serial commands after it send a response
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

#if defined(TS_ARM)
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

#if defined(TS_ARM)
    #define LED_ON 0
    #define LED_OFF 1
#else
    #define LED_ON 1
    #define LED_OFF 0
#endif

#endif /* CONFIG_H_ */
