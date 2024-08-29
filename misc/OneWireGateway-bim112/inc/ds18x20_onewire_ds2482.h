/*
 *  ds18x20.h - Digital-output 1-Wire Temperature Sensors
 *
 *  Copyright (c) 2015 Erkan Colak <erkanc@gmx.de>
 *  Modified for use with DS2482 by Oliver Stefan (2021)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ds18x20_h
#define ds18x20_h

#ifndef onewire_h
//#error "Please Include first <OneWire.h>"
#include <sblib/i2c/ds2482.h>
#endif

#ifndef MAX_DS_DEVICES
#define MAX_DS_DEVICES 5
#endif

#define DS18X20_SEARCH 1

#if DS18X20_SEARCH
// 1-Wire family code
enum eDsType {
	DS18S20 = 0x10,  // DS18S20 & DS1820
	DS18B20 = 0x28,  // MAX31820 & DS18B20
	DS1822 = 0x22,
	DS_UNKNOWN = 0x00 // Unknown or No Sensor found
};
#endif

// Ds18x20 device struct
struct sDS18x20 {
#if DS18X20_SEARCH
	eDsType type;             // Device type
#endif
	uint8_t res_type;         // Resolution type 0 or 1
	byte data[12];            // Read data
	byte addr[8];             // Device ROM. 64bit Lasered ROM-Code to detect the Family Code
	bool crcOK;               // Last read crc state
	bool lastReadOK;          // Read state of last received value
	float current_temperature;          // Current temperature
	float last_temperature;   // Last temperature. Use always This!
};

// Temperature conversions types
typedef enum {
	CELCIUS = 0, FARENHEIT = 1, KELVIN = 2
} eScale;

/*
 * DHT Class
 */
class DS18x20_OneWireDS2482 {
private:
	OneWireDS2482 *_OWDS2482_DS18x;

public:
	uint8_t m_foundDevices;           // Number of found devices
	sDS18x20 m_dsDev[MAX_DS_DEVICES]; // Object list and informations of the found devices

	/*
	 * Function name:  ds18x20Init
	 * Descriptions:   Initialize the ds18x20
	 *                 Initialize DS18x20 only once.
	 *                 bParasiteMode is default off!
	 * parameters:     pin (PORT) and bParasiteMode (set to true if power via the dataline)
	 * Returned value: none
	 */
	void DS18x20Init(bool bParasiteMode, uint8_t ds2482Address);

	/*
	 * Function name:  ds18x20DeInit
	 * Descriptions:   DeInitialize the ds18x20
	 * parameters:     none
	 * Returned value: none
	 */
	void DS18x20DeInit();

	/*
	 * Function name:  Search
	 * Descriptions:   Search for 1-Wire DS18x20 Family devices.
	 *                 Set MAX_DS_Devices to the number of devices you
	 *                 want to search/find
	 * parameters:     uMaxDeviceSearch Maximal count of devices to find.
	 * Returned value: true, if one or more devices are found. Then, the
	 *                 following global Parameters will be filled:
	 *                 m_foundDevices - Includes the number of found devices
	 *                 m_dsDev        - Includes the Information about the devices
	 */
#if DS18X20_SEARCH
	bool Search(uint8_t uMaxDeviceSearch = MAX_DS_DEVICES);
#endif

	/*
	 * Function name:  readTemperature
	 * Descriptions:   Reads the Temperature of the given sDS18x20 device.
	 *                 Basically you need to set the '.addr'.
	 * parameters:     sDS18x20 *sDev: which must include the '.addr' of the
	 *                 DS18x20 device.(Is filled automatically by Search() function.)
	 * Returned value: true, if one or more devices reads are successful.
	 *                 following global Parameter of sDS18x20 will be filled:
	 *                 last_temperature - the current read temperature
	 *                 lastReadOK       - will be set to true if read was successful
	 */
	bool readTemperature(sDS18x20 *sDev);

	/*
	 * Function name:  readTemperatureAll
	 * Descriptions:   Iterates the the global m_dsDev object and calls
	 *                 readTemperature() for each device.
	 * parameters:    none
	 * Returned value: true, if one or more devices reads are successful.
	 *                 following global object parameters will be filled:
	 *                 last_temperature - the current read temperature
	 *                 lastReadOK       - will be set to true if read was successful
	 */
	bool readTemperatureAll();

	/*
	 * Function name:  ConvertTemperature
	 * Descriptions:  Will convert given fTemperature to given scale
	 * parameters:    fTemperature: the Temperature to convert
	 *                eScale: FARENHEIT | KELVIN | CELSIUS
	 * Returned value: Converted value
	 */
	float ConvertTemperature(float fTemperature, eScale Scale);
};

#endif /* ds18x20_h */
