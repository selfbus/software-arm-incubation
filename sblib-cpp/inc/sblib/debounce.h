/*
 *  debounce.h - A debouncer.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_debounce_h
#define sblib_debounce_h

/**
 * A debouncer for debouncing a value. The debouncer ensures that a value
 * stays the same over a period of time before it is used.
 *
 * Example:
 *
 *     Debouncer d;
 *     ...
 *     int value = d.debounce(digitalRead(PIO1_8), 100);
 */
class Debouncer
{
public:
    /**
     * Create a debouncer.
     */
    Debouncer();

    /**
     * Send the current value into the debouncer. When the value is the same for at least
     * the debounce time, the new value becomes the valid value.
     *
     * @param current - the current value.
     * @param timeout - the debounce time in milliseconds. Default: 100 msec
     *
     * @return The debounced value.
     */
    int debounce(int current, unsigned int timeout = 100);

    /**
     * @return The debounced value.
     */
    int value() const;

    /**
     * Set the debounced value without debouncing.
     *
     * @param newValue - the new debounced value.
     */
    void init(int newValue);

    /**
     * @return The last temporary value that was sent to debounce()
     */
    int lastValue() const;

private:
    unsigned int time;
    int valid, last;
};


//
//  Inline functions
//

inline int Debouncer::value() const
{
    return valid;
}

inline void Debouncer::init(int newValue)
{
    valid = newValue;
    time = 0;
}

inline int Debouncer::lastValue() const
{
    return last;
}

#endif /*sblib_debounce_h*/
