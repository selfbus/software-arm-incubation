/*
 *  ManualCtrl.cpp - Manual push buttons
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <ManualCtrl.h>
#include <sblib/timer.h>

ManualCtrl manuCtrl;

ManualCtrl::ManualCtrl() {
#if BUTTONLEDCNT >= 1
 pins_def[0] = BUTTONLEDCH1;
#endif
#if BUTTONLEDCNT >= 2
 pins_def[1] = BUTTONLEDCH2;
#endif
#if BUTTONLEDCNT >= 3
 pins_def[2] = BUTTONLEDCH3;
#endif
#if BUTTONLEDCNT >= 4
 pins_def[3] = BUTTONLEDCH4;
#endif
#if BUTTONLEDCNT >= 5
 pins_def[4] = BUTTONLEDCH5;
#endif
#if BUTTONLEDCNT >= 6
 pins_def[5] = BUTTONLEDCH6;
#endif
#if BUTTONLEDCNT >= 7
 pins_def[6] = BUTTONLEDCH7;
#endif
#if BUTTONLEDCNT >= 8
 pins_def[7] = BUTTONLEDCH8;
#endif
}

void ManualCtrl::StartManualCtrl(void)
{
 pinMode(BUTTONLEDCOM, OUTPUT);
 digitalWrite(BUTTONLEDCOM, false);

 button_states = ReadButtons();
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
  button_debounce[cnt] = 0;
 led_states = 0;
}

/*
 * Strom sparen?
 * Initialisierung genau wann? Bei RelSpi passiert das auch einfach im Konstruktor.
 * Bereits am Anfang einmal die Buttons einlesen um Aktionen bei einer klemmenden Taste zu verhindern.
 */

// Gehen wir mal von Aufrufen alle 10ms aus
#define BUTTONDOWNDELAY 5 // 50ms
#define BUTTONUPDELAY 25 // 250ms

unsigned ManualCtrl::DoManualCtrl(void)
{
 // Nur alle x ms durchführen
 unsigned act_buttons = ReadButtons();
 unsigned btndnevent = 0;
 unsigned int mask=1;
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  if (button_states & mask)
  {
   if (act_buttons & mask)
   {
    button_debounce[cnt] = 0;
   } else {
    if (button_debounce[cnt] >= BUTTONUPDELAY)
    {
     // Beim Loslassen der Taste keine weiteren Aktionen
     button_states &= ~mask;
     button_debounce[cnt] = 0;
    } else {
     button_debounce[cnt]++;
    }
   }
  } else {
   if (act_buttons & mask)
   {
    if (button_debounce[cnt] >= BUTTONDOWNDELAY)
    {
     // Flag setzen
     btndnevent |= mask;
     button_states |= mask;
     button_debounce[cnt] = 0;
    } else {
     button_debounce[cnt]++;
    }
   } else {
    button_debounce[cnt] = 0;
   }
  }
  mask <<= 1;
 }
 return btndnevent;
}

unsigned ManualCtrl::ReadButtons(void)
{
 // Erst LEDs aus und damit Signalknoten entladen
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  digitalWrite(pins_def[cnt], false);
 }
 delayMicroseconds(2);
 // Dann Signale als Input schalten
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  pinMode(pins_def[cnt], INPUT);
 }
 digitalWrite(BUTTONLEDCOM, true);
 delayMicroseconds(10);
 // Jetzt Taster auslesen
 unsigned int act_buttons=0;
 unsigned int mask=1;
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  act_buttons &= ~mask;
  if (digitalRead(pins_def[cnt]))
   act_buttons |= mask;
  mask <<= 1;
 }
 // Und wieder zurück auf LED-Betrieb
 digitalWrite(BUTTONLEDCOM, false);
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  pinMode(pins_def[cnt], OUTPUT);
 }
 SetLeds();
 return act_buttons;
}

void ManualCtrl::SetLeds(void)
{
 for (int cnt=0; cnt < BUTTONLEDCNT; cnt++)
 {
  digitalWrite(pins_def[cnt], ((led_states >> cnt) & 1) != 0);
 }
}

void ManualCtrl::UpdateLeds(int newstates)
{
 if (led_states != newstates)
 {
  led_states = newstates;
  SetLeds();
 }
}
