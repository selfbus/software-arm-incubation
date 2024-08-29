/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eib/mask0701.h>
#include <sblib/eib/com_objects.h>
#include <oneWireChannel.h>

extern MASK0701 bcu;

void OneWireChannel::init(uint8_t i2cAddress) {
	bool parasiteMode = getParasiteMode(channelNumber);
	ds18x20.DS18x20Init(parasiteMode, i2cAddress);
	channelNumber = i2cAddress;
}

void OneWireChannel::cyclicSend() {

	// alle in der App eingestellten OneWire Geräte durchgehen
	for (uint8_t appDeviceCount = 0; appDeviceCount < MAX_NUMBER_OF_ONEWIRE_DEVICES; appDeviceCount++) {
		// wenn der Wert zyklisch gesendet werden soll
		if (cyclicObjectSend[appDeviceCount] == true) {
			cyclicObjectSend[appDeviceCount] = false;
			// alle gefundenen OneWire geräte durchgehen
			for (uint8_t onewireDeviceCount = 0; onewireDeviceCount < ds18x20.m_foundDevices; onewireDeviceCount++) {
				if (oneWireAssociationTable[onewireDeviceCount] == appDeviceCount) {
					sendTemperatureObject(onewireDeviceCount);
				}
			}
		}
	}
}

void OneWireChannel::checkPeriodic() {

	ds18x20.Search(MAX_NUMBER_OF_ONEWIRE_DEVICES);

	// neue Geräte suchen und zu den in der App eingestellten Adressen zuordnen
	searchForNewOneWireDevices();

	// alle DS18X20 Temperatursensoren lesen
	ds18x20.readTemperatureAll();

	// alle gefundenen OneWire geräte durchgehen
	for (uint8_t onewireDeviceCount = 0; onewireDeviceCount < ds18x20.m_foundDevices; onewireDeviceCount++) {
		if (oneWireAssociationTable[onewireDeviceCount] != 0xFF) {
			updateComObjectValue(onewireDeviceCount);
			// wenn der Wert für dieses Gerät nach Wertänderung gesendet werden soll
			if (checkValueChangeSend(channelNumber, oneWireAssociationTable[onewireDeviceCount])) {
				float changeValue = getChangeValue(channelNumber, oneWireAssociationTable[onewireDeviceCount]) / 100;
				float diffValue = lastSendedValue[oneWireAssociationTable[onewireDeviceCount]]
						- ds18x20.m_dsDev[onewireDeviceCount].last_temperature;
				if (diffValue < 0) {
					diffValue = -diffValue;
				}
				if (diffValue > changeValue) {
					sendTemperatureObject(onewireDeviceCount);
				}
			}
		}
	}
}

// Es wird die aktuell gemessene Temperatur mit dem Korekturwert verrechnet und versendet
// Parameter:
//   onewireDeviceCount: die Nummer des gefundenen Gerätes (nicht die in der App eingestellte Nummer)
void OneWireChannel::sendTemperatureObject(uint8_t onewireDeviceCount) {
	lastSendedValue[oneWireAssociationTable[onewireDeviceCount]] = ds18x20.m_dsDev[onewireDeviceCount].last_temperature;
	float sensorValue = ds18x20.m_dsDev[onewireDeviceCount].last_temperature;
	float correctionValue = getCorrectionValue(channelNumber, oneWireAssociationTable[onewireDeviceCount]) / 100;

	// Als Offset die Anzahl der Kanäle hinzurechnen, da diese Anzahl als ComObjekte für die Adressenversendung vorgehalten wird
	bcu.comObjects->objectWriteFloat(
			oneWireAssociationTable[onewireDeviceCount] + NUMBER_OF_ONEWIRE_CHANNELS
					+ (channelNumber * MAX_NUMBER_OF_ONEWIRE_DEVICES), (sensorValue + correctionValue) * 100);
}

// Es wird die aktuell gemessene Temperatur mit dem Korekturwert verrechnet und in der sblib hinterlegt (damit der aktuelle Sensorwert gelesen werden kann)
// Parameter:
//   onewireDeviceCount: die Nummer des gefundenen Gerätes (nicht die in der App eingestellte Nummer)
void OneWireChannel::updateComObjectValue(uint8_t onewireDeviceCount) {
	float sensorValue = ds18x20.m_dsDev[onewireDeviceCount].last_temperature;
	float correctionValue = getCorrectionValue(channelNumber, oneWireAssociationTable[onewireDeviceCount]) / 100;

	// Als Offset die Anzahl der Kanäle hinzurechnen, da diese Anzahl als ComObjekte für die Adressenversendung vorgehalten wird
	bcu.comObjects->objectUpdateFloat(
			oneWireAssociationTable[onewireDeviceCount] + NUMBER_OF_ONEWIRE_CHANNELS
					+ (channelNumber * MAX_NUMBER_OF_ONEWIRE_DEVICES), (sensorValue + correctionValue) * 100);
}

