/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef UART_H_
#define UART_H_

void UART_Init (unsigned int baud_rate);
void UART_PutChar (unsigned char byte);
void UART_PutString (unsigned char * string, signed short length);
unsigned char UART_GetChar (void);

#endif /* UART_H_ */
