/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef USB_IF_KNXF_CONFIG_H_
#define USB_IF_KNXF_CONFIG_H_

#include <stdint.h>
#include <sblib/digital_pin.h>

/**
 * @def USE_ISP_UART
 * Comment out, to change UART to 2x10 pole connecter P4
 * @note Needs soldering of Tx/Rx to pins on P4
 */
#define USE_ISP_UART_PINS

#if defined(USE_ISP_UART_PINS)
    constexpr uint32_t PinSerialRx = PIO1_6; // Pin 3 of JP7 (ISP header)
    constexpr uint32_t PinSerialTx = PIO1_7; // Pin 5 of JP7 (ISP header)
#else
    constexpr uint32_t PinSerialRx = PIO3_1; // Pin 17 of P4 (IO7 of TS_ARM)
    constexpr uint32_t PinSerialTx = PIO3_0; // Pin 15 of P4 (IO6 of TS_ARM)
#endif



#endif /* USB_IF_KNXF_CONFIG_H_ */
