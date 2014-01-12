/*
 *  telegram.h - An EIB bus telegram.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_telegram_h
#define sblib_telegram_h

#include <sblib/printable.h>
#include <sblib/types.h>

enum
{
    /**
     * The maximum size of a telegram in bytes.
     */
    TELEGRAM_SIZE = 23
};


/**
 * A telegram as it is transfered on the EIB bus.
 */
class Telegram: public Printable
{
public:
    /**
     * Begin composing a telegram.
     *
     * @param prio - the priority (0..3)
     */
    void begin(int prio);

    /**
     * @return The 16 bit sender address.
     */
    int sender() const;

    /**
     * Set the sender address. Usually the sender address is set
     * automatically when a telegram is being sent by the bus.
     *
     * @param addr - the 16 bit sender address.
     */
    void sender(int addr);

    /**
     * @return The 16 bit sender address.
     */
    int receiver() const;

    /**
     * Set the receiver address.
     *
     * @param addr - the address.
     * @param isGroup - 1 if the address is a group address, 0 if not
     */
    void receiver(int addr, bool isGroup);

    /**
     * @return The length of the telegram's data, which starts at data[7].
     */
    byte length() const;

    /**
     * Set the length of the telegram's data, which starts at data[7].
     *
     * @param len - the length
     */
    void length(byte len);

    /**
     * Print the telegram to the print target as a string of hex bytes.
     *
     * @param out - the target to print to.
     * @return The number of bytes written.
     */
    virtual int printTo(Print& out) const;

    /**
     * The raw telegram data.
     */
    byte data[TELEGRAM_SIZE];
};


//
//  Inline functions
//

inline void Telegram::begin(int prio)
{
    data[0] = 0xb0 | (prio << 2);
}

inline int Telegram::sender() const
{
    return (data[1] << 8) | data[2];
}

inline void Telegram::sender(int addr)
{
    data[1] = addr >> 8;
    data[2] = addr;
}

inline int Telegram::receiver() const
{
    return (data[3] << 8) | data[4];
}

inline byte Telegram::length() const
{
    return data[5] & 0x0f;
}

inline void Telegram::length(byte len)
{
    data[5] = (data[5] & 0xf0) | (len & 0x0f);
}

#endif /*sblib_telegram_h*/
