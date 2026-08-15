/*
 *  config.h - Build time configuration of the TP-UART emulator.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef TPUART_CONFIG_H_
#define TPUART_CONFIG_H_

#include <sblib/io_pin_names.h>

/** Serial-Rx LED pin, lit while the host talks to us. */
#define LED_SERIAL_RX               (PIO0_6)
/** KNX-Rx LED pin, lit while telegrams are received from the bus. */
#define LED_KNX_RX                  (PIO0_7)
/** LED is turned on by driving the pin low. */
#define LED_ON                      (false)
/** LED is turned off by driving the pin high. */
#define LED_OFF                     (true)
/** Duration of the LED startup test in milliseconds. */
#define LED_TEST_MS                 (250)
/** How long a LED stays lit after activity, in milliseconds. */
#define LED_BLINK_MS                (50)

/** Serial Tx pin towards the host. */
#define PIN_TPUART_SERIAL_TX        (PIO1_7)
/** Serial Rx pin towards the host. */
#define PIN_TPUART_SERIAL_RX        (PIO1_6)
/** Host interface baud rate. A real TP-UART 2 runs at 19200 baud, 8E1. */
#define TPUART_BAUDRATE             (19200)

/**
 * Physical address used as sender address for frames that the host sends
 * without a valid source address. It is overwritten with the source address
 * of every frame the host transmits, and by @ref U_SET_ADDRESS_REQ.
 */
#define TPUART_OWN_KNX_ADDRESS      (0xFFFF)

/**
 * Delay between two bytes sent to the host, in milliseconds.
 *
 * A real TP-UART forwards received octets at KNX TP1 speed (about 1.4 ms per
 * octet), so a host never sees a burst. sblib on the other hand hands over a
 * complete telegram at once. Pacing the output keeps the emulator compatible
 * with hosts that assume TP1 timing, most notably the OpenKNX stack whose
 * OVERRUN_COUNT check discards frames when more than 7 octets are already
 * buffered when a frame starts.
 *
 * Set to 0 to send without pacing (higher throughput, needs a host that
 * tolerates bursts, e.g. knxd, or OpenKNX built with -DOVERRUN_COUNT=64).
 */
#define TPUART_TX_PACING_MS         (1)

/**
 * Time in milliseconds after which an unconfirmed bus transmission is
 * reported to the host as a negative L_Data.con.
 *
 * Must stay below the confirmation timeout of the host. The OpenKNX stack
 * uses 500 ms (CONFIRM_TIMEOUT), knxd retries after 2 s.
 */
#define TPUART_TX_CONFIRM_TIMEOUT_MS (400)

/** Maximum size of a KNX frame including its checksum octet. */
#define TPUART_MAX_FRAME_SIZE       (64)

/** Size of the ring buffer towards the host. Must be a power of two. */
#define TPUART_TX_QUEUE_SIZE        (256)

#endif /* TPUART_CONFIG_H_ */
