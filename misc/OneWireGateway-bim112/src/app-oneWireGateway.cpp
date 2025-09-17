/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/digital_pin.h>
#include <sblib/io_pin_names.h>
#include <sblib/eib/com_objects.h>

#include "config.h"
#include "oneWireChannel.h"
#include "app-OneWireGateway.h"
#include "oneWireParameters.h"
#include "ds18x20_onewire_ds2482.h"

bool readAllSensors = false;
bool searchNewSensors = false;
uint8_t minuteCounter = 0;
uint8_t secondCounter = 0;

uint8_t seqChannelCheck = 0;
uint8_t seqChannelSearch = 0;

OneWireChannel oneWireChannelA;
OneWireChannel oneWireChannelB;
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
OneWireChannel oneWireChannelC;
OneWireChannel oneWireChannelD;
#endif

void objectUpdated(int objno) {

}

void checkPeriodicFuntions(void) {
	// Alle Sensorwerte abgefragt werden
	// Die Kanäle werden sequentiell bei jedem Funktionsaufruf abgefragt,
	// um nach kurzer Zeit wieder in die Hauptfunktion zu gelangen.
	if (readAllSensors) {

		switch(seqChannelCheck){
		case 0:
			oneWireChannelA.checkPeriodic();
			seqChannelCheck = 1;
			break;
		case 1:
			oneWireChannelB.checkPeriodic();
#if NUMBER_OF_ONEWIRE_CHANNELS == 2
			seqChannelCheck = 0;
			readAllSensors = false;
			searchNewSensors = true;
			break;
#else
			seqChannelCheck = 2;
			break;
		case 2:
			oneWireChannelC.checkPeriodic();
			seqChannelCheck = 3;
			break;
		case 3:
			oneWireChannelD.checkPeriodic();
			seqChannelCheck = 0;
			readAllSensors = false;
			searchNewSensors = true;
			break;
#endif
		}

	}

	if(searchNewSensors) {
		switch(seqChannelSearch){
		case 0:
			if(oneWireChannelA.searchForNewOneWireDevices()){
				seqChannelSearch = 1;
			}
			break;
		case 1:
#if NUMBER_OF_ONEWIRE_CHANNELS == 2
			if(oneWireChannelB.searchForNewOneWireDevices()){
				seqChannelSearch = 0;
				searchNewSensors = false;
			}
			break;
#else
			if(oneWireChannelB.searchForNewOneWireDevices()){
				seqChannelSearch = 2;
			}
			break;
		case 2:
			if(oneWireChannelC.searchForNewOneWireDevices()){
				seqChannelSearch = 3;
				break;
			}
		case 3:
			if(oneWireChannelD.searchForNewOneWireDevices()){
				seqChannelSearch = 0;
				searchNewSensors = false;
			}
			break;
#endif
		}
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
	oneWireChannelA.searchInitialForOneWireDevices();
	oneWireChannelB.init(0b01);
	oneWireChannelB.searchInitialForOneWireDevices();
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
	oneWireChannelC.init(0b10);
	oneWireChannelC.searchInitialForOneWireDevices();
	oneWireChannelD.init(0b11);
	oneWireChannelD.searchInitialForOneWireDevices();
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
#if NUMBER_OF_ONEWIRE_CHANNELS == 4
		oneWireChannelC.checkTimeToCyclicSend(minuteCounter);
		oneWireChannelD.checkTimeToCyclicSend(minuteCounter);
#endif
	}

	readAllSensors = true;
}
