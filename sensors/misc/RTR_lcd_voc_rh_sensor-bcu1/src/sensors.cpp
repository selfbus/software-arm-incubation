#include <sblib/timeout.h>
#include <sblib/i2c/SHT2x.h>
#include <sblib/i2c/iaq-core.h>
#include <sblib/sensors/ds18x20.h>
#include <sblib/eib/datapoint_types.h>
#include <sblib/ioports.h>
#include "sensors.h"
#include "params.h"

SHT2xClass SHT21;

CCS811Class CCS811;

DS18x20 extDS18B20;

struct air_quality_values air_quality;
struct air_humidity_values air_humidity;


int  __attribute__ ((aligned (4))) tempEepromData[TEMP_EEPROM_SIZE];
temp_values& temp = *(temp_values*) tempEepromData;

bool TempSensAvailable;

bool SollTempFlagMem = extEeprom.eepromGetUInt32(UF_TEMP_SOLL_TEMP_FLAG);

void initSensors(){
	SHT21.Init();

	CCS811.begin(CCS_811_ADDR, PIO2_8); //Address and Wake Pin of CCS811

	extDS18B20.DS18x20Init(PIO2_5, false); //Data Pin, no parasite mode
	extDS18B20.Search(1); //externen Sensor suchen
	if(extDS18B20.m_foundDevices) {
		extDS18B20.startConversion(0); // fang schon mal an
	}
}

void checkTempSensors(void) {
	if(temp.ExtTempSensSource == ExtTempAtBoard){
		if(extDS18B20.m_foundDevices == 0){ //beim Start wird der Sonsor manchmal nicht gefunden
			extDS18B20.Search(1); //externen Sensor erneut suchen
		}
		if (extDS18B20.readResult(0)) {
			if (extDS18B20.lastReadOk(0)) {
			temp.tempExtern = (int)(extDS18B20.m_dsDev[0].last_temperature*100);
			}
			extDS18B20.startConversion(0); // fuer's naechste Mal
		}
	}
	temp.tempIntern = SHT21.GetTemperature();
	if(temp.tempIntern == -273){
		TempSensAvailable = false;
	}else{
		TempSensAvailable = true;
	}
}

void handleSetTempSourcePeriodic(void) {
//	if(SollTempFlagMem != memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG)) { //start timeout if modus is changed
	if(SollTempFlagMem != extEeprom.eepromGetUInt32(UF_TEMP_SOLL_TEMP_FLAG)) { //start timeout if modus is changed
//		SollTempFlagMem = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		unsigned int autoResetTime;
		extEeprom.read(UF_TEMP_AUTO_RESET_TIME, (char*)&autoResetTime, 4);
//		memMapper.readMemPtr(UF_TEMP_AUTO_RESET_TIME, (byte*)&autoResetTime, 4);
		if(autoResetTime != 0){ //if time is 0, never change back to automatic temperature
			SollTempFlagMem = extEeprom.eepromGetUInt32(UF_TEMP_SOLL_TEMP_FLAG);
			timeout[RETURN_TO_AUTO].start(autoResetTime*600); //calculating ms from (minutes with factor 100)
		}
	}
	if (timeout[RETURN_TO_AUTO].started() && timeout[RETURN_TO_AUTO].expired()) {
//		memMapper.setUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
//		memMapper.doFlash();
		extEeprom.write_to_chip();
	}
}

void checkAirQuality(void) {

//	uint16_t CO2eq;
//	IAQcore.GetIAQco2(CO2eq);
	CCS811.getData();
	air_quality.AirCO2 = CCS811.readCO2();
	evalIAQ();
}

void checkAirHumidity(void){
	air_humidity.AirRH = SHT21.GetHumidity();
//	air_humidity.AirDewPoint = SHT21.GetDewPoint()*100; //TODO: remove float!
}

void evalIAQ(void){
	if(air_quality.AirCO2 <= 800){
		air_quality.IAQcondition = IAQgood;
	}else if(air_quality.AirCO2 > 800 && air_quality.AirCO2 <= 1600){
		air_quality.IAQcondition = IAQneutral;
	}else if(air_quality.AirCO2 > 1600){
		air_quality.IAQcondition = IAQbad;
	}
}
