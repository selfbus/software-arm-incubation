/*
 * Timefunctions.h
 *
 *  Created on: 25.02.2020
 *      Author: x
 */

#ifndef TIMEFUNCTIONS_H_
#define TIMEFUNCTIONS_H_


//enum TimeFctStates : byte {Idle, StairOn, StairWarn, DelayOn, DelayOff};

class Timefunctions {
public:
	void init(int ch);

	/* -wird periodisch aufgerufen
	 * -prüft ob Zeit abgelaufen ist
	 * -setzt TimeFctState und führt Dimmbefehl aus
	 */
	void checkTimeout();

	void objSwitch(int objVal);
	void objStairs(int objVal);

private:
	enum TimeFctStates {Idle, StairDelayOn, StairOn, StairWarn, DelayOn, DelayOff};
	int TimeFctState = Idle;
	int channel;
	int dimmValue;				// Wert auf den nach Ablauf des Timers gedimmt wird
	Timeout timeout;

	/* -dimmt mit ausschaltgeschw. auf 0
	 * -setzt TimeFctState = Idle
	 */
	void switchOff();

	/* - dimmt je nach Zustand von TimeFctState auf die entspr. Helligkeit
	 * - prüft ob gespeicherte Helligkeit param ist
	 */
	void switchOn();
};

#endif /* TIMEFUNCTIONS_H_ */
