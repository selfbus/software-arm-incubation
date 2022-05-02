/*
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *                2015 Deti Fliegl <deti@fliegl.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "convert_hsv_rgb.h" // includes <math.h> so it needs to be included before sblib's math.h
#include <Appl.h>
#include <sblib/timeout.h>
#include <com_objs.h>
#include <config.h>
#include "pwmout.h"
#include "dimming.h"
#include "Timefunctions.h"
#include "Relay.h"
#ifdef DEBUG
#   include <sblib/serial.h>  //debugging only
#endif

MASK0701 bcu = MASK0701();

#ifdef PWM
pwmout pwmout[4];
#endif

dimming dimming[4];
Timefunctions timefunctions[4];
Relay relay;

void initApplication(void) {
	Timeout startupDelay;
	// delay in config is in seconds
	unsigned int delay = bcu.userEeprom->getUInt16(APP_STARTUP_TIMEOUT) * 1000;
	startupDelay.start(delay);
	if (delay) {
		while (!startupDelay.expired()) {
			waitForInterrupt();
		}
	}

#ifdef PWM
	for (unsigned int i=0; i<currentVersion->noOfChannels; i++) {
		pwmout[i].begin(i);
	}
#endif
	for (int i = 0; i < 4; i++) {
		dimming[i].init(i);
		timefunctions[i].init(i);
	}
	relay.init();
}

void objectUpdated(int objno) {
	if (objno < OBJ_CENTRAL_BASE) {    		// Kanalabhängige Funktionen
		int objchannel = objno / OFSCHANNELOBJECTS;
		int objfunction = objno % OFSCHANNELOBJECTS;
		switch (objfunction) {
		case OBJ_SWITCH:
			timefunctions[objchannel].objSwitch(bcu.comObjects->objectRead(objno));
			break;
		case OBJ_STAIRS:
			timefunctions[objchannel].objStairs(bcu.comObjects->objectRead(objno));
			break;
		case OBJ_DIM_REL:
			handleRelDimmingObject(bcu.comObjects->objectRead(objno), objchannel);
			break;
		case OBJ_DIM_ABS:
			handleAbsDimmingObject(bcu.comObjects->objectRead(objno), objchannel);
			break;
		case OBJ_BLOCKING1:
			handleBlocking1Object(bcu.comObjects->objectRead(objno), objchannel);
			break;
		case OBJ_BLOCKING2:
			handleBlocking2Object(bcu.comObjects->objectRead(objno), objchannel);
			break;
		default:
		    bcu.comObjects->objectRead(objno);
			break;
		}
	} else {					    		// Zentralfunktionen
		switch (objno) {
		case OBJ_C_SWITCH:
			for (int ch = 0; ch < 4; ch++) {
				if (centralObjectsActive(ch)) {
					timefunctions[ch].objSwitch(bcu.comObjects->objectRead(objno));
				}
			}
			break;
		case OBJ_C_DIM_ABS:
			for (int ch = 0; ch < 4; ch++) {
				if (centralObjectsActive(ch)) {
					handleAbsDimmingObject(bcu.comObjects->objectRead(objno), ch);
				}
			}
			break;
		case OBJ_RGBW_SWITCH:
			for (int ch = 0;
					ch < 2 + (bcu.userEeprom->getUInt8(APP_FUNCTION_SEL) & 0b11);
					ch++) {	//ch<3 bei RGB-Mode; ch<4 bei RGBW-Mode
				timefunctions[ch].objSwitch(bcu.comObjects->objectRead(objno));
			}
			break;
		case OBJ_RGBW_COLOR:
			handleRGBAbsDimmingObject(bcu.comObjects->objectRead(objno));
			break;
		case OBJ_HSV_COLOR:
			handleHSVAbsDimmingObject(bcu.comObjects->objectRead(objno));
			break;
		case OBJ_C_RELAY:
			relay.onOff(bcu.comObjects->objectRead(objno));
			break;
		default:
		    bcu.comObjects->objectRead(objno);
			break;
		}
	}
}

void checkPeriodic(void) {
	for (int ch = 0; ch < 4; ch++) {
		timefunctions[ch].checkTimeout();
		dimming[ch].checkperiodic();
		handleBusReturn(ch);

	}
	relay.handle();
}

//*****************************************************
//Objekte
//*****************************************************

void handleRelDimmingObject(int objectValue, int channel) {
	if (!(objectValue & 0b00000111))
	{
        // Stop telegram
        dimming[channel].stop();
        //SendBrightness();
	    return;
	}

    // auf oder abdimmen
    int dimmSteps = 255;
    int destinationValue;

    /* Schritte für Dimmbefehle 1,5% - 100% ausrechnen */
    for (uint8_t i = 1; i < (objectValue & 0b00000111); i++) {
        dimmSteps >>= 1;
    }
    if (objectValue & 0b00001000) {
        /* aufdimmen */
        /* Zielhelligkeit setzen wenn Maximum nicht überschritten wird */
        destinationValue =
                255 - dimming[channel].getactualdimvalue() > dimmSteps ?
                        dimming[channel].getactualdimvalue() + dimmSteps :
                        255;
    } else {
        /* abdimmen */
        /* Zielhelligkeit setzen wenn 0 oder 1 nicht unterschritten wird */
        int min = 1;
        if (bcu.userEeprom->getUInt8(APP_SW_OFF_REL + channel * APP_CH_OFFS) & APP_SW_OFF_REL_M) {
            min = 0;  // Ausschalten bei relativ Dimmen
        }
        destinationValue =
                dimming[channel].getactualdimvalue() > dimmSteps ?
                        dimming[channel].getactualdimvalue() - dimmSteps :
                        min;
    }
    dimming[channel].start(destinationValue,
            bcu.userEeprom->getUInt8(APP_REL_SPEED + channel * APP_CH_OFFS));
}

