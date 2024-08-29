/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <oneWireChannel.h>
#include "app-OneWireGateway.h"
#include "oneWireParameters.h"
#include "config.h"
#include <sblib/eib.h>
#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>

#include "ds18x20_onewire_ds2482.h"
#include <sblib/eib/com_objects.h>

bool readAllSensors = false;
uint8_t minuteCounter = 0;
uint8_t secondCounter = 0;

OneWireChannel oneWireChannelA;
OneWireChannel oneWireChannelB;
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
OneWireChannel oneWireChannelC;
OneWireChannel oneWireChannelD;
#endif

void objectUpdated(int objno) {

}

void checkPeriodicFuntions(void) {
	// alle 10 Sekunden sollen alle Werte geholt und nach neuen OneWire Geräten gesucht werden
	if (readAllSensors) {
		readAllSensors = false;
		oneWireChannelA.checkPeriodic();
		oneWireChannelB.checkPeriodic();
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
		oneWireChannelC.checkPeriodic();
		oneWireChannelD.checkPeriodic();
#endif
	}

	oneWireChannelA.cyclicSend();
	oneWireChannelB.cyclicSend();
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
	oneWireChannelC.cyclicSend();
	oneWireChannelD.cyclicSend();
#endif
}

void initApplication(void) {

	// Initialize DS2482 Onewire Channels
	oneWireChannelA.init(0b00);
	oneWireChannelB.init(0b01);
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
	oneWireChannelC.init(0b10);
	oneWireChannelD.init(0b11);
#endif

	// generellen Timer stellen, um alle 10 Sekunden alle Sensoren abzufragen

	// Enable the timer interrupt
	enableInterrupt(TIMER_32_0_IRQn);

	// Begin using the timer
	timer32_0.begin();

	// Let the timer count milliseconds
	timer32_0.prescaler((SystemCoreClock / 1000) - 1);

	// On match of MAT0, generate an interrupt and reset the timer
	timer32_0.matchMode(MAT0, RESET | INTERRUPT);

	// Match MAT0 when the timer reaches this value (in milliseconds)
	timer32_0.match(MAT0, TIMER32_0_STEP); //10s period

	// set Timer priority lower than normal, because sblib interrupts have to be served with highest priority (prio = 0)
	NVIC_SetPriority(TIMER_32_0_IRQn, 1);

	timer32_0.start();
}

extern "C" void TIMER32_0_IRQHandler() {

	timer32_0.resetFlags(); /* clear MAT0 interrupt flag */

	secondCounter += 10;
	if (secondCounter == 60) {
		minuteCounter += 1;
		secondCounter = 0;
	}
	if (minuteCounter == 240) {
		minuteCounter = 0;
	}

	// falls eine Minute vorbei ist, soll geprüft werden, ob ein ComObject zyklisch gesendet werden soll
	if (secondCounter == 0) {
		oneWireChannelA.checkTimeToCyclicSend(minuteCounter);
		oneWireChannelB.checkTimeToCyclicSend(minuteCounter);
	}

	readAllSensors = true;
}
