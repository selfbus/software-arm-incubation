/*
 *  stream.h - Base class for character-based streams.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_stream_h
#define sblib_stream_h

#include <sblib/print.h>
#include <string.h>


/**
 * Base class for character-based streams.
 * Streams can be read from and written to.
 */
class Stream: public Print
{
public:
    /**
     * Set the maximum number of milliseconds that are waited for a
     * character to arrive. Default is 1000 msec.
     *
     * @param timeout - the number of milliseconds to wait.
     */
    void setTimeout(unsigned int timeout);

    /**
     * Read a single byte.
     *
     * @return The read byte (0..255) or -1 if no byte was received.
     */
    virtual int read() = 0;

    /**
     * Query the next byte to be read, without reading it.
     *
     * @return The next byte (0..255) or -1 if no byte is available
     *         for reading.
     */
    virtual int peek() = 0;

    /**
     * @return The number of bytes that are available for reading.
     */
    virtual int available() = 0;

    /**
     * Wait until all bytes are written.
     */
    virtual void flush() = 0;

    /**
     * Read the first valid integer value from the current position. Initial characters
     * that are not digits (or the minus sign) are skipped. The integer is terminated
     * by the first character that is not a digit.
     *
     * @return The read integer, or 0 if no integer could be read due to timeout.
     */
    int parseInt();

    /**
     * Read the first valid integer value from the current position. Initial characters
     * that are not digits (or the minus sign) are skipped. The integer is terminated
     * by the first character that is not a digit. If the skip character is read, it
     * is ignored and skipped. Digits are no valid skip character, they are never ignored.
     *
     * @param skipChar - the character to ignore when read (do not use a digit)
     *
     * @return The read integer, or 0 if no integer could be read due to timeout.
     */
    int parseInt(char skipChar);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read or a timeout occurs.
     *
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    int readBytes(char* buffer, int length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read or a timeout occurs.
     *
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    int readBytes(byte* buffer, int length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator - the terminator character
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    int readBytesUntil(char terminator, char* buffer, int length);

    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator - the terminator character
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    int readBytesUntil(char terminator, byte* buffer, int length);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target - the target string to find, zero terminated.
     * @return true if target string is found, false if timed out.
     */
    bool find(const char* target);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target - the target string to find, zero terminated.
     * @return true if target string is found, false if timed out.
     */
    bool find(const byte* target);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target - the target string to find.
     * @param length - the length of the target string to find.
     *
     * @return true if target string is found, false if timed out.
     */
    bool find(const char* target, int length);

    /**
     * Reads data from the stream until the target string is read.
     *
     * @param target - the target string to find.
     * @param length - the length of the target string to find.
     *
     * @return true if target string is found, false if timed out.
     */
    bool find(const byte* target, int length);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find, zero terminated.
     * @param terminator - the terminator string, zero terminated.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const char* target, const char* terminator);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find, zero terminated.
     * @param terminator - the terminator string, zero terminated.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const byte* target, const char* terminator);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find.
     * @param targetLen - the length of the target string.
     * @param terminator - the terminator string.
     * @param termLen - the length of the terminator string.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const char* target, int targetLen, const char* terminator, int termLen);

    /**
     * Reads data from the stream until the target string is read. Reading stops
     * if the target string is read, the terminator string is read, or a timeout
     * occurs.
     *
     * @param target - the target string to find.
     * @param targetLen - the length of the target string.
     * @param terminator - the terminator string.
     * @param termLen - the length of the terminator string.
     *
     * @return true if target string is found, false if the terminator string was
     *         read or a timeout occurred.
     */
    bool findUntil(const byte* target, int targetLen, const char* terminate, int termLen);

protected:
    unsigned int timeout; //!< timeout for timed reads in milliseconds

    /**
     * Create a stream with the default timeout of 1 second.
     */
    Stream();

    /**
     * Read the next byte. Wait up to the number of milliseconds that are configured
     * as timeout.
     *
     * @return The read byte (0..255) or -1 if no byte was read within the timeout.
     */
    int timedRead();

    /**
     * Query the next byte to be read, without reading it. Wait up to the number of
     * milliseconds that are configured as timeout.
     *
     * @return The peeked byte (0..255) or -1 if no byte was peeked within the timeout.
     */
    int timedPeek();

    /**
     * Query the next numeric digit (or minus) from the stream. Non-digit characters
     * are discarded.
     *
     * @return The next digit or -1 if a timeout occurs.
     */
    int peekNextDigit();

private:
    /**
     * Read characters from the stream into the buffer. Reading stops if length
     * characters have been read, the terminator character is read, or a timeout occurs.
     *
     * @param terminator - the terminator character or -1 if none
     * @param buffer - the buffer to read into
     * @param length - the maximum number of bytes to read
     *
     * @return The number of bytes read.
     */
    int _readBytesUntil(int terminator, char* buffer, int length);
};


//
//  Inline functions
//

inline Stream::Stream()
{
    timeout = 1000;
}

inline void Stream::setTimeout(unsigned int tmout)
{
    timeout = tmout;
}

inline int Stream::parseInt()
{
    return parseInt('0');
}

inline int Stream::readBytes(char* buffer, int length)
{
    return _readBytesUntil(-1, buffer, length);
}

inline int Stream::readBytes(byte* buffer, int length)
{
    return _readBytesUntil(-1, (char*) buffer, length);
}

inline int Stream::readBytesUntil(char terminator, char* buffer, int length)
{
    return _readBytesUntil(terminator, (char*) buffer, length);
}

inline int Stream::readBytesUntil(char terminator, byte* buffer, int length)
{
    return _readBytesUntil(terminator, (char*) buffer, length);
}

inline bool Stream::find(const char* target)
{
    return findUntil(target, strlen(target), (const char*) 0, 0);
}

inline bool Stream::find(const byte* target)
{
    return find((const char*) target);
}

inline bool Stream::find(const char* target, int length)
{
    return findUntil(target, length, (const char*) 0, 0);
}

inline bool Stream::find(const byte* target, int length)
{
    return find((const char*) target, length);
}

inline bool Stream::findUntil(const char* target, const char* terminator)
{
    return findUntil(target, strlen(target), terminator, strlen(terminator));
}

inline bool Stream::findUntil(const byte* target, const char* terminator)
{
    return findUntil((const char*) target, terminator);
}

inline bool Stream::findUntil(const byte* target, int targetLen, const char* terminate, int termLen)
{
    return findUntil((const char*) target, targetLen, terminate, termLen);
}

#endif /*sblib_print_h*/
