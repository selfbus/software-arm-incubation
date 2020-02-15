/*
 *  Copyright (c) 2016 Oliver Stefan <MAIL>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef SENSORS_H
#define SENSORS_H

#include <sblib/i2c/SHT2x.h>
#include <sblib/i2c/CCS811.h>

extern SHT2xClass SHT21;

extern CCS811Class CCS811;

struct air_quality_values{
	bool functionActive;		//air quality functions are active

	unsigned int sendIntervall;

	unsigned int AirVOC;				//VOC in Air
	unsigned int AirCO2;				//CO2 equivalent of VOC
	uint8_t IAQcondition;	//current status of IAQ (good, neutral, bad)
};

enum IAQconditions{
	IAQgood,
	IAQneutral,
	IAQbad
};

extern struct air_quality_values air_quality;


struct air_humidity_values{
	bool functionActive;		//air humidity functions are active

	unsigned int sendIntervall;

//	unsigned short AirDewPoint;
	int AirDewPoint;

//	unsigned short AirRH;
	int AirRH;

};

extern struct air_humidity_values air_humidity;

// value to check if the temperature sensor is connected (Application Board is connected)
extern bool TempSensAvailable;

enum SollTemp {
	SollTempIntern,
	SollTempExtern
};

enum connExtTemp {
	ExtTempOverKNX,
	ExtTempAtBoard
};

struct temp_values{
	unsigned int functionActive;	//temperature functions are active

	unsigned int sendInterval;

//	unsigned int autoResetTime; 	// Zeit, nach der von Hand- auf Automatik-Temperaturvorgabe zurückgesetzt wird [Minuten]

//	unsigned int SollTempFlag;		//Entscheidung ob Solltemperatur von Bedienteil oder extern versendet werden soll

//	unsigned short TempSollIntern;
//	int TempSollIntern;				//Solltemperatur, die am Bedienelement eingestellt wurde (Faktor 100)

//	int TempSollInternLuft;			//Solltemperatur, die am Bedienelement für die Lft eingestellt wurde (nur für 2 Heizkreis Variante) (Faktor 100)

//	unsigned short TempSollExtern;
//	int TempSollExtern;				//Solltemperatur, die von extern übermittelt wurde (Faktor 100)

//	unsigned short tempIntern; 		//DPT9 format, Messwert vom internen Temperatursensor
	int tempIntern;

//	unsigned short tempExtern;		//DPT9 format, Messwert vom externen Temperatursensor
	int tempExtern;

	bool floorTempShow; 			//If the Floor Temp should be showed in the LCD

	int ExtTempSensSource;			//Define where the external Temperature Sensor is connected (at the RTR or via KNX)
};

extern temp_values& temp;
//extern struct temp_values temp;

#define TEMP_EEPROM_SIZE 9

extern int tempEepromData[TEMP_EEPROM_SIZE];


void initSensors(void);
void checkTempSensors(void);
void handleSetTempSourcePeriodic(void);
void checkAirQuality(void);
void checkAirHumidity(void);
void evalIAQ(void);

#endif
