/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sb_hal_h
#define sb_hal_h

/*
 * This file contains low-level defines for the hardware configuration that the sblib uses.
 * One target processors should be defined. If none is defined, LPC11xx is assumed.
 *
 * Valid target processors are:
 *
 * SB_TARGET_LPC11XX:
 *     Configuration suitable for most LPC11xx, LPC11Cxx, LPC11Uxx processors
 *     16-bit timer #1 for bus access
 *     bus in: P1.8,  bus out: P1.10,  PROG button: P1.5
 *
 * SB_TARGET_LPC11XX_ALT:
 *     Alternative configuration
 *     16-bit timer #1 for bus access
 *     bus in: P1.8,  bus out: P1.9,  PROG button: P1.5
 */

// Fallback if no target processor is set
#if !defined(SB_TARGET_LPC11XX)
#  define SB_TARGET_LPC11XX
#endif


/*
 * Bus access
 */
#if defined(SB_TARGET_LPC11XX)
#  define LPC_TMR16B1_MR_OUT LPC_TMR16B1->MR1    /* Timer match register for bus out */
#  define LPC_IOCON_BUS_OUT LPC_IOCON->PIO1_10   /* IOCON register for bus out */
#  define BUS_OUT_IOCON_PWM 2                    /* IOCON for bus out: PWM channel selection */
#  define BUS_OUT_PORT_PIN 1,10                  /* Port and pin for bus out */
#endif
#if defined(SB_TARGET_LPC11XX_ALT)
#  define LPC_TMR16B1_MR_OUT LPC_TMR16B1->MR0
#  define LPC_IOCON_BUS_OUT LPC_IOCON->PIO1_9
#  define BUS_OUT_IOCON_PWM 1
#  define BUS_OUT_PORT_PIN 1,9
#endif


/*
 * Programming mode button and led
 */
#if defined(SB_TARGET_LPC11XX) || defined(SB_TARGET_LPC11XX_ALT)
#  define SB_PROG_PORT    1                      /* port for prog button+led */
#  define SB_PROG_BIT     5                      /* port bit for prog button+led */
#  define LPC_IOCON_PROG  LPC_IOCON->PIO1_5      /* IOCON registor for prog button+ld */
#  define SB_PROG_IOCON   0x20                   /* IOCON configuration for prog button+led */
#endif

// Bitmask for programming mode button and led
#define SB_PROG_MASK  (1 << SB_PROG_BIT)


#endif /*sb_hal_h*/
