/*
 *  ioports.h - Definition of the I/O ports and port pins.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_ioports_h
#define sblib_ioports_h

/**
 * Digital ports.
 */
enum Port
{
    // Port 1
	PIO0 = 0,

	// Port 1
	PIO1 = 1,

	// Port 2
	PIO2 = 2,

	// Port 3
	PIO3 = 3
};


/**
 * Constants for port pin functions
 */
enum PinFunc
{
    PF_NONE = 0, PF_PIO, PF_AD, PF_RESET, PF_SWDIO, PF_SWCLK, PF_MAT, PF_CAP,
    PF_CLKOUT, PF_SDA, PF_SCL, PF_SSEL, PF_MISO, PF_MOSI, PF_SCK, PF_RXD,
    PF_TXD, PF_RTS, PF_DTR, PF_DSR, PF_CTS, PF_DCD, PF_RI,
    PF_USBP, PF_USB_M, PF_USB_VBUS, PF_USB_CONNECT, PF_USB_FTOGGLE,
    PF_ARM_TRACE_CLK, PF_ARM_TRACE_SWV,
    PF_SCLK
};


/**
 * Select a specific port pin function when setting the pin mode.
 * This macro is intended to be used in combination with pinMode().
 *
 * Example: pinMode(PIO1_6, INPUT | PINMODE_FUNC(PF_RXD));
 * This enables the UART RXD function on pin PIO1_6.
 */
#define PINMODE_FUNC(f) ((f) << 18)


// Constants for port pin function manipulation
enum
{
    PFL_ADMODE = 0x100,

    PFF_SHIFT_OFFSET = 5,
    PFF_MASK = (1 << PFF_SHIFT_OFFSET) - 1,

    PF0_SHIFT = 9,
    PF1_SHIFT = PF0_SHIFT + PFF_SHIFT_OFFSET,
    PF2_SHIFT = PF1_SHIFT + PFF_SHIFT_OFFSET,
    PF3_SHIFT = PF2_SHIFT + PFF_SHIFT_OFFSET
};

#define FUNC0(x) ((x) << PF0_SHIFT)
#define FUNC1(x) ((x) << PF1_SHIFT)
#define FUNC2(x) ((x) << PF2_SHIFT)
#define FUNC3(x) ((x) << PF3_SHIFT)

#if defined (__LPC11XX__)
/**
 * Port pins.
 *
 * The bits of the constants are used as follows:
 * Bit 0-4:   pin (0..31)
 * Bit 5-6:   port (0..3)
 * Bit 8:     pin has A/D mode selection bit
 * Bit 9-14:  function 0: none, pio, reset, sw-debug (swdio/swclk)
 * Bit 15-19: function 1: none, pio, A/D, timer match, timer capture, clkout,
 *                        I2C (sda/scl), SPI (ssel, miso, mosi, sck),
 *                        SERIAL (rxd, txd, rts, dtr, dsr, cts, dcd, ri)
 * Bit 20-24: function 2: none, A/D, timer match, timer capture,
 *                        SPI (ssel, miso, mosi, sck), SERIAL (rxd, txd)
 * Bit 25-29: function 3: timer match, timer capture, SERIAL (rxd, txd)
 */
enum PortPin
{
    // Port 0 pin 0
    PIO0_0 = 0x00 | FUNC0(PF_RESET) | FUNC1(PF_PIO),

    // Port 0 pin 1
    PIO0_1 = 0x01 | FUNC0(PF_PIO) | FUNC1(PF_CLKOUT) | FUNC2(PF_MAT),

    // Port 0 pin 2
    PIO0_2 = 0x02 | FUNC0(PF_PIO) | FUNC1(PF_SSEL) | FUNC2(PF_CAP),

    // Port 0 pin 3
    PIO0_3 = 0x03 | FUNC0(PF_PIO),

    // Port 0 pin 4
    PIO0_4 = 0x04 | FUNC0(PF_PIO) | FUNC1(PF_SCL),

    // Port 0 pin 5
    PIO0_5 = 0x05 | FUNC0(PF_PIO) | FUNC1(PF_SDA),

    // Port 0 pin 6
    PIO0_6 = 0x06 | FUNC0(PF_PIO) | FUNC1(PF_NONE) | FUNC2(PF_SCK),

    // Port 0 pin 7
    PIO0_7 = 0x07 | FUNC0(PF_PIO) | FUNC1(PF_CTS),

    // Port 0 pin 8
    PIO0_8 = 0x08 | FUNC0(PF_PIO) | FUNC1(PF_MISO) | FUNC2(PF_MAT),

