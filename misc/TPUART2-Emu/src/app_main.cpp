/*
 *  app_main.cpp - Selfbus TP-UART 2 emulator for the LPC1115.
 *
 *  Turns an SB-Interface (LPC1115) into a KNX TP1 transceiver that speaks the
 *  TP-UART host protocol on its UART, so it can be driven by knxd, the OpenKNX
 *  stack or any other TP-UART capable host.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include <sblib/serial.h>
#include <sblib/timer.h>
#include <sblib/version.h>

#include "bcu_tpuart.h"
#include "config.h"
#include "tpuart_emulator.h"

APP_VERSION("SBtpuart", "0", "01") // Don't forget to also change the build-variable sw_version

BcuTpUart bcuTpUart = BcuTpUart();      //!< the bus coupling unit
TpUartEmulator tpUart(bcuTpUart);       //!< the TP-UART host protocol emulation

/**
 * Initialize the application.
 */
BcuBase* setup()
{
    pinMode(LED_KNX_RX, OUTPUT);
    digitalWrite(LED_KNX_RX, LED_ON);
    delay(LED_TEST_MS);
    pinMode(LED_SERIAL_RX, OUTPUT);
    digitalWrite(LED_SERIAL_RX, LED_ON);
    delay(LED_TEST_MS);
    digitalWrite(LED_KNX_RX, LED_OFF);
    delay(LED_TEST_MS);
    digitalWrite(LED_SERIAL_RX, LED_OFF);

    bcuTpUart.begin(); // userRam->status() is set in BcuTpUart::begin()
    bcuTpUart.setOwnAddress(TPUART_OWN_KNX_ADDRESS);

    serial.setRxPin(PIN_TPUART_SERIAL_RX);
    serial.setTxPin(PIN_TPUART_SERIAL_TX);
    pinMode(PIN_TPUART_SERIAL_RX, SERIAL_RXD | PULL_UP | HYSTERESIS);
    serial.begin(TPUART_BAUDRATE, SERIAL_8E1);
    serial.clearBuffers();

    tpUart.begin();
    return (&bcuTpUart);
}

/**
 * The main processing loop.
 */
void loop()
{
    tpUart.loop();
}

/**
 * The processing loop while no KNX application is loaded.
 */
void loop_noapp()
{
    loop();
}
