/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.
 ---------------------------------------------------------------------------*/

#ifndef FT12_CONFIG_H_
#define FT12_CONFIG_H_

#define LED_SERIAL_RX               (PIO0_6) //!< Serial-Rx LED Pin
#define LED_KNX_RX                  (PIO0_7) //!< KNX-Rx LED Pin
#define LED_ON                      (false)  //!< Led is turned on by setting the IO-pin to low/ground
#define LED_OFF                     (true)   //!< Led is turned off by setting the IO-pin to high/+3V3
#define LED_TEST_MS                 (250)    //!< Time in milliseconds every led should light up on startup test

#define PIN_FT_SERIAL_TX            (PIN_TX) //!< Serial-Tx Pin
#define PIN_FT_SERIAL_RX            (PIN_RX) //!< Serial-Rx Pin

#define LED_KNX_RX_BLINKTIME        (100)    //!< Receiving KNX packets blinking timeout in milliseconds

#define FT_OWN_KNX_ADDRESS          (0x11fe) //!< Our own knx-address: 1.1.254
#define FT_FRAME_SIZE               (32)     //!< Maximum size of FT1.2 frames
#define FT_MAX_SEND_RETRY           (0)      //!< Do not repeat sending
#define FT_BAUDRATE                 (19200)  //!< Ft12 baudrate



#endif /* FT12_CONFIG_H_ */