    // Port 0 pin 9
    PIO0_9 = 0x09 | FUNC0(PF_PIO) | FUNC1(PF_MOSI) | FUNC2(PF_MAT),

    // Port 0 pin 10
    PIO0_10 = 0x0a | FUNC0(PF_SWCLK) | FUNC1(PF_PIO) | FUNC2(PF_SCK) | FUNC3(PF_MAT),

    // Port 0 pin 11
    PIO0_11 = 0x0b | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,


    // Port 1 pin 0
    PIO1_0 = 0x20 | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_CAP) | PFL_ADMODE,

    // Port 1 pin 1
    PIO1_1 = 0x21 | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 1 pin 2
    PIO1_2 = 0x22 | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 1 pin 3
    PIO1_3 = 0x23 | FUNC0(PF_SWDIO) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 1 pin 4
    PIO1_4 = 0x24 | FUNC0(PF_PIO) | FUNC1(PF_AD) | FUNC2(PF_MAT) | PFL_ADMODE,

    // Port 1 pin 5
    PIO1_5 = 0x25 | FUNC0(PF_PIO) | FUNC1(PF_RTS) | FUNC2(PF_CAP),

    // Port 1 pin 6
    PIO1_6 = 0x26 | FUNC0(PF_PIO) | FUNC1(PF_RXD) | FUNC2(PF_MAT),

    // Port 1 pin 7
    PIO1_7 = 0x27 | FUNC0(PF_PIO) | FUNC1(PF_TXD) | FUNC2(PF_MAT),

    // Port 1 pin 8
    PIO1_8 = 0x28 | FUNC0(PF_PIO) | FUNC1(PF_CAP),

    // Port 1 pin 9
    PIO1_9 = 0x29 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_MOSI),

    // Port 1 pin 10
    PIO1_10 = 0x2a | FUNC0(PF_PIO) | FUNC1(PF_AD) | FUNC2(PF_MAT) | FUNC3(PF_MISO) | PFL_ADMODE,

    // Port 1 pin 11
    PIO1_11 = 0x2b | FUNC0(PF_PIO) | FUNC1(PF_AD) | FUNC2(PF_CAP) | PFL_ADMODE,


    // Port 2 pin 0
    PIO2_0 = 0x40 | FUNC0(PF_PIO) | FUNC1(PF_DTR) | FUNC2(PF_SSEL),

    // Port 2 pin 1
    PIO2_1 = 0x41 | FUNC0(PF_PIO) | FUNC1(PF_DSR) | FUNC2(PF_SCK),

    // Port 2 pin 2
    PIO2_2 = 0x42 | FUNC0(PF_PIO) | FUNC1(PF_DCD) | FUNC2(PF_MISO),

    // Port 2 pin 3
    PIO2_3 = 0x43 | FUNC0(PF_PIO) | FUNC1(PF_RI) | FUNC2(PF_MOSI),

    // Port 2 pin 4
    PIO2_4 = 0x44 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_SSEL),

    // Port 2 pin 5
    PIO2_5 = 0x45 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 2 pin 6
    PIO2_6 = 0x46 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 2 pin 7
    PIO2_7 = 0x47 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_RXD),

    // Port 2 pin 8
    PIO2_8 = 0x48 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_TXD),

    // Port 2 pin 9
    PIO2_9 = 0x49 | FUNC0(PF_PIO) | FUNC1(PF_CAP),

    // Port 2 pin 10
    PIO2_10 = 0x4a | FUNC0(PF_PIO),

    // Port 2 pin 11
    PIO2_11 = 0x4b | FUNC0(PF_PIO) | FUNC1(PF_SCK) | FUNC2(PF_CAP),


    // Port 3 pin 0
    PIO3_0 = 0x60 | FUNC0(PF_PIO) | FUNC1(PF_DTR) | FUNC2(PF_MAT) | FUNC3(PF_TXD),

    // Port 3 pin 1
    PIO3_1 = 0x61 | FUNC0(PF_PIO) | FUNC1(PF_DSR) | FUNC2(PF_MAT) | FUNC3(PF_RXD),

    // Port 3 pin 2
    PIO3_2 = 0x62 | FUNC0(PF_PIO) | FUNC1(PF_DCD) | FUNC2(PF_MAT) | FUNC3(PF_SCK),

    // Port 3 pin 3
    PIO3_3 = 0x63 | FUNC0(PF_PIO) | FUNC1(PF_RI) | FUNC2(PF_CAP),

    // Port 3 pin 4
    PIO3_4 = 0x64 | FUNC0(PF_PIO) | FUNC1(PF_CAP) | FUNC2(PF_RXD),

    // Port 3 pin 5
    PIO3_5 = 0x65 | FUNC0(PF_PIO) | FUNC1(PF_CAP) | FUNC2(PF_TXD)
};
#elif defined (__LPC11UXX__)
enum PortPin
{
    // Port 0 pin 0
    PIO0_0 = 0x00 | FUNC0(PF_RESET) | FUNC1(PF_PIO),
    REST_PIO0_0 = 0x00 | FUNC0(PF_RESET) | FUNC1(PF_PIO),

