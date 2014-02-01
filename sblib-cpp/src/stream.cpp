/*
 *  stream.cpp - Base class for character-based streams.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/stream.h>

#include <sblib/math.h>
#include <sblib/timer.h>

int Stream::parseInt(char skipChar)
{
    bool negative = false;
    int value = 0;

    int ch = peekNextDigit(); // skip leading non numeric characters
    if (ch == '-')
    {
        negative = true;
        ch = '0';
    }

    while (ch >= 0)
    {
        if (ch >= '0' && ch <= '9')
            value = value * 10 + ch - '0';
        else if (ch != skipChar)
            break;

        read(); // consume the character we got with peek
        ch = timedPeek();
    }

    if (negative) return -value;
    return value;
}

int Stream::_readBytesUntil(int terminator, char* buffer, int length)
{
    int ch, count;

    for (count = 0; count < length; ++count)
    {
        ch = timedRead();
        if (ch < 0 || ch == terminator)
            break;

        buffer[count] = ch;
    }

    return count;
}

bool Stream::findUntil(const char* target, int targetLen, const char* terminator, int termLen)
{
    int targetIdx = 0;
    int termIdx = 0;
    int ch;

    while ((ch = timedRead()) >= 0)
    {
        if (ch == target[targetIdx])
        {
            if (++targetIdx >= targetLen)
                return true;
        }
        else targetIdx = 0;

        if (termLen > 0)
        {
            if (ch == terminator[termIdx])
            {
                if (++termIdx >= termLen)
                    return false;
            }
            else termIdx = 0;
        }
    }

    return false;
}

int Stream::timedRead()
{
    int start = millis();

    int ch = peeked;
    peeked = -1;

    while (ch < 0 && elapsed(start) < timeout)
    {
        ch = read();
    }

    return ch;
}

int Stream::timedPeek()
{
    int start = millis();

    while (peeked < 0 && elapsed(start) < timeout)
    {
        peek();
    }

    return peeked;
}

int Stream::peekNextDigit()
{
    int c;
    while (true)
    {
        c = timedPeek();
        if (c < 0) break; // timeout

        if (c == '-') break;
        if (c >= '0' && c <= '9') break;

        read(); // discard non-numeric
    }

    return c;
}
