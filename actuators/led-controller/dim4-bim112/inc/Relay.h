/*
 * Relay.h
 *
 *  Created on: 09.03.2020
 *      Author: uwe223
 */

#ifndef RELAY_H_
#define RELAY_H_

class Relay {
public:
	/*-setzt die entspr. Portrichtungsregister
	 */
	void init();

	/* -wird periodisch aufgerufen
	 * -schaltet den Triac-Ausgang und die Ausgänge für das bistabile Relais wenn alle Kanäle 0 oder >0 sind
	 * -Verzögerung beim Abschalten in config.h einstellbar
	 */
	void handle();

	/*-schaltet die Ausgänge direkt
	 * -wird verwendet wenn Relais als extra Schaltkanal param. ist.
	 * -prüft nicht wie param. ist, weil das Kommunikationsobjekt nur dann angezeigt wird
	 */
	void onOff(int val);

private:
	enum relStates {relOff, relDelayOff, relOn, relOnImpuls, relOffImpuls};
	int relState = relOff;
	unsigned int TFTargetTime;		//Zielzeitpunkt f. Zeitfunktionen
	bool checkChannels();			//true wenn mindestens 1 Kanal ein; false wen alle Kanäle aus
};


#endif /* RELAY_H_ */
