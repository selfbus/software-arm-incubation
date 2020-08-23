/*
 * Relay.cpp
 *
 *  Created on: 09.03.2020
 *      Author: uwe223
 */

#include "Relay.h"
#include "config.h"
#include "dimming.h"
#include "com_objs.h"
#include <sblib/core.h>
#include <sblib/timeout.h>

extern dimming dimming[];
Timeout RelayTimeout;

void Relay::init() {
	pinMode(RELON, OUTPUT);
	pinMode(RELOFF, OUTPUT);
	pinMode(TRIAC, OUTPUT);
	digitalWrite(RELON, 0);
	digitalWrite(RELOFF, 0);
	digitalWrite(TRIAC, 0);
}

void Relay::handle() {
	if (userEeprom.getUInt8(APP_USE_RELAY)) { //Relais schaltet wenn Kanäle ein/aus
		switch (relState) {
		case relDelayOff: //alle Kanäle sind 0, Verzögerungszeit bis zum abschalten läuft
			if (this->checkChannels()) {
				//prüfen ob zwischenzeitlich wieder hochgedimmt wurde
				RelayTimeout.stop();
				relState = relOn;
				break;
			}
			if (RelayTimeout.started() && RelayTimeout.expired()) {
				digitalWrite(RELOFF, 1);
				digitalWrite(TRIAC, 0);
				relState = relOffImpuls;
				RelayTimeout.start(RELAYPULSEDURATION);
			}
			break;
		case relOn:				//mindestens 1 Kanal ist nicht 0
			if (!this->checkChannels()) {
				//wenn alle Kanäle 0 --> Ausschalttimer starten
				RelayTimeout.start(RELAYOFFDELAY);
				relState = relDelayOff;
			}
			break;
		case relOnImpuls://Impulsausgang füe bistabiles Relais einschalten ist aktiv
			if (RelayTimeout.expired()) {
				digitalWrite(RELON, 0);
				relState = relOn;
			}
			break;
		case relOffImpuls://Impulsausgang füe bistabiles Relais ausschalten ist aktiv
			if (this->checkChannels()) {
				//prüfen ob zwischenzeitlich wieder hochgedimmt wurde
				digitalWrite(RELON, 1);
				digitalWrite(TRIAC, 1);
				relState = relOnImpuls;
				RelayTimeout.start(RELAYPULSEDURATION);
				break;
			}
			if (RelayTimeout.expired()) {
				digitalWrite(RELOFF, 0);
				relState = relOff;
			}
			break;
		default:				//case relOff:	//Relais ist aus
			if (this->checkChannels()) {
				//mind. 1 Kanal ein --> Relais einschalten
				digitalWrite(RELON, 1);
				digitalWrite(TRIAC, 1);
				relState = relOnImpuls;
				RelayTimeout.start(RELAYPULSEDURATION);
			}
			break;
		}
	} else {							//extra Schaltkanal
		switch (relState) {
		case relOnImpuls://Impulsausgang füe bistabiles Relais einschalten ist aktiv
			if (RelayTimeout.expired()) {
				digitalWrite(RELON, 0);
				relState = relOn;
			}
			break;
		case relOffImpuls://Impulsausgang füe bistabiles Relais ausschalten ist aktiv
			if (RelayTimeout.expired()) {
				digitalWrite(RELOFF, 0);
				relState = relOff;
			}
			break;
		default:				//Relais ist an oder aus
			break;
		}
	}
}

void Relay::onOff(int val) {
	if (val) {
		digitalWrite(RELON, 1);
		digitalWrite(TRIAC, 1);
		relState = relOnImpuls;
		RelayTimeout.start(RELAYPULSEDURATION);
	} else {
		digitalWrite(RELOFF, 1);
		digitalWrite(TRIAC, 0);
		relState = relOffImpuls;
		RelayTimeout.start(RELAYPULSEDURATION);
	}
}

bool Relay::checkChannels() {
	for (int ch = 0; ch < 4; ch++) {
		if (dimming[ch].getswitchstatus()) {
			return true;
		}
	}
	return false;
}
