/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef __ONEWIRECHANNELS_H__
#define __ONEWIRECHANNELS_H__

#include <sblib/eib.h>
#include "ds18x20_onewire_ds2482.h"
#include "oneWireParameters.h"
#include "config.h"



class OneWireChannel {
public:
	void init(uint8_t i2cAddress);
	void checkPeriodic();
	void cyclicSend();
	void checkTimeToCyclicSend(uint8_t minuteCounter);


private:
	void sendTemperatureObject(uint8_t onewireDeviceCount);
	void updateComObjectValue(uint8_t onewireDeviceCount);

	DS18x20_OneWireDS2482 ds18x20;

	void searchForNewOneWireDevices();

	// die Kanal Nummer (ist auch gleich der I2C Adresse des DS2482)
	uint8_t channelNumber;

	// in dieserVariablen wird die Zuordnung der gefundenen OneWire Geräte zu den in der App eingestellten Geräten gespeichert
	// Der Index des Feldes bezieht sich auf den Index der am Bus gefundenen Geräte, der Inhalt des Feldes stellt die in der App verwendete Gerätenummer dar.
	uint8_t oneWireAssociationTable[MAX_DS_DEVICES];

	// in diesem Feld wird die Anforderung zur Versendung eines Wertes abgelegt. Die Anforderung wird durch den Ablauf der Zykluszeit bestimmt.
	bool cyclicObjectSend[MAX_NUMBER_OF_ONEWIRE_DEVICES];

	// der Index des ersten unbekannten am OneWire Bus gefunden Gerätes wird hier abgelegt (es darf sich maximal ein unbekanntes Gerät am Bus befinden)
	//uint8_t unknownOneWireAddressPointer = 0xFF;

	// die letzten gesendeten Werte werden für Vergleiche hier gespeichert. Es wird nach den in der App eingestellten Devices indiziert
	// der Grund für diese Indizierung ist die Möglichkeit einer anderen Reihenfolge der gefundenen Geräte, nachdem ein Gerät im Beriteb hinzugefügt wurde
	float lastSendedValue[MAX_NUMBER_OF_ONEWIRE_DEVICES];

	// die letzte
	byte lastSendUnknownAddr[8];
};

#endif /* __ONEWIRECHANNELS_H__ */
