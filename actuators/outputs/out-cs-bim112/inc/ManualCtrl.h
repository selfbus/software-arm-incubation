/*
 *  ManualCtrl.h - Manual push buttons
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef MANUALCTRL_H_
#define MANUALCTRL_H_

#include <config.h>

class ManualCtrl;

extern ManualCtrl manuCtrl;

class ManualCtrl {
public:
 ManualCtrl();
 void StartManualCtrl(void); // Liest den Tastenzustand ein und setzt diesen als aktuellen Zustand

 unsigned DoManualCtrl(void); // Muss in einem festen Zeitraster aufgerufen werden. z.B. alle 10ms
 // Liefert als Rückgabewert eine Bitmaske mit je einem gesetzten Bit für eine neu als gedrückt erkannte Taste.

 void UpdateLeds(int newstates); // Weist den LEDs einen neuen Zustand zu und gibt sie aus bei Änderung
 void SetLeds(void); // Setzt die LED-Ausgänge
private:
 unsigned ReadButtons(void);
 int pins_def[BUTTONLEDCNT];
 int led_states;
 unsigned button_states;
 byte button_debounce[BUTTONLEDCNT];
};

#endif /* MANUALCTRL_H_ */
