/*
 *  main.cpp - The library's main.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/main.h>

#include <sblib/eib.h>
#include <sblib/interrupt.h>
#include <sblib/timer.h>

#include <sblib/internal/functions.h>
#include <sblib/internal/variables.h>

// Pin of the programming mode button+led
#define PROG_PIN  PIO1_5

Debouncer progButtonDebouncer;
unsigned int writeUserEepromTime;


/**
 * Setup the library.
 */
static inline void lib_setup()
{
    // Configure the system timer to call SysTick_Handler once every 1 msec
    SysTick_Config(SystemCoreClock / 1000);
    systemTime = 0;
    writeUserEepromTime = 0;

    bcu.progPin = PROG_PIN;
    progButtonDebouncer.init(1);

    bcu.begin();
}

/**
 * The main of the library.
 *
 * In your program, you will implement setup() and loop(), which are both
 * called by this function.
 */
int main()
{
    lib_setup();
    setup();

    while (1)
    {
        if (bus.telegramReceived() && !bus.sendingTelegram() && (userRam.status & BCU_STATUS_TL))
            bcu.processTelegram();

        if (!bus.sendingTelegram())
            sendNextGroupTelegram();

        if (bcu.progPin)
        {
            // Detect the falling edge of pressing the prog button
            pinMode(bcu.progPin, INPUT);
            int oldValue = progButtonDebouncer.value();
            if (!progButtonDebouncer.debounce(digitalRead(bcu.progPin), 50) && oldValue)
                userRam.status ^= 0x81;  // toggle programming mode and checksum bit

            pinMode(bcu.progPin, OUTPUT);
            digitalWrite(bcu.progPin, !(userRam.status & BCU_STATUS_PROG));
        }

        if (userEeprom.isModified() && bus.idle() && bus.telegramLen == 0 && !bcu.directConnection())
        {
            if (writeUserEepromTime)
            {
                if (millis() - writeUserEepromTime > 0)
                    writeUserEeprom();
            }
            else writeUserEepromTime = millis() + 50;
        }

        // We must enter loop() if the prog button is handled inside loop()
        if (bcu.applicationRunning() || !bcu.progPin)
            loop();
        else waitForInterrupt();
    }
}
