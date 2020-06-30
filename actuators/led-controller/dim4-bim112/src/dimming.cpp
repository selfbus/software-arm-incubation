/*
 * dimming.cpp
 *
 *  Created on: 21.02.2020
 *      Author: X
 */

#include <dimming.h>
//#include <sblib/eib.h>
#include <com_objs.h>
#include "config.h"
#include "DimmingCurves.h"
#include <sblib/serial.h>  //debugging only

// System time in milliseconds (from timer.cpp)
extern volatile unsigned int systemTime;


void dimming::init(int ch) {
	channel = ch;
}

void dimming::start(int destination, int speed) {
	if (!blocked1 && !blocked2) {
		dimStartTime = systemTime;
		dimSpeed = speed*1000; 				// speed in s, dimSpeed in ms
		dimStartValue = actualDimValue;
		dimDestinationValue = (2*MAXOUTPUTVALUE*destination/255+1)/2;  //destination 0-255, dimDestinationValue 0-MAXOUTPUTVALUE
		finished = false;
		isDimming = true;
		isOn = true;
		if (destination > 0) {
			lastDimValue = destination;
		}
		if (dimSpeed == 0) {  //Wert sofort einstellen
			actualDimValue = dimDestinationValue;
			this->stop();
			setOutput(channel, actualDimValue);
		}
	}
}

void dimming::stop() {
	finished = true;
	isDimming = false;
	if (actualDimValue > MAXOUTPUTVALUE) {		//Ausgangsgröße begrenzen
		actualDimValue = MAXOUTPUTVALUE;
	}
	if (actualDimValue <= 0) {					//Ausgangsgröße begrenzen
		actualDimValue = 0;
		isOn = false;
		isOnOneCycle = true;
		isOffOneCycle = true;
	}
}

void dimming::checkperiodic() {
	if (isDimming) {
		if (dimStartValue <= dimDestinationValue) {			//aufwärts dimmen
			int tRest = dimSpeed * (MAXOUTPUTVALUE - dimStartValue) / MAXOUTPUTVALUE;
			actualDimValue = dimStartValue + ((systemTime - dimStartTime) * (MAXOUTPUTVALUE - dimStartValue) / tRest);
			if (actualDimValue >= dimDestinationValue) {	//Zielhelligkeit erreicht
				this->stop();
			}
		} else {											//abwärts dimmen
			int tRest = dimSpeed * dimStartValue / MAXOUTPUTVALUE;
			actualDimValue = dimStartValue - ((systemTime - dimStartTime) * dimStartValue / tRest);
			// TODO minimale Helligkeit bearbeiten
			if (actualDimValue <= dimDestinationValue) {	//Zielhelligkeit erreicht
				this->stop();
			}
		}
		setOutput(channel, actualDimValue);
	}
}

int dimming::getactualdimvalue() {
	return actualDimValue*255/MAXOUTPUTVALUE;		// Rückgabe im KNX-Format 0-255
}

int dimming::getlastdimvalue() {
	return lastDimValue;
}

bool dimming::getswitchstatus() {
	return isOn;
}

bool dimming::getIsDimming() {
	return isDimming;
}

bool dimming::changedtoon() {
	if (isOn && isOnOneCycle) {
		isOnOneCycle = false;
		return true;
	}
	return false;
}

bool dimming::changedtooff() {
	if (!isOn && isOffOneCycle) {
		isOffOneCycle = false;
		return true;
	}
	return false;
}
