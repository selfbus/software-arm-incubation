/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef USB_IF_COMMON_ERROR_HANDLER_H_
#define USB_IF_COMMON_ERROR_HANDLER_H_

#ifdef __USE_LPCOPEN
/**
 * Enter a fatal error state and blink all 4 mode LEDs every 100ms.
 *
 * @warning This function never returns.
 */
void fatalError();
#endif

/**
 * Triggers a fatal error in debug mode.
 *
 * @details Only in debug builds (`DEBUG` macro defined), this function calls `fatalError()`. *
 * @warning This function never returns.
 */
void failHardInDebug();


#endif /* USB_IF_COMMON_ERROR_HANDLER_H_ */