void handleAbsDimmingObject(int objectValue, int channel) {
	dimming[channel].start(objectValue,
			bcu.userEeprom->getUInt8(APP_ABS_SPEED + channel * APP_CH_OFFS));
}

void handleRGBAbsDimmingObject(int objectValue) {
	for (int ch = 2; ch >= 0; ch--) {
		dimming[ch].start(objectValue & 0xFF,
				bcu.userEeprom->getUInt16(APP_RGB_ABS_SPEED));
		objectValue >>= 8;
	}
}

void handleHSVAbsDimmingObject(int objectValue) {
	unsigned char vVal = objectValue & 0xFF;
	objectValue >>= 8;
	unsigned char sVal = objectValue & 0xFF;
	objectValue >>= 8;
	unsigned char hVal = objectValue & 0xFF;
	storedHueValue = hVal;		// Farbwert speichern
	unsigned char rVal, gVal, bVal;
	hsv2rgb(hVal, sVal, vVal, rVal, gVal, bVal);
	dimming[0].start(rVal, bcu.userEeprom->getUInt16(APP_RGB_ABS_SPEED));
	dimming[1].start(gVal, bcu.userEeprom->getUInt16(APP_RGB_ABS_SPEED));
	dimming[2].start(bVal, bcu.userEeprom->getUInt16(APP_RGB_ABS_SPEED));
}

void handleBlocking1Object(int objectValue, int channel) {
	if (objectValue) {
		switch ((bcu.userEeprom->getUInt8(APP_BLOCK1_BEHAV + channel * APP_CH_OFFS)
				& 0xF0) >> 4) {		// Verhalten bei Sperrobjekt 1 = 1
		case 0:		//aus
			dimming[channel].start(0,
					bcu.userEeprom->getUInt8(APP_OFF_SPEED + channel * APP_CH_OFFS)); //es wird mit "Ausschaltgeschwindigkeit" auf 0 gedimmt
			break;
		case 3:		//parametrierter Helligkeitswert
			dimming[channel].start(
					bcu.userEeprom->getUInt8(
							APP_BLOCK1_1_VAL + channel * APP_CH_OFFS),
					bcu.userEeprom->getUInt8(APP_ABS_SPEED + channel * APP_CH_OFFS)); //es wird mit Dimmgeschw. absolutes dimmen auf den parametrierten Wert gedimmt
			break;
		default:	//case 2: keine Änderung
			break;
		}
		dimming[channel].blocked1 = true;
	} else {
		dimming[channel].blocked1 = false;
		switch (bcu.userEeprom->getUInt8(APP_BLOCK1_BEHAV + channel * APP_CH_OFFS)
				& 0x0F) {		// Verhalten bei Sperrobjekt 1 = 0
		case 0:		//aus
			dimming[channel].start(0,
					bcu.userEeprom->getUInt8(APP_OFF_SPEED + channel * APP_CH_OFFS)); //es wird mit "Ausschaltgeschwindigkeit" auf 0 gedimmt
			break;
		case 3:		//parametrierter Helligkeitswert
			dimming[channel].start(
					bcu.userEeprom->getUInt8(
							APP_BLOCK1_0_VAL + channel * APP_CH_OFFS),
					bcu.userEeprom->getUInt8(APP_ABS_SPEED + channel * APP_CH_OFFS)); //es wird mit Dimmgeschw. absolutes dimmen auf den parametrierten Wert gedimmt
			break;
		default:	//case 2: keine Änderung
			break;
		}
	}
}

