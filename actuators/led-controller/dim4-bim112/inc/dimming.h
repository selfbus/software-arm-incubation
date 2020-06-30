/*
 * dimming.h
 *
 *  Created on: 21.02.2020
 *      Author: Uwe S.
 *
 *      -berechnet anhand der vergangenen Zeit die Dimmwerte der einzelnen Kanäle A-D oder RGBW
 *      -wird direkt von den Dimmobjekten, den Ein- Ausschaltverzögerungen usw. aufgerufen oder
 *      über die Umrechnungsfunktion HSV zu RGB
 *      -Dimmgeschwindigkeit=0 --> Wert wird direkt eingestellt
 *      -Eingang Helligkeit 0-255; Ausgang 0-MAXOUTPUTVALUE
 */

#ifndef DIMMING_H_
#define DIMMING_H_

class dimming {
public:
	bool finished = true;		// Dimmen ist beendet zum senden der Rückmeldeobj. und Relaisschalten nach Dimmende, wird nach Bearbeitung von außen wieder auf false gesetzt
	bool blocked1 = false;		// gesperrt über Sperrobjekt 1
	bool blocked2 = false;		// gesperrt über Sperrobjekt 2
	void init(int ch);
	void checkperiodic();
	void start(int destination, int speed);		// Zielhelligkeit byte 0-255 = 0-100%, Dimmgeschwindigkeit in s
	void stop();				// Auf- Abdimmen wird angehalten
	int getactualdimvalue();
	int getlastdimvalue();		// lezter Ziel-Dimmwert >0 zur Verwendung beim nächsten Einschalten wenn parametriert
	bool getswitchstatus();
	bool getIsDimming();
	bool changedtoon();
	bool changedtooff();
private:
	int channel;
	bool isOn = false;			// true wenn momentaner Dimmwert > 0
	bool isDimming = false;		// true wenn gerade auf oder abgedimmt wird, false wenn zielhelligkeit erreicht
	bool isOnOneCycle = true;	// genau 1 mal true wenn der Kanal seit dem letzten Aufruf eingeschaltet wurde
	bool isOffOneCycle = true;	// genau 1 mal true wenn der Kanal seit dem letzten Aufruf ausgeschaltet wurde
	unsigned int dimStartTime;			// Systemzeit in ms zu Dimmbeginn
	unsigned int dimStartValue;			// Helligkeit zu Dimmbeginn 0-MAXOUTPUTVALUE
	unsigned int dimDestinationValue;	// Zielhelligkeit 0-MAXOUTPUTVALUE
	unsigned int dimSpeed;				// Dimmgeschwindigkeit 0-100% in ms
	unsigned int actualDimValue = 0;		// momentaner Dimmwert 0-MAXOUTPUTVALUE
	unsigned int lastDimValue = 255;		// lezter Ziel-Dimmwert 1-255 (nach Reset 255)
};

#endif /* DIMMING_H_ */
