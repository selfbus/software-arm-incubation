/*
 *  app_out8.h - The application for the 8 channel output acting as a Jung 2118
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef APP_OUT8_H_
#define APP_OUT8_H_

#include "config.h"
#include "com_objs.h"

// EEPROM addresses
#define APP_PIN_STATE_MEMORY         (0x100)
#define APP_SPECIAL_FUNC_OBJ_1_2     (0x1D8)
#define APP_SPECIAL_FUNC_OBJ_3_4     (0x1D9)
#define APP_DELAY_FACTOR_ON          (0x1DA) // Einschaltverzoegerung Faktor (0...127)
#define APP_DELAY_FACTOR_OFF         (0x1E2) // Ausschaltverzoegerung Faktor (0...127)
//< 0x00=no timer active, 0x01=timer on object 1 active, 0x03=timer on object 1+2 active
#define APP_DELAY_ACTIVE             (0x1EA)
#define APP_DELAY_ACTION             (0x1EB)

// from 0x01D8-0x01D9
#define APP_SPECIAL_CONNECT          (0x1D8)
#define APP_SPECIAL_FUNC_MODE        (0x1ED)
#define APP_SPECIAL_LOGIC_MODE       (0x1EE)
#define APP_SPECIAL_FUNCTION1        (0x1EF)
#define APP_SPECIAL_FUNCTION2        (0x1F0)
#define APP_SPECIAL_POLARITY         (0x1F1)

#define APP_CLOSER_MODE              (0x1F2)
#define APP_REPORT_BACK_INVERT       (0x1F3)
// 0x01F6-0x01F7 output reaction after bus power restore
#define APP_RESTORE_AFTER_PL_LO      (0x1F6)
#define APP_RESTORE_AFTER_PL_HI      (0x1F7)
#define APP_DELAY_BASE               (0x1F9) // Einschalt/Ausschaltverzoegerung Basis

extern const int outputPins[NO_OF_OUTPUTS];
void objectUpdated(int objno);
void checkTimeouts(void);
void initApplication(int lastRelayState = 0x00);
unsigned int  getRelaysState();
/**
 * The values of the communication objects
 */
extern ObjectValues& objectValues;


#endif /* APP_OUT8_H_ */
