/*
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef UART_H_
#define UART_H_

/**
 * Initializes the the UART peripheral with the specified baud rate.
 *
 * @param baud_rate     the requested baud rate
 */
void UART_Init (unsigned int baud_rate);

/**
 * Out out one char on the serial line
 *
 * @param byte   the byte which should be put out
 */
void UART_PutChar (char byte);

/**
 * Out add a string to on the serial line.
 *
 * @param string        the string which should be put out
 * @param length        the number of bytes which should be put out.
 *                      If the length is -1 the string will be treated as a
 *                      NULL terminated string and the length will be
 *                      calculated.
 */
void UART_PutString (char * string, signed short length);

/**
 * Return one byte out of the receive buffer.
 *
 * @return  byte receive on the serial line
 */
unsigned char UART_GetChar (void);

#endif /* UART_H_ */
