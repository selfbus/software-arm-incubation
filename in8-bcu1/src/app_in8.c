/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_in8.h"
#include "com_objs.h"
#include "driver_config.h"

#include "sb_comobj.h"
#include "sb_memory.h"
#include "sb_bus.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#include "gpio.h"
#include "sb_utils.h"


// The previous value of the input pins
unsigned short lastInputsVal;

// Data structure for the debouncing of the inputs
SbDebounce inputDebounce = {0x00};

// The com-object values
ComObjectValues* comObjValues = (ComObjectValues*)(sbUserRamData + UR_COM_OBJ_VALUE0);

// Mask for clearing the relevant com flags when sending a com-object at the low nibble
#define SB_COMFLAG_CLEAR_LOW (~SB_COMFLAG_UPDATE | 0xf0)

// Mask for clearing the relevant com flags when sending a com-object at the high nibble
#define SB_COMFLAG_CLEAR_HIGH ((~SB_COMFLAG_UPDATE << 4) | 0x0f)

/**
 * Get the configured input type of the idx-th input.
 * See INPUT_TYPE_xx defines in com_objs.h
 *
 * @param idx - the index of the input (0..7)
 * @return The configured input type.
 */
static inline unsigned char getInputType(unsigned char idx)
{
    unsigned char type = sbEepromData[EE_INPUT1_TYPE + (idx >> 1)];

    if (idx & 1) type >>= 1;
    else type &= 15;

    return type;
}

/**
 * Handle the input of a switch type input channel.
 *
 * @param channel - the input channel (0..7)
 * @param val - the current value of the pin (0 or 1)
 */
void handle_switch_input(unsigned char channel, unsigned char val)
{
    if (val)
        comObjValues->primary[channel] = 0;
    else comObjValues->primary[channel] = 1;

    sb_send_obj_value(channel);
}

/**
 * Handle input channels
 */
void handle_inputs()
{
    unsigned char inputsVal = sb_debounce(LPC_GPIO[2]->DATA & 0xff, SB_DEBOUNCE_10MS, & inputDebounce);

    if (lastInputsVal == 0xffff) // handle all input pins on first run
        lastInputsVal = ~inputsVal;

    if (inputsVal != lastInputsVal)
    {
        unsigned char valChanged = inputsVal ^ lastInputsVal;
        unsigned char val, channel, mask;

        for (channel = 0, mask = 1; channel < 8; ++channel, mask <<= 1)
        {
            if (!(valChanged & mask)) // do nothing if the pin has not changed
                continue;

            val = (val & mask) != 0;
            switch (getInputType(channel))
            {
            case INPUT_TYPE_SWITCH:
                handle_switch_input(channel, val);
                break;

            default:
                break;
            }
        }

        lastInputsVal = inputsVal;
    }
}

/**
 * Handle an updated com-object.
 *
 * @param objno - the com-object that was updated.
 */
void com_obj_updated(unsigned char objno)
{
    if (objno >= COMOBJ_LOCK1 && objno <= COMOBJ_LOCK1 + 7)
    {

    }
}

/**
 * Handle updates of the com-objects.
 */
void handle_comobj_updates()
{
    sb_process_flags(SB_COMFLAG_UPDATE, &com_obj_updated);
}

/**
 * Initialize the application.
 */
void app_init()
{
    // Configure inputs: direction
    LPC_GPIO[2]->DIR &= ~0xff;

    // Configure inputs: pull-up, hysteresis
    LPC_IOCON->PIO2_0 = 0x30;
    LPC_IOCON->PIO2_1 = 0x30;
    LPC_IOCON->PIO2_2 = 0x30;
    LPC_IOCON->PIO2_3 = 0x30;
    LPC_IOCON->PIO2_4 = 0x30;
    LPC_IOCON->PIO2_5 = 0x30;
    LPC_IOCON->PIO2_6 = 0x30;
    LPC_IOCON->PIO2_7 = 0x30;
}
