/*
 *  spi.h - Serial peripheral interface (SPI).
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_spi_h
#define sblib_spi_h

#include <sblib/platform.h>


/**
 * The SPI transfer modes.
 */
enum SpiTransferMode
{
    /**
     * Leave the slave select pin active (low). Use this if you want to send
     * another byte to the same slave.
     */
    SPI_CONTINUE,

    /**
     * Deactivate the slave select pin after the transfer. Use this if you
     * do not want to send another byte.
     */
    SPI_LAST
};


/**
 * The SPI port.
 */
enum SpiPort
{
    SPI_PORT_0,
    SPI_PORT_1
};

/**
 * The SPI data size in bits.
 */
enum SpiDataSize
{
    SPI_DATA_4BIT = 4,
    SPI_DATA_5BIT,
    SPI_DATA_6BIT,
    SPI_DATA_7BIT,
    SPI_DATA_8BIT,
    SPI_DATA_9BIT,
    SPI_DATA_10BIT,
    SPI_DATA_11BIT,
    SPI_DATA_12BIT,
    SPI_DATA_13BIT,
    SPI_DATA_14BIT,
    SPI_DATA_15BIT,
    SPI_DATA_16BIT
};

/**
 * The SPI mode settings.
 */
enum SpiMode
{
    /**
     * Frame format: SPI (normal mode).
     */
    SPI_FORMAT_SPI = 0,

    /**
     * Frame format: TI
     */
    SPI_FORMAT_TI = 0,

    /**
     * Frame format: Microwire
     */
    SPI_FORMAT_MICROWIRE = 0,

    /**
     * Low clock polarity: the clock pin will be low between transmits.
     */
    SPI_CPOL_LOW = 0,

    /**
     * High clock polarity: the clock pin will be high between transmits.
     */
    SPI_CPOL_HIGH = (1<<6),

    /**
     * Capture data on the rising edge of the SPI clock.
     */
    SPI_CPHASE_RAISE = 0,

    /**
     * Capture data on the falling edge of the SPI clock.
     */
    SPI_CPHASE_FALL = (1<<7),
};

// The default SPI mode settings
#define SPI_DEFAULT_MODE  (SPI_FORMAT_SPI | SPI_CPOL_LOW | SPI_CPHASE_RAISE)


/**
 * Class for accessing the serial peripheral interfaces (SPI).
 *
 * To use the SPI, the IO pins that shall be used for SPI must be configured
 * for SPI mode using pinMode(). You need to assign the following SPI lines
 * to IO pins when using SPI port 0: SCK0, MISO0, MOSI0. Ffor SPI port 1, use
 * SCK1, MISO1, MOSI1.
 *
 * This class implements a SPI master.
 */
class SPI
{
public:
    /**
     * Create a SPI access object.
     *
     * @param spiPort - the SPI port: SPI_PORT_0, SPI_PORT_1.
     * @param mode - the SPI port mode. This is a combination of the values of enum SpiMode (see above).
     *               The default mode is SPI_FORMAT_SPI | SPI_CPOL_LOW | SPI_CPHASE_RAISE.
     */
    SPI(int spiPort, int mode = SPI_DEFAULT_MODE);

    /**
     * Power on the SPI port. Call this method before starting to use the SPI
     * port.
     */
    void begin();

    /**
     * Power off the SPI port. Call this method when you are done using the
     * SPI port to reduce power consumption.
     */
    void end();

    /**
     * Set the SPI clock divider. The clock is divided by this divider
     * to get the SPI clock that is output on the SCK clock pin. Default
     * is 1 (do not divide the clock).
     *
     * @param div - the clock divider in the range 1..256
     */
    void setClockDivider(int div);

    /**
     * Set the SPI data size. Default is 8 bit.
     *
     * @param dataSize - the data size, e.g. SPI_DATA_8BIT
     */
    void setDataSize(SpiDataSize dataSize);

    /**
     * Transfer one byte over the SPI bus. Sending and receiving of one
     * byte is done.
     *
     * Before the transfer starts, the specified slave select pin is pulled
     * to low. After the transfer, the slave select pin is deactivated (pulled high).
     * This is the same as calling transfer with the transfer mode SPI_LAST.
     *
     * @param val - the byte to transfer.
     *
     * @return The received byte
     */
    int transfer(int val);

    /**
     * Transfer a value over the SPI bus and receive a value from the bus. The number
     * of bits that the value has depends on the configured data size - @see setDataSize() above.
     *
     * Before the transfer starts, the specified slave select pin is pulled
     * to low. After the transfer, the slave select pin is deactivated (pulled high).
     *
     * @param val - the value to transfer.
     * @param transferMode - the transfer mode: SPI_CONTINUE or SPI_LAST
     *
     * @return The received value
     */
    int transfer(int val, SpiTransferMode transferMode);

protected:
    LPC_SSP_TypeDef& port;
    int clockDiv;
};


inline int SPI::transfer(int value)
{
    return transfer(value, SPI_LAST);
}

#endif /*sblib_spi_h*/