// Es wird geprüft, welches ComObject zyklisch gesendet werden soll und ob der Zyklus abgelaufen ist
// Parameter:
//		minuteCounter: die Anzahl der Minuten
void OneWireChannel::checkTimeToCyclicSend(uint8_t minuteCounter) {
	// alle in der App eingestellten OneWire Geräte durchgehen
	for (uint8_t appDeviceCount = 0; appDeviceCount < MAX_NUMBER_OF_ONEWIRE_DEVICES; appDeviceCount++) {
		// wenn der Wert für dieses Gerät zyklisch gesendet werden soll
		if (checkCyclicSend(channelNumber, appDeviceCount)) {
			uint8_t cyclicSendTime = getCyclicTime(channelNumber, appDeviceCount);
			if ((minuteCounter % cyclicSendTime) == 0) {
				cyclicObjectSend[appDeviceCount] = true;
			}
		}
	}
}

// Es werden neue Geräte gesucht und zu den in der App eingestellten Adressen zugeordnet
// Falls eine gefundene Adresse nciht in den Paametern gefunden werden kann, wird sie mit dem entprechenden ComObject versendet
void OneWireChannel::searchForNewOneWireDevices() {
	for (uint8_t onewireDeviceCount = 0; onewireDeviceCount < MAX_DS_DEVICES; onewireDeviceCount++) {
		oneWireAssociationTable[onewireDeviceCount] = 0xFF; // Initialisieren aller Felder
	}

	// alle in der App eingestellten OneWire Geräte durchgehen
	for (uint8_t appDeviceCount = 0; appDeviceCount < MAX_NUMBER_OF_ONEWIRE_DEVICES; appDeviceCount++) {

		uint8_t appDeviceAddress[8];
		getOneWireAddress(appDeviceAddress, channelNumber, appDeviceCount);

		// alle gefundenen OneWire geräte durchgehen
		for (uint8_t onewireDeviceCount = 0; onewireDeviceCount < ds18x20.m_foundDevices; onewireDeviceCount++) {

			uint8_t *onewireDeviceAddress = ds18x20.m_dsDev[onewireDeviceCount].addr;

			// Vergleichen, ob Adressen übereinstimmen
			bool equalAddress = true;
			for (uint8_t byteCounter = 0; byteCounter < 8; byteCounter++) {

				uint8_t addDevAddr = appDeviceAddress[byteCounter];
				uint8_t owDevAddr = onewireDeviceAddress[byteCounter];

				if (addDevAddr != owDevAddr) {
					equalAddress = false;
					break;
				}
			}

			if (equalAddress) {
				oneWireAssociationTable[onewireDeviceCount] = appDeviceCount;
				break;
			}
		}
	}

	bool unknownDeviceFound = false;
	// wenn zu einem am Bus gefundenen Objekt keine passende Adresse aus der ETS gefunden wurde, wird diese versendet
	for (uint8_t onewireDeviceCount = 0; onewireDeviceCount < ds18x20.m_foundDevices; onewireDeviceCount++) {
		if (oneWireAssociationTable[onewireDeviceCount] == 0xFF) {
			unknownDeviceFound = true;

			uint8_t *actualAddress = ds18x20.m_dsDev[onewireDeviceCount].addr;
			bool addressIsToSend = true;

			// wenn die Adresse nicht zyklisch gesendet werden soll, soll sie nur einmal versendet werden
			// dazu wird verglichen, ob die Adresse breits versendet wurde
			if (!getUnknownAddressCyclicSend(channelNumber)) {
				addressIsToSend = false;
				for (uint8_t byteCounter = 0; byteCounter < 8; byteCounter++) {
					if (actualAddress[byteCounter] != lastSendUnknownAddr[byteCounter]) {
						addressIsToSend = true;
						break;
					}
				}
			}

			if (addressIsToSend) {
				// die Adresse muss umgedreht werden, damit Sie korrekt in der ETS dargestellt wird
				uint8_t turnedAddress[8];
				turnOneWireAddress(actualAddress, turnedAddress);
				bcu.comObjects->objectWrite(channelNumber, turnedAddress);
				for (uint8_t byteCounter = 0; byteCounter < 8; byteCounter++) {
					lastSendUnknownAddr[byteCounter] = actualAddress[byteCounter];
				}
			}
			break;
		}
	}

	// falls kein unbekanntes OneWire Gerät gefunden wurde, soll die Adresse im jeweiligen Adress-ComObjekt genullt werden
	// somit kann per ETS eindeutig festgestellt werden ob sich noch ein unbekanntes Gerät am Bus befindet
	if(!unknownDeviceFound){
		uint8_t nullAddress[8] = {0,0,0,0,0,0,0,0};
		bcu.comObjects->objectUpdate(channelNumber, nullAddress);
	}
}
