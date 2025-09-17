/*
 *  OneWire Gateway Applikation for LPC1115
 *
 *  Copyright (C) 2021-2024 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/eib/mask0701.h>
#include <oneWireParameters.h>
#include "config.h"

extern MASK0701 bcu;

// prüfen, ob der parasitäre Versorgungsmodus aktiviert werden soll
bool getParasiteMode(uint8_t channel) {
	unsigned char configByte = bcu.userEeprom->getUInt8(EE_GENERAL_SETTINGS);

	if ((configByte >> channel) & 1) {
		return true;
	}
	return false;
}

bool getUnknownAddressSearchSend(uint8_t channel) {
	unsigned char configByte = bcu.userEeprom->getUInt8(EE_GENERAL_SETTINGS);

	if ((configByte >> (channel + NUMBER_OF_ONEWIRE_CHANNELS)) & 1) {
		return true;
	}
	return false;
}

// die in der ETS eingestellte Adresse des OneWire Geräte lesen
void getOneWireAddress(uint8_t oneWireAddress[], uint8_t channel, uint8_t deviceNumber) {
	uint32_t memoryOffset = (((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE) + EE_FIRST_ONEWIRE_ADRESS;

	// das Array leeren
	for (unsigned char byteCounter = 0; byteCounter < EE_SIZE_OF_ONEWIRE_ACSII_ADDRESS / 2; byteCounter++) {
		oneWireAddress[byteCounter] = 0;
	}

	// Adresse liegt im Speicher als ASCII Code vor (in hexadezimaler Schreibweise)
	for (unsigned char byteCounter = 0; byteCounter < EE_SIZE_OF_ONEWIRE_ACSII_ADDRESS; byteCounter++) {

		uint8_t owAddrByte = bcu.userEeprom->getUInt8(memoryOffset + byteCounter);

		// wenn 0...9 in ACSII
		if (owAddrByte >= 48 && owAddrByte <= 57) {	//Berechnung zum shiften jedes 2. Nibbles (0., 2., 4. Nibble muss um 4 Bits hochgeshiftet werden)
			oneWireAddress[byteCounter / 2] |= ( owAddrByte - 48)
					<< (4 - ((byteCounter % 2) * 4));
		}
		// wenn A...F in ASCII
		else if (owAddrByte >= 65 && owAddrByte <= 70) {
			oneWireAddress[byteCounter / 2] |= (owAddrByte - 55)
					<< (4 - ((byteCounter % 2) * 4));
		}
		// wenn a...f in ACSII
		else if (owAddrByte >= 97 && owAddrByte <= 102) {
			oneWireAddress[byteCounter / 2] |= (owAddrByte - 87)
					<< (4 - ((byteCounter % 2) * 4));
		}
	}
}

// den OneWire Gerätetyp lesen (zur Zeit nur DS18X20 Geräte möglich)
unsigned char getDeviceType(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;

	unsigned char configByte = bcu.userEeprom->getUInt8(EE_FIRST_ONEWIRE_CONFIG_BYTE + memoryOffset);

	return ((configByte >> 4) & 0x0F);
}

// prüfen, ob zyklisch gesendet werden soll
bool checkCyclicSend(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;

	unsigned char configByte = bcu.userEeprom->getUInt8(EE_FIRST_ONEWIRE_CONFIG_BYTE + memoryOffset);

	if ((configByte >> 3) & 1) {
		return true;
	}
	return false;
}

// prüfen, ob nach Änderung des Messwertes gesendet werden soll
bool checkValueChangeSend(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;

	unsigned char configByte = bcu.userEeprom->getUInt8(EE_FIRST_ONEWIRE_CONFIG_BYTE + memoryOffset);

	if ((configByte >> 2) & 1) {
		return true;
	}
	return false;
}

// die Zykluszeit für zyklisches Versenden lesen
uint8_t getCyclicTime(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;

	return bcu.userEeprom->getUInt8(EE_FIRST_CYCLIC_TIME_BYTE + memoryOffset);
}

// den Wert holen, um den sich der Messwert ändern muss um gesendet zu werden
float getChangeValue(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;

	// der Wert liegt im EEPROM als DPT9 codiert vor
	unsigned short dpt9CodedValue = bcu.userEeprom->getUInt16(EE_FIRST_VALUE_CHANGE_VALUE + memoryOffset);

	return dpt9ToFloat(dpt9CodedValue);
}

// den Wert holen, um den der Sensorwert korrigiert werden soll
float getCorrectionValue(uint8_t channel, uint8_t deviceNumber) {
	int memoryOffset = ((channel * MAX_NUMBER_OF_ONEWIRE_DEVICES) + deviceNumber) * EE_SIZE_OF_EACH_ONEWIRE_DEVICE;
	unsigned short dpt9CodedValue;

	dpt9CodedValue = bcu.userEeprom->getUInt16(EE_FIRST_VALUE_CORRECT_VALUE + memoryOffset);

	return dpt9ToFloat(dpt9CodedValue);
}

void turnOneWireAddress(uint8_t originalAddress[], uint8_t turnedAddress[]) {
	for (uint8_t byteCounter = 0; byteCounter < 8; byteCounter++) {
		turnedAddress[7 - byteCounter] = originalAddress[byteCounter];
	}
}