    // Port 0 pin 1
    PIO0_1 = 0x01 | FUNC0(PF_PIO) | FUNC1(PF_CLKOUT) | FUNC2(PF_MAT) | FUNC3(PF_USB_FTOGGLE),

    // Port 0 pin 2
    PIO0_2 = 0x02 | FUNC0(PF_PIO) | FUNC1(PF_SSEL) | FUNC2(PF_CAP),

    // Port 0 pin 3
    PIO0_3 = 0x03 | FUNC0(PF_PIO) | FUNC1(PF_USB_VBUS),

    // Port 0 pin 4
    PIO0_4 = 0x04 | FUNC0(PF_PIO) | FUNC1(PF_SCL),

    // Port 0 pin 5
    PIO0_5 = 0x05 | FUNC0(PF_PIO) | FUNC1(PF_SDA),

    // Port 0 pin 6
    PIO0_6 = 0x06 | FUNC0(PF_PIO) | FUNC1(PF_USB_CONNECT) | FUNC2(PF_SCK),

    // Port 0 pin 7
    PIO0_7 = 0x07 | FUNC0(PF_PIO) | FUNC1(PF_CTS),

    // Port 0 pin 8
    PIO0_8 = 0x08 | FUNC0(PF_PIO) | FUNC1(PF_MISO) | FUNC2(PF_MAT) | FUNC3(PF_ARM_TRACE_CLK),

    // Port 0 pin 9
    PIO0_9 = 0x09 | FUNC0(PF_PIO) | FUNC1(PF_MOSI) | FUNC2(PF_MAT) | FUNC3(PF_ARM_TRACE_SWV),

    // Port 0 pin 10
    PIO0_10 = 0x0a | FUNC0(PF_SWCLK) | FUNC1(PF_PIO) | FUNC2(PF_SCK) | FUNC3(PF_MAT),

