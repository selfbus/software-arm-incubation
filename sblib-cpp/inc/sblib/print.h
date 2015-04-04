/*
 *  print.h - Base class that provides print() and println()
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_print_h
#define sblib_print_h

#include <sblib/types.h>

/**
 * Base for printing numbers.
 */
enum Base
{
    /** A decimal number. */
    DEC = 10,

    /** A hexadecimal number. */
    HEX = 16,

    /** A octal number. */
    OCT = 8,

    /** A binary number. */
    BIN = 2
};

/**
 * Base class that provides print() and println().
 */
class Print
{
public:
    /**
     * Print a character.
     *
     * @param ch - the character to print.
     *
     * @return The number of bytes that were written.
     */
    int print(const char ch);

    /**
     * Print a zero terminated string.
     *
     * @param str - the string to print.
     *
     * @return The number of bytes that were written.
     */
    int print(const char* str);

    /**
     * Print a number.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(int value, Base base = DEC, int digits = -1);

    /**
     * Print an unsigned number.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int print(unsigned int value, Base base = DEC, int digits = -1);

    /**
     * Print a new line by sending a carriage return '\r' (ASCII 13) followed
     * by a newline '\n' (ASCII 10).
     *
     * @return the number of bytes written.
     */
    int println();

    /**
     * Print a zero terminated string followed by a new line.
     *
     * @param str - the string to print.
     *
     * @return The number of bytes that were written.
     */
    int println(const char* str);

    /**
     * Print a number followed by a new line.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(int value, Base base = DEC, int digits = -1);

    /**
     * Print an unsigned number followed by a new line.
     *
     * @param value - the number to print
     * @param base - the base of the number, default: DEC
     * @param digits - output at least this number of digits (optional)
     *
     * @return The number of bytes that were written.
     */
    int println(unsigned int value, Base base = DEC, int digits = -1);

    /**
     * Write a zero terminated string.
     *
     * @param str - the string to write.
     * @return The number of bytes that were written.
     */
    int write(const char* str);

    /**
     * Write a number of bytes.
     *
     * @param data - the bytes to write.
     * @param count - the number of bytes to write.
     *
     * @return The number of bytes that were written.
     */
    virtual int write(const byte* data, int count);

    /**
     * Write a single byte.
     *
     * @param ch - the byte to write.
     *
     * @return 1 if the byte was written, 0 if not.
     */
    virtual int write(byte ch) = 0;

private:
    int printNumber(unsigned int value, Base base);
};


//
// Inline functions
//

inline int Print::print(const char ch)
{
    return this->write((const byte*)&ch, 1);
}

inline int Print::print(const char* str)
{
    return this->write(str);
}

inline int Print::println(const char* str)
{
    return this->write(str) + println();
}

inline int Print::println(int value, Base base, int digits)
{
    return print(value, base, digits) + println();
}

inline int Print::println(unsigned int value, Base base, int digits)
{
    return print(value, base, digits) + println();
}

#endif /*sblib_print_h*/
