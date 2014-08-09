/*
 *  graphical_sed1520.cpp - class for graphical LCD display with a SED1520
 *                          compatible controller.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/lcd/graphical_sed1520.h>
#include <sblib/digital_pin.h>

#define DISPLAY_WIDTH 132
#define DISPLAY_HEIGHT 4

#define DATA_PORT_PINMASK 255


LcdGraphicalSED1520::LcdGraphicalSED1520(int dataPort, int pinA0, int pinCS1, int pinCS2,
    int pinRD, int pinWR, const Font& fnt)
:LcdGraphical(fnt, DISPLAY_WIDTH, DISPLAY_HEIGHT)
,dataPort(dataPort)
,pinA0(pinA0)
,pinCS1(pinCS1)
,pinCS2(pinCS2)
,pinRD(pinRD)
,pinWR(pinWR)
{
    portMode(dataPort, DATA_PORT_PINMASK, INPUT);
    pinMode(pinA0, OUTPUT);
    pinMode(pinCS1, OUTPUT);
    pinMode(pinCS2, OUTPUT);
    pinMode(pinRD, OUTPUT);
    pinMode(pinWR, OUTPUT);

    digitalWrite(pinCS1, 1);
    digitalWrite(pinCS2, 1);
    digitalWrite(pinRD, 1);
    digitalWrite(pinWR, 1);
}

void LcdGraphicalSED1520::idle()
{

}
