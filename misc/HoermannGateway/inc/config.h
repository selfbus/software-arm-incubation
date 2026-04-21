/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef SB_GARAGE_DOOR_GATEWAY_CONFIG_H_
#define SB_GARAGE_DOOR_GATEWAY_CONFIG_H_

#include <sblib/ioports.h>
#include <cstdint>

#define PIO_BREAK_TRIGGER PIO3_2
#define PIO_DRIVER_ENABLE 0      //!< disable driver-enable for debugging
//#define PIO_RS485_TX      PIO1_7 //!< rs232 Tx pin on swd for debugging
//#define PIO_RS485_RX      PIO1_6 //!< rs232 Rx pin on swd for debugging

//#define PIO_DRIVER_ENABLE PIO1_5 //!< RTS pin for driver_enable (DE)
#define PIO_RS485_TX PIO3_0      //!< RS485 transmit pin
#define PIO_RS485_RX PIO3_1      //!< RS485 receive pin

#define PIO_LED_1 PIO1_1  //!< Digital pin for LED 1
#define PIO_LED_2 PIO2_11 //!< Digital pin for LED 2
constexpr uint32_t debugLeds[] = { PIO_LED_1, PIO_LED_2 };

/**
 * SoftUART Tx pin (for debugging only).
 */
//#define PIO_SOFT_UART_TX PIO1_2
#define PIO_SOFT_UART_TX PIO1_7 //!< rs232 Tx pin on swd for debugging

/**
 * SoftUART Rx pin (for debugging only).
 * @note Make sure that the corresponding PIOINTx_IRQHandler calls
 *       softUART.handleGpioInterrupt().
 */
#define PIO_SOFT_UART_RX PIO2_3

#endif /* SB_GARAGE_DOOR_GATEWAY_CONFIG_H_ */