    // Port 0 pin 11
    PIO0_11 = 0x0b | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 0 pin 12
    PIO0_12 = 0x0c | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_CAP) | PFL_ADMODE,

    // Port 0 pin 13
    PIO0_13 = 0x0d | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 0 pin 14
    PIO0_14 = 0x0e | FUNC0(PF_NONE) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 0 pin 15
    PIO0_15 = 0x0f | FUNC0(PF_SWDIO) | FUNC1(PF_PIO) | FUNC2(PF_AD) | FUNC3(PF_MAT) | PFL_ADMODE,

    // Port 0 pin 16
    PIO0_16 = 0x10 | FUNC0(PF_PIO) | FUNC1(PF_AD) | FUNC2(PF_MAT) | PFL_ADMODE,

    // Port 0 pin 17
    PIO0_17 = 0x11 | FUNC0(PF_PIO) | FUNC1(PF_RTS) | FUNC2(PF_CAP) | FUNC3(PF_SCLK),

    // Port 0 pin 18
    PIO0_18 = 0x12 | FUNC0(PF_PIO) | FUNC1(PF_RXD) | FUNC2(PF_MAT),

    // Port 0 pin 19
    PIO0_19 = 0x13 | FUNC0(PF_PIO) | FUNC1(PF_TXD) | FUNC2(PF_MAT),

    // Port 0 pin 20
    PIO0_20 = 0x14 | FUNC0(PF_PIO) | FUNC1(PF_CAP),

    // Port 0 pin 21
    PIO0_21 = 0x15 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_MOSI),

    // Port 0 pin 2
    PIO0_22 = 0x16 | FUNC0(PF_PIO) | FUNC1(PF_AD) | FUNC2(PF_MAT) | FUNC3(PF_MISO) | PFL_ADMODE,

    // Port 0 pin 23
    PIO0_23 = 0x17 | FUNC0(PF_PIO) | FUNC1(PF_AD) |  PFL_ADMODE,


    // Port 1 pin 0
    PIO1_0 = 0x20 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 1
    PIO1_1 = 0x21 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 2
    PIO1_2 = 0x22 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 3
    PIO1_3 = 0x23 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 4
    PIO1_4 = 0x24 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 5
    PIO1_5 = 0x25 | FUNC0(PF_PIO) | FUNC1(PF_CAP),

    // Port 1 pin 7
    PIO1_7 = 0x27 | FUNC0(PF_PIO),

    // Port 1 pin 8
    PIO1_8 = 0x28 | FUNC0(PF_PIO),

    // Port 1 pin 10
    PIO1_10 = 0x2a | FUNC0(PF_PIO),

    // Port 1 pin 11
    PIO1_11 = 0x2b | FUNC0(PF_PIO),

    // Port 1 pin 13
    PIO1_13 = 0x2d | FUNC0(PF_PIO) | FUNC1(PF_DTR) | FUNC2(PF_MAT) | FUNC3(PF_TXD),

    // Port 1 pin 14
    PIO1_14 = 0x2e | FUNC0(PF_PIO) | FUNC1(PF_DSR) | FUNC2(PF_MAT) | FUNC3(PF_RXD),

    // Port 1 pin 15
    PIO1_15 = 0x2f | FUNC0(PF_PIO) | FUNC1(PF_DCD) | FUNC2(PF_MAT) | FUNC3(PF_SCK),

    // Port 1 pin 16
    PIO1_16 = 0x30 | FUNC0(PF_PIO) | FUNC1(PF_RI) | FUNC2(PF_CAP),

    // Port 1 pin 17
    PIO1_17 = 0x31 | FUNC0(PF_PIO) | FUNC1(PF_CAP) | FUNC2(PF_RXD),

    // Port 1 pin 18
    PIO1_18 = 0x32 | FUNC0(PF_PIO) | FUNC1(PF_CAP) | FUNC2(PF_TXD),


    // Port 1 pin 19
    PIO1_19 = 0x33 | FUNC0(PF_PIO) | FUNC1(PF_DTR) | FUNC2(PF_SSEL),

    // Port 1 pin 20
    PIO1_20 = 0x34 | FUNC0(PF_PIO) | FUNC1(PF_DSR) | FUNC2(PF_SCK),

    // Port 1 pin 21
    PIO1_21 = 0x35 | FUNC0(PF_PIO) | FUNC1(PF_DCD) | FUNC2(PF_MISO),

    // Port 1 pin 22
    PIO1_22 = 0x36 | FUNC0(PF_PIO) | FUNC1(PF_RI) | FUNC2(PF_MISO),

    // Port 1 pin 23
    PIO1_23 = 0x37 | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_SSEL),

    // Port 1 pin 24
    PIO1_24 = 0x38 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 25
    PIO1_25 = 0x39 | FUNC0(PF_PIO) | FUNC1(PF_MAT),

    // Port 1 pin 26
    PIO1_26 = 0x3a | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_RXD),

    // Port 1 pin 27
    PIO1_27 = 0x3b | FUNC0(PF_PIO) | FUNC1(PF_MAT) | FUNC2(PF_TXD),

    // Port 1 pin 28
    PIO1_28 = 0x3c | FUNC0(PF_PIO) | FUNC1(PF_CAP) | FUNC2(PF_SCK),

    // Port 1 pin 29
    PIO1_29 = 0x3d | FUNC0(PF_PIO) | FUNC1(PF_SCK) | FUNC2(PF_CAP),

    // Port 1 pin 31
    PIO1_31 = 0x3f | FUNC0(PF_PIO),
};
#endif

/**
 * Analog channels.
 */
enum AnalogChannel
{
	AD0 = 0,   //!< Analog channel 0 (this is pin PIO0_11)
	AD1 = 1,   //!< Analog channel 1 (this is pin PIO1_0)
	AD2 = 2,   //!< Analog channel 2 (this is pin PIO1_1)
	AD3 = 3,   //!< Analog channel 3 (this is pin PIO1_2)
	AD4 = 4,   //!< Analog channel 4 (this is pin PIO1_3)
	AD5 = 5,   //!< Analog channel 5 (this is pin PIO1_4)
	AD6 = 6,   //!< Analog channel 6 (this is pin PIO1_10)
	AD7 = 7    //!< Analog channel 7 (this is pin PIO1_11)
};

/**
 * Array for masked port pin access.
 */
extern const int portMask[12];

#undef FUNC0
#undef FUNC1
#undef FUNC2
#undef FUNC3

/**
 * Find the function number for the function of a pin.
 *
 * @param pin - the pin
 * @param func - the port function to find, e.g. PF_PIO
 * @return the function number, or -1 if not found
 */
short getPinFunctionNumber(int pin, short func);


#endif /*sblib_ioports_h*/
