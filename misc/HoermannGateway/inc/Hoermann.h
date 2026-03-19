/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef HOERMANN_H_
#define HOERMANN_H_

#include "CRC.h"
#include "HoermannState.h"
#include <sblib/types.h>


class Hoermann {
public:
    Hoermann() = default;

    /**
     * Initializes the serial interface for RS485 communication with the Hoermann drive.
     *
     * Configures the UART peripheral at 19200 baud (8N1), assigns the TX and RX pins,
     * sets up the RTS pin for hardware RS485 driver enable (active-low DE), and enables
     * RS485 direction control via the UART hardware.
     *
     * @param pinTx  Pin to use as UART transmit (TX).
     * @param pinRx  Pin to use as UART receive (RX), configured with pull-up and hysteresis.
     * @param pinRTS Pin to use as RS485 direction control (RTS / DE), driven low during transmission.
     */
    static void begin(uint32_t pinTx, uint32_t pinRx, uint32_t pinRTS);

    void loop();
    void open();
    void close();
    void stop();
    void venting();
    void light(bool state);
    void emergencyStop(bool state);

    HoermannState state;

protected:
    uint8_t slaveResponse[2] = { 0x00, 0x10 };
    int stateMachine = 0;
    int address = 0;
    int counter = 0;
    int length = 0;
    int position = 0;
    byte data[15];
    uint8_t myCounter = 0;
    CRC crc;

    void sendResponse(uint8_t addr, uint8_t bytes[], uint8_t len);
};

#endif /* HOERMANN_H_ */
