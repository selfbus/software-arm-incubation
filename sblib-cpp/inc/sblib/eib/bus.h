/*
 *  bus.h - Low level EIB bus access.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_bus_h
#define sblib_bus_h

#include <sblib/core.h>
#include <sblib/eib/com_objects.h>

class Bus;


/**
 * Low level class for EIB bus access.
 *
 * When creating a bus object, the handler for the timer must also be
 * created. Example:
 *
 * Bus mybus(timer32_0);
 * BUS_TIMER_INTERRUPT_HANDLER(TIMER32_0_IRQHandler, mybus);
 */
class Bus
{
public:
    /**
     * Create a bus access object.
     *
     * @param timer - the timer to use.
     */
    Bus(Timer& timer);

    /**
     * Begin using the bus.
     *
     * This powers on all used components.
     * This method must be called before the bus can be used.
     */
    void begin();

    /**
     * End using the bus.
     *
     * This powers the bus off.
     */
    void end();

    /**
     * Test if the bus is idle and no telegram is about to being sent.
     *
     * @return 1 when idle, 0 when not.
     */
    bool idle() const;

    /**
     * Send a telegram. The checksum byte will be added at the end of telegram[].
     * Ensure that there is at least one byte space at the end of telegram[].
     *
     * @param telegram - the telegram to be sent.
     * @param length - the length of the telegram in sbSendTelegram[], without the checksum
     */
    void sendTel(unsigned char* telegram, unsigned short length);

    /**
     * This method shall be called by the timer interrupt handler.
     */
    void timerInterrupt();

protected:
    /**
     * Prepare the telegram for sending. Set the sender address to our own
     * address, and calculate the checksum of the telegram.
     * Stores the checksum at telegram[length].
     *
     * @param telegram - the telegram to process
     * @param length - the length of the telegram
     */
    void prepareTel(unsigned char* telegram, unsigned short length) const;

protected:
    Timer& timer;
};

/**
 * Create an interrupt handler for the EIB bus access.
 *
 * @param handler - the name of the interrupt handler, e.g. TIMER16_0_IRQHandler
 * @param busObj - the bus object that shall receive the interrupt.
 */
#define BUS_TIMER_INTERRUPT_HANDLER(handler, busObj) \
    extern "C" void handler() { busObj.timerInterrupt(); }

#endif /*sblib_bus_h*/
