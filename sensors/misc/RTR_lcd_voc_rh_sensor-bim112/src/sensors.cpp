#include "sensors.h"
#include "params.h"

#include <sblib/timeout.h>
#include <sblib/sensors/ds18x20.h>
#include <sblib/eib/datapoint_types.h>
#include <sblib/ioports.h>

#include "app_rtr.h"

SHT2xClass SHT21;

CCS811Class CCS811;

#if EXTERNAL_TEMP_SENS
DS18x20 extDS18B20;
#endif // EXTERNAL_TEMP_SENS

struct air_quality_values air_quality;
struct air_humidity_values air_humidity;


int  __attribute__ ((aligned (4))) tempEepromData[TEMP_EEPROM_SIZE];
temp_values& temp = *(temp_values*) tempEepromData;

bool TempSensAvailable;

bool SollTempFlagMem = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);

void initSensors(){
	SHT21.Init();

	CCS811.begin(CCS_811_ADDR, PIO2_8); //Address and Wake Pin of CCS811

#if EXTERNAL_TEMP_SENS
	extDS18B20.DS18x20Init(PIO2_5, false); //Data Pin, no parasite mode
	extDS18B20.Search(1); //externen Sensor suchen
#endif // EXTERNAL_TEMP_SENS
}

void checkTempSensors(void) {
#if EXTERNAL_TEMP_SENS
	if(temp.ExtTempSensSource == ExtTempAtBoard){
		if(extDS18B20.m_foundDevices == 0){ //beim Start wird der Sonsor manchmal nicht gefunden
			extDS18B20.Search(1); //externen Sensor erneut suchen
		}
		extDS18B20.readTemperature(extDS18B20.m_dsDev);
		temp.tempExtern = (int)(extDS18B20.m_dsDev[0].last_temperature*100);
	}
#endif // EXTERNAL_TEMP_SENS
	temp.tempIntern = SHT21.GetTemperature();
	if(temp.tempIntern == -273){
		TempSensAvailable = false;
	}else{
		TempSensAvailable = true;
	}
}

void handleTargetTempSourcePeriodic(void) {
	if(use_external_target_temperature){
		unsigned int autoResetTime = memMapper.getUInt32(UF_TEMP_AUTO_RESET_TIME);

		SollTempFlagMem = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		if(SollTempFlagMem != memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG)) { //start timeout if modus is changed
			if(autoResetTime != 0){ //if time is 0, never change back to automatic temperature
				SollTempFlagMem = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
				timeout[RETURN_TO_AUTO].start(autoResetTime*600); //calculating ms from (minutes with factor 100)
			}
		}

		if (timeout[RETURN_TO_AUTO].started() && timeout[RETURN_TO_AUTO].expired() && autoResetTime != 0) {
			memMapper.setUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
			memMapper.doFlash();
		}
	}
}

void checkAirQuality(void) {
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
