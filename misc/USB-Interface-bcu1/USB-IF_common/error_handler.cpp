/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <stdint.h>
#ifdef __USE_LPCOPEN
#   include <chip.h>
#   include <gpio_11xx_1.h>
#   include <core_cm0.h>
#else
#   include <sblib/utils.h>
#endif

#include "error_handler.h"

#ifdef __USE_LPCOPEN

extern volatile unsigned int systemTime;
void blinkLed(uint16_t blinkMilliSeconds);

/**
* Interrupt service routine to override the default HardFault exception handler in cr_startup_lpc11uxx.cpp
* This function will never return and all LEDs will blink rapidly.
*/
extern "C" void HardFault_Handler()
{
   blinkLed(25);
}

/**
 * Blinks all 4 mode LEDs with the specified duration in milliseconds.
 *
 * @param blinkMilliSeconds Time in milliseconds between toggles.
 *
 * @warning This function never returns.
 */
void blinkLed(uint16_t blinkMilliSeconds)
{
    SysTick_Config((SystemCoreClock / 1000) * blinkMilliSeconds);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 16);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 22);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 15);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 17);

    bool state = false;
    while (1)
    {
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
        {
            state = !state;
            Chip_GPIO_SetPinState(LPC_GPIO, 1, 16, state);
            Chip_GPIO_SetPinState(LPC_GPIO, 1, 22, state);
            Chip_GPIO_SetPinState(LPC_GPIO, 1, 15, state);
            Chip_GPIO_SetPinState(LPC_GPIO, 0, 17, state);
        }
    }
}

void fatalError()
{
    blinkLed(100);
}
#endif

void failHardInDebug()
{
#ifdef DEBUG
    fatalError();
#endif
}

