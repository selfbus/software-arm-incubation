/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in8.h"
#include "com_objs.h"
#include "params.h"

#include <sblib/eib.h>
#include <sblib/eib/sblib_default_objects.h>
#include <sblib/timeout.h>


//	WICHTIG!!!
//Hier muss EINE! Hardware ausgewählt werden (IN8_230V_4TE und IN8_24V_4TE werden identisch behandelt)

#define IN8_230V_4TE   // Platine: fb_in_8_230V_42  -  8x230V Eingang im 4TE Gehäuse
//#define IN8_24V_4TE    // Platine: fb_in8_24VDC_3.2  -  8x24V Eingang im 4TE Gehäuse
//#define IN8_24V_2TE    // Platine: 2te8x24V_DC_In_4_v02  -  8x24V Eingang im 2TE Gehäuse mit 2te8LED_v03 Platine


// Digital pin for LED
#define PIO_LED PIO2_0

#if defined(IN8_230V_4TE) || defined(IN8_24V_4TE)
	#define DIRECT_IO
#endif

// Debouncers for inputs
Debouncer inputDebouncer[NUM_CHANNELS];

const byte* channelParams = userEepromData + (EE_CHANNEL_PARAMS_BASE - USER_EEPROM_START);
const byte* channelTimingParams = userEepromData + (EE_CHANNEL_TIMING_PARAMS_BASE - USER_EEPROM_START);

#ifdef DIRECT_IO
// Input pins
const int inputPins[] =
//    { PIO2_2, PIO0_7, PIO2_10, PIO2_9, PIO0_2, PIO0_8, PIO0_9, PIO2_11 };  // 4TE controller
    { PIO2_2, PIO0_9, PIO2_11, PIO1_1, PIO3_0, PIO3_1, PIO3_2, PIO2_9 };   // TS-ARM

void setupIO(void)
{
    // Configure the input pins and initialize the debouncers with the current
    // value of the pin.
    for (int channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        pinMode(inputPins[channel], INPUT | HYSTERESIS | PULL_DOWN);
        inputDebouncer[channel].init(digitalRead(inputPins[channel]));
    }
}

void scanIO(void)
{

}

int readIO(int channel)
{
    return digitalRead(inputPins[channel]);
}

#else
#include <sblib/spi.h>

SPI spi(SPI_PORT_0);
static unsigned int ioState;

void setupIO(void)
{
   ioState = 0;
   pinMode(PIO0_8,  OUTPUT | SPI_MISO);  // MISO - IO2 ARM 2TE (LPC1115_2TE_V1.04)
   pinMode(PIO0_2,  OUTPUT);             // SSEL - IO3 ARM 2TE (LPC1115_2TE_V1.04)
   pinMode(PIO0_6, OUTPUT | SPI_CLOCK);  // SCK  - IO1 ARM 2TE (LPC1115_2TE_V1.04)

   spi.setClockDivider(128);
   spi.begin();
   spi.setDataSize(SPI_DATA_8BIT);
}

void scanIO(void)
{
    digitalWrite(PIO0_2, 0);
    digitalWrite(PIO0_2, 1);
    ioState = spi.transfer(0x00);
}

int readIO(int channel)
{
    return ioState & (1 << channel);

}
#endif


#ifdef IN8_24V_2TE

// Setup eines SPI für den IC 74HC595 auf der 2TE LED Platine
SPI spi1(SPI_PORT_1);

void setupLEDs(void)
{
	pinMode(PIO2_1, OUTPUT | SPI_CLOCK);
	pinMode(PIO2_3, OUTPUT | SPI_MOSI);
	pinMode(PIO2_4, OUTPUT); //SSEL

	spi1.setClockDivider(128);
	spi1.begin();
	spi1.setDataSize(SPI_DATA_8BIT);
}

// den Status der eingelesenen Eingänge auf die LEDs übertragen
void setLEDs(void)
{
	digitalWrite(PIO2_4, 0);
	spi1.transfer(ioState);
	digitalWrite(PIO2_4, 1);
}

#endif

/**
 * Application setup
 */
void setup()
{
    bcu.begin(4, 0x7054, 2); // We are a "Jung 2118" device, version 0.2

    pinMode(PIO_LED, OUTPUT);
    digitalWrite(PIO_LED, 1);

    setupIO();
#ifdef IN8_24V_2TE
    setupLEDs();
#endif
    // Handle configured power-up delay
    unsigned int startupTimeout = calculateTime
            ( userEeprom[EE_BUS_RETURN_DELAY_BASE] >> 4
            , userEeprom[EE_BUS_RETURN_DELAY_FACT] &  0x7F
            );
    Timeout delay;
    int debounceTime = userEeprom[EE_INPUT_DEBOUNCE_TIME] >> 1;
    delay.start(startupTimeout);
    while (delay.started() && !delay.expired())
    {   // while we wait for the power on delay to expire we debounce the input channels
        scanIO();
        for (int channel = 0; channel < NUM_CHANNELS; ++channel)
        {
            inputDebouncer[channel].debounce(readIO(channel), debounceTime);
        }
        waitForInterrupt();
    }
    initApplication();
}


/**
 * The application's main.
 */
void loop()
{
    int debounceTime = userEeprom[EE_INPUT_DEBOUNCE_TIME] >> 1;
    int objno, channel, value, lastValue;

    scanIO();
#ifdef IN8_24V_2TE
    setLEDs();
#endif
    // Handle the input pins
    for (channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        lastValue = inputDebouncer[channel].value();
        value = inputDebouncer[channel].debounce(readIO(channel), debounceTime);

        if (lastValue != value)
            inputChanged(channel, value);
    }

    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    // Handle timed functions (e.g. periodic update)
    handlePeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