void handleBlocking2Object(int objectValue, int channel) {
	if (objectValue) {
		switch ((bcu.userEeprom->getUInt8(APP_BLOCK2_BEHAV + channel * APP_CH_OFFS)
				& 0xF0) >> 4) {		// Verhalten bei Sperrobjekt 2 = 1
		case 0:		//aus
			dimming[channel].start(0,
					bcu.userEeprom->getUInt8(APP_OFF_SPEED + channel * APP_CH_OFFS)); //es wird mit "Ausschaltgeschwindigkeit" auf 0 gedimmt
			break;
		case 3:		//parametrierter Helligkeitswert
			dimming[channel].start(
					bcu.userEeprom->getUInt8(
							APP_BLOCK2_1_VAL + channel * APP_CH_OFFS),
					bcu.userEeprom->getUInt8(APP_ABS_SPEED + channel * APP_CH_OFFS)); //es wird mit Dimmgeschw. absolutes dimmen auf den parametrierten Wert gedimmt
			break;
		default:	//case 2: keine Änderung
			break;
		}
		dimming[channel].blocked2 = true;
	} else {
		dimming[channel].blocked2 = false;
		switch (bcu.userEeprom->getUInt8(APP_BLOCK2_BEHAV + channel * APP_CH_OFFS)
				& 0x0F) {		// Verhalten bei Sperrobjekt 2 = 0
		case 0:		//aus
			dimming[channel].start(0,
					bcu.userEeprom->getUInt8(APP_OFF_SPEED + channel * APP_CH_OFFS)); //es wird mit "Ausschaltgeschwindigkeit" auf 0 gedimmt
			break;
		case 3:		//parametrierter Helligkeitswert
			dimming[channel].start(
					bcu.userEeprom->getUInt8(
							APP_BLOCK2_0_VAL + channel * APP_CH_OFFS),
					bcu.userEeprom->getUInt8(APP_ABS_SPEED + channel * APP_CH_OFFS)); //es wird mit Dimmgeschw. absolutes dimmen auf den parametrierten Wert gedimmt
			break;
		default:	//case 2: keine Änderung
			break;
		}
	}
}

//*****************************************************
//Objekte Ende
//*****************************************************

int lastTransmit[4] { 0,0,0,0 };
void handleBusReturn(int channel){
	//Schaltrückmeldeobjekt & Kanalstatus vergleichen und senden
	if (bcu.comObjects->objectRead(channel * OFSCHANNELOBJECTS + OBJ_STAT_SWITCH) != dimming[channel].getswitchstatus())
	{
		bcu.comObjects->objectWrite(channel * OFSCHANNELOBJECTS + OBJ_STAT_SWITCH, dimming[channel].getswitchstatus());
	}

	if (dimming[channel].finished || (dimming[channel].getIsDimming() && lastTransmit[channel] + 1000 < millis()))
	{
		lastTransmit[channel] = millis();
		bcu.comObjects->objectWrite(channel * OFSCHANNELOBJECTS + OBJ_STAT_DIM,
				dimming[channel].getactualdimvalue());
		if (!dimming[0].getIsDimming()
				&& !dimming[1].getIsDimming()
				&& !dimming[2].getIsDimming())
		{// Wert nur senden wenn alle 3 Kanäle fertig sind
			unsigned int rgbValue = (dimming[0].getactualdimvalue() << 16)
							| (dimming[1].getactualdimvalue() << 8)
							| dimming[2].getactualdimvalue();
			bcu.comObjects->objectWrite(OBJ_STAT_RGBW, rgbValue);
		}
		dimming[channel].finished = false;
	}
}

bool centralObjectsActive(int channel) {
	return bcu.userEeprom->getUInt8(APP_CENT_OBJ_ACT + channel * 0x05) & 0x01;
}
