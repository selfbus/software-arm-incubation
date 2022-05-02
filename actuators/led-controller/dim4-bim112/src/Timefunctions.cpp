/*
 * Timefunctions.cpp
 *
 *  Created on: 25.02.2020
 *      Author: x
 */
#include <sblib/timeout.h>
#include <com_objs.h>
#include <dimming.h>
#include <Timefunctions.h>
#include "Appl.h"

#ifdef DEBUG
#   include <sblib/serial.h>  //debugging only
#endif

extern dimming dimming[];

void Timefunctions::init(int ch) {
	channel = ch;
}

void Timefunctions::checkTimeout() {
	if (timeout.started() && timeout.expired()) {
		switch (TimeFctState) {
		case DelayOn:
			this->switchOn();
			TimeFctState = Idle;
			break;
		case DelayOff:
			this->switchOff();
			break;
		case StairDelayOn:
			TimeFctState = StairOn;
			this->switchOn();
			timeout.start(bcu.userEeprom->getUInt16(APP_STAIR_DUR + channel * APP_CH_OFFS) * 1000 + 1);
			break;
		case StairOn:
			TimeFctState = StairWarn;
			this->switchOn();
			timeout.start(bcu.userEeprom->getUInt16(APP_STAIR_PREWARN + channel * APP_CH_OFFS) * 1000 + 1);
			break;
		case StairWarn:
			this->switchOff();
			break;
		default:			//case Idle :
			break;
		}
	}
}

void Timefunctions::objSwitch(int objVal) {
	if (objVal == dimming[channel].getswitchstatus()) {
		return;
	}
	if (objVal && (TimeFctState != DelayOn)) {
		timeout.start(bcu.userEeprom->getUInt16(APP_ON_DELAY + channel * APP_CH_OFFS) * 1000 + 1);//Einschaltverzögerung in sek.; +1 weil timeout mit 0 nicht startet
		TimeFctState = DelayOn;
	}
	if (!objVal && (TimeFctState != DelayOff)) {
		timeout.start(bcu.userEeprom->getUInt16(APP_OFF_DELAY + channel * APP_CH_OFFS) * 1000 + 1);//Ausschaltverzögerung in sek.; +1 weil timeout mit 0 nicht startet
		TimeFctState = DelayOff;
	}
}

void Timefunctions::objStairs(int objVal) {
	if (objVal) {
#ifdef DEBUG
        //serial.println(bcu.userEeprom->getUInt8(APP_STAIR_EXTENSION + channel * APP_CH_OFFS));
#endif
		switch (TimeFctState) {
		case Idle:
			timeout.start(bcu.userEeprom->getUInt16(APP_ON_DELAY + channel * APP_CH_OFFS) * 1000 + 1);
			TimeFctState = StairDelayOn;
			break;
		case StairOn:
		case StairWarn:
			if (bcu.userEeprom->getUInt8(APP_STAIR_EXTENSION + channel * APP_CH_OFFS) & APP_STAIR_EXTENSION_M) {
				//wenn Treppenlicht verlängern ein
				timeout.start(bcu.userEeprom->getUInt16(APP_STAIR_DUR + channel * APP_CH_OFFS) * 1000 + 1);
				TimeFctState = StairOn;
				this->switchOn();
			}
			break;
		default:
			break;
		}
	} else {
		if (bcu.userEeprom->getUInt8(APP_STAIR_DEACT + channel * APP_CH_OFFS) & APP_STAIR_DEACT_M) {
					//wenn Treppenlicht vorzeitig abgeschaltet werden darf
			this->switchOff();
		}
	}
}

void Timefunctions::switchOff() {
	dimming[channel].start(0,
			bcu.userEeprom->getUInt8(APP_OFF_SPEED + channel * APP_CH_OFFS));
	TimeFctState = Idle;
}

void Timefunctions::switchOn() {
	int dimmValue;
	switch (TimeFctState) {
	case StairOn:
		dimmValue = 0xFF;	//Treppenlicht wird beim Einschalten immer auf 100% gedimmt
		break;
	case StairWarn:
		dimmValue = bcu.userEeprom->getUInt8(APP_STAIR_DIM + channel * APP_CH_OFFS);
		break;
	default:
		if (bcu.userEeprom->getUInt8(APP_START_BEHAVIOUR + channel * APP_CH_OFFS) & APP_START_BEHAVIOUR_M) {
			dimmValue = dimming[channel].getlastdimvalue();
		} else {
			dimmValue = bcu.userEeprom->getUInt8(APP_START_UP_VAL + channel * APP_CH_OFFS);
		}
		break;
	}
	dimming[channel].start(dimmValue,
			bcu.userEeprom->getUInt8(APP_ON_SPEED + channel * APP_CH_OFFS));
}


















