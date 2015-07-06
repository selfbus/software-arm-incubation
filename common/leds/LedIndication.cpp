/*
 * LedIndication.cpp
 *
 *  Created on: 04.07.2015
 *      Author: glueck
 */

#include "LedIndication.h"
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>

#define POSTOMASK(i) (1 << (i - 1))
const unsigned int ledToMask[] =
{
    POSTOMASK(2),  POSTOMASK(3),  POSTOMASK(4),  POSTOMASK(6)
  , POSTOMASK(1),  POSTOMASK(5),  POSTOMASK(7),  POSTOMASK(8)
  , POSTOMASK(16), POSTOMASK(15), POSTOMASK(14), POSTOMASK(9)
  , POSTOMASK(13), POSTOMASK(12), POSTOMASK(11), POSTOMASK(10)
};

Led_Indication::Led_Indication(int spiPort, int pinData, int pinClock, int pinRCK, int pinOE)
    : spi(spiPort)
    , pinRCK(pinRCK)
    , pinOE(pinOE)
    , _state(0)
{
    pinMode(pinData,  OUTPUT | SPI_MOSI);
    pinMode(pinClock, OUTPUT | SPI_CLOCK);
    pinMode(pinRCK,   OUTPUT);
    pinMode(pinOE,    OUTPUT);
    digitalWrite(pinOE, true);
    digitalWrite(pinRCK, true);
}

void Led_Indication::setStatus(unsigned int ledNo, unsigned int state, bool update)
{
    unsigned int mask = ledToMask[ledNo];
    if (state)  _state |= mask;
    else        _state &= ~mask;
    if (update)
        updateLeds();
}

void Led_Indication::begin()
{
    spi.begin();
    spi.setClockDivider(128);
    spi.setDataSize(SPI_DATA_16BIT);
    updateLeds();
    digitalWrite(pinOE, false);
}

void Led_Indication::updateLeds(void)
{
    digitalWrite(pinRCK, false);
    spi.transfer(_state);
    digitalWrite(pinRCK, true);
}
