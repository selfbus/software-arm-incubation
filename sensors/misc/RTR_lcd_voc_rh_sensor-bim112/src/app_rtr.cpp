/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/eib/bcu1.h>
#include <sblib/timeout.h>
#include <sblib/eib/com_objects.h>
#include "app_rtr.h"
#include "params.h"
#include "sensors.h"
#include "lcd.h"
#include "config.h"

extern BCU1 bcu;

const byte* functionsParams;
//const byte* TimingParams;

bool applicationBoardConnected = true;
bool use_external_target_temperature;
/*
 * Funktion zur Umrechnung der Zeitbasis und der Einheit zu einem Wert in Millisekunden
 */
unsigned int factortime_to_ms(uint8_t timeParaNumber) {
	int memoryOffset = timeParaNumber * EE_SIZE_OF_EACH_SENDING_PARAM;

	uint8_t time_factor, time_unit;
	unsigned int result = 1000; //start at 1s = 1000ms
	time_factor = ((*(bcu.userEeprom))[EE_SENDING_PARAMS_BASE + memoryOffset] & 0xFC) >> 2; // Bits 2..7 are the factor
	time_unit   = (*(bcu.userEeprom))[EE_SENDING_PARAMS_BASE + memoryOffset] & 0x03; // Bits 0..1 are the unit of time (0=sec, 1=min, 2=hours)

	for(uint8_t i = 0; i<time_unit; i++){
		result *= 60; //calculate the factor (0*60 = sec, 1*60=min, 2*60=hours)
	}
	return (result *= time_factor);
}

/*
 * Funktion zur Prüfung, ob nach erfolgtem langem Druck beider Tasten die Neuinitialisierung aller Komponenten nötig ist
 * Nötig wird dieses, nachdem die Applikationsplatine vom Controller abgezogen und wieder angesteckt wurde.
 */
void rebootApplication(void){
	if(TempSensAvailable == true){
	   	if(applicationBoardConnected == false){
	       	setup();
	       	applicationBoardConnected = true;
	   	}
	}else{
		applicationBoardConnected = false;
	}
}

void initApplication(void) {
    functionsParams = bcu.userMemoryPtr(EE_FUNCTIONS_PARAMS_BASE);
    //TimingParams = bcu.userMemoryPtr(EE_TIMING_PARAMS_BASE);

#if TEMP_SENS_INSTALLED
	if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_INTERNAL_TEMPERATURE_CYCLIC){
		temp.sendInterval = factortime_to_ms(EE_INTERNAL_TEMP_SENDING_PARAM);
		timeout[TEMPERATURES_KO].start(temp.sendInterval);
	}
	timeout[TEMPERATURES_LCD].start(1000);
#endif // TEMP_SENS_INSTALLED

	if((*(bcu.userEeprom))[EE_TARGET_TEMP_FUNCTIONS_BASE] & SEND_TARGET_TEMPERATURE_CYCLIC){
		temp.sendIntervalTargetTemp = factortime_to_ms(EE_TARGET_TEMP_SENDING_PARAM);
		timeout[TARGET_TEMPERATURE_KO].start(temp.sendIntervalTargetTemp);
	}

#if EXTERNAL_TEMP_SENS
	if((*(bcu.userEeprom))[EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE] & EXT_TEMP_SENSOR_IS_CONNECTED){
		temp.floorTempShow = true;
		if ((*(bcu.userEeprom))[EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE] & CONN_EXT_TEMP_SENSOR) {
			temp.ExtTempSensSource = ExtTempAtBoard; // externer Temperatursensor über OneWire direkt angeschlossen
		}else{
			temp.ExtTempSensSource = ExtTempOverKNX; // die externe Temperatur wird über KNX übermittelt
		}

		if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_EXTERNAL_TEMPERATURE_CYCLIC){
			temp.sendIntervalExternalTemp = factortime_to_ms(EE_EXTERNAL_TEMP_SENDING_PARAM);
			timeout[EXTERNAL_TEMPERATURE_KO].start(temp.sendIntervalExternalTemp);
		}
	}
#endif // EXTERNAL_TEMP_SENS

#if VOC_SENSOR_INSTALLED
	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_VOC_CYCLIC) {
		air_quality.sendIntervall = factortime_to_ms(EE_VOC_SENDING_PARAM);
		timeout[AIR_QUALITY_KO].start(air_quality.sendIntervall);
	}
	timeout[AIR_QUALITY_LCD].start(1000);
#endif /* VOC_SENSOR_INSTALLED */

#if HUMIDITY_SENSOR_INSTALLED
	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_HUMIDITY_CYCLIC) {
		air_humidity.sendIntervall = factortime_to_ms(EE_HUMIDITY_SENDING_PARAM);
		timeout[AIR_HUMIDITY_KO].start(air_humidity.sendIntervall);
	}
	timeout[AIR_HUMIDITY_LCD].start(1000);
#endif /* HUMIDITY_SENSOR_INSTALLED */


	if ((*(bcu.userEeprom))[EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE] & DISPLAY_WINDOW_OPEN) {
		window_ventilation.show_window_state = true;
	}

	if ((*(bcu.userEeprom))[EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE] & DISPLAY_AIR_VENTILATION) {
		window_ventilation.show_ventilation_state = true;
	}

	if ((*(bcu.userEeprom))[EE_EXT_TEMP_DISPLAY_FUNCTIONS_BASE] & DISPLAY_HEATING_STATE) {
		window_ventilation.show_heating_state = true;
	}

	if ((*(bcu.userEeprom))[EE_TARGET_TEMP_FUNCTIONS_BASE] & USE_EXTERNAL_TARGET_TEMPERATURE) {
		use_external_target_temperature = true;
	}
}

unsigned int lastTargetTemperature;

void sendTargetTemperature(){

	unsigned int actualSetTemperature = -255;

	if(use_external_target_temperature){
		int SollTempFlag = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		if(SollTempFlag == SollTempIntern){
			actualSetTemperature = memMapper.getUInt32(UF_TEMP_SOLL_INTERN);
		}else if (SollTempFlag == SollTempExtern){
			actualSetTemperature = memMapper.getUInt32(UF_TEMP_SOLL_EXTERN);
		}
	}else{
		actualSetTemperature = memMapper.getUInt32(UF_TEMP_SOLL_INTERN);
	}

	if(actualSetTemperature != lastTargetTemperature){
		bcu.comObjects->objectWriteFloat(SEND_SET_TEMP, actualSetTemperature);
		lastTargetTemperature = actualSetTemperature;
	}

}

// handle external-set-temperature, window state, ventilation state
void objectUpdated(int objno) {
	if(objno == REC_WINDOW_STATE){
		window_ventilation.window_state = (bcu.comObjects->objectRead(objno) & 0x01);
	}
	if(objno == REC_VENTILATION_LEVEL){
		window_ventilation.ventilation_level = (bcu.comObjects->objectRead(objno) & 0xFF);
	}
	if(objno == REC_EXT_SET_TEMP){
		memMapper.setUInt32(UF_TEMP_SOLL_EXTERN, dpt9ToFloat(bcu.comObjects->objectRead(objno) & 0xFFFF));
		memMapper.doFlash();
	}

	if(objno == SEND_REC_EXT_TEMP && temp.floorTempShow && temp.ExtTempSensSource == ExtTempOverKNX){
		temp.tempExtern = dpt9ToFloat(bcu.comObjects->objectRead(objno) & 0xFFFF);
	}
}

// den Wert holen, um den sich der Messwert ändern muss um gesendet zu werden
float getChangeValue(uint8_t timeParaNumber) {
	int memoryOffset = timeParaNumber * EE_SIZE_OF_EACH_SENDING_PARAM;

	/*unsigned char *ChangeValuePointer = userEepromData
			+ (EE_SENDING_PARAMS_BASE + 1 + memoryOffset - USER_EEPROM_START);

	// der Wert liegt im EEPROM als DPT9 codiert vor
	unsigned short dpt9CodedValue = ChangeValuePointer[1] | (ChangeValuePointer[0] << 8);*/

	// ToDo: check if change is functional ok
	unsigned short dpt9CodedValue = (*(bcu.userEeprom))[EE_SENDING_PARAMS_BASE + 2 + memoryOffset] | (*(bcu.userEeprom))[EE_SENDING_PARAMS_BASE + 1 + memoryOffset];

	return dpt9ToFloat(dpt9CodedValue);
}

// send values periodic (temperature internal, temperature external, temperature set value, air quality, air humidity)
void handlePeriodic(void) {

	if (timeout[TEMPERATURES_KO].started() && timeout[TEMPERATURES_KO].expired()) {
		temp.tempInternOld = temp.tempIntern;
		bcu.comObjects->objectWriteFloat(SEND_INTERN_TEMP, temp.tempIntern);

		timeout[TEMPERATURES_KO].start(temp.sendInterval);
	}

	if (timeout[TARGET_TEMPERATURE_KO].started() && timeout[TARGET_TEMPERATURE_KO].expired()) {
		sendTargetTemperature();

		timeout[TARGET_TEMPERATURE_KO].start(temp.sendIntervalTargetTemp);
	}

	if (timeout[TEMPERATURES_LCD].started() && timeout[TEMPERATURES_LCD].expired()) {
		//read all temperatures from sensors
		checkTempSensors();

		bcu.comObjects->objectUpdateFloat(SEND_INTERN_TEMP, temp.tempIntern); // update object value for reading

		if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_INTERNAL_TEMPERATURE_AFTER_CHANGE){
			int diffValue = temp.tempInternOld - temp.tempIntern;
			if (diffValue < 0) {
				diffValue = -diffValue;
			}

			if(diffValue > getChangeValue(EE_INTERNAL_TEMP_SENDING_PARAM)){
				temp.tempInternOld = temp.tempIntern;
				bcu.comObjects->objectWriteFloat(SEND_INTERN_TEMP, temp.tempIntern);
			}
		}
#if EXTERNAL_TEMP_SENS

		objectUpdateFloat(SEND_REC_EXT_TEMP, temp.tempExtern); // update object value for reading

		if(timeout[EXTERNAL_TEMPERATURE_KO].started() && timeout[EXTERNAL_TEMPERATURE_KO].expired()) {
			temp.tempInternOld = temp.tempExtern;
			bcu.comObjects->objectWriteFloat(SEND_REC_EXT_TEMP, temp.tempExtern);
			timeout[EXTERNAL_TEMPERATURE_KO].start(temp.sendIntervalExternalTemp);
		}

		if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_EXTERNAL_TEMPERATURE_AFTER_CHANGE){
			int diffValue = temp.tempExternOld - temp.tempExtern;
			if (diffValue < 0) {
				diffValue = -diffValue;
			}

			if(diffValue > getChangeValue(EE_EXTERNAL_TEMP_SENDING_PARAM)){
				temp.tempInternOld = temp.tempExtern;
				bcu.comObjects->objectWriteFloat(SEND_REC_EXT_TEMP, temp.tempExtern);
			}
		}
#endif

		timeout[TEMPERATURES_LCD].start(1000);
	}

#if VOC_SENSOR_INSTALLED
	if (timeout[AIR_QUALITY_KO].started() && timeout[AIR_QUALITY_KO].expired()) {
		// send air quality
		air_quality.AirCO2Old = air_quality.AirCO2;
		bcu.comObjects->objectWriteFloat(SEND_AIR_QUALITY, (air_quality.AirCO2*100));
		timeout[AIR_QUALITY_KO].start(air_quality.sendIntervall);
	}

	if (timeout[AIR_QUALITY_LCD].started() && timeout[AIR_QUALITY_LCD].expired()) {

		checkAirQuality(); // read the Sensor values

		bcu.comObjects->objectUpdateFloat(SEND_AIR_QUALITY, (air_quality.AirCO2*100)); // update object value for reading

		if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_VOC_AFTER_CHANGE){
			int diffValue = air_quality.AirCO2Old - air_quality.AirCO2;
			if (diffValue < 0) {
				diffValue = -diffValue;
			}

			if(diffValue * 100 > getChangeValue(EE_VOC_SENDING_PARAM)){ // dpt9->float wird immer mit Faktor 100 ausgegeben
				air_quality.AirCO2Old = air_quality.AirCO2;
				bcu.comObjects->objectWriteFloat(SEND_AIR_QUALITY, (air_quality.AirCO2*100));
			}
		}

		timeout[AIR_QUALITY_LCD].start(1000);
	}
#endif

	if(timeout[AIR_HUMIDITY_KO].started() && timeout[AIR_HUMIDITY_KO].expired()) {
		//send air humidity
		air_humidity.AirRHOld = air_humidity.AirRH;
		bcu.comObjects->objectWriteFloat(SEND_AIR_HUMIDITY, air_humidity.AirRH);
		timeout[AIR_HUMIDITY_KO].start(air_humidity.sendIntervall);
	}

	if(timeout[AIR_HUMIDITY_LCD].started() && timeout[AIR_HUMIDITY_LCD].expired()) {

		checkAirHumidity(); // read the sensor values

		bcu.comObjects->objectUpdateFloat(SEND_AIR_HUMIDITY, air_humidity.AirRH); // update object value for reading

		if((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & SEND_HUMIDITY_AFTER_CHANGE){
					int diffValue = air_humidity.AirRHOld - air_humidity.AirRH;
					if (diffValue < 0) {
						diffValue = -diffValue;
					}

					if(diffValue > getChangeValue(EE_HUMIDITY_SENDING_PARAM)){
						air_humidity.AirRHOld = air_humidity.AirRH;
						bcu.comObjects->objectWriteFloat(SEND_AIR_HUMIDITY, air_humidity.AirRH);
					}
				}

		timeout[AIR_HUMIDITY_LCD].start(1000);
	}

	//handle periodic if the setpoint temperature is changed from internal to external
	handleTargetTempSourcePeriodic();
}


// initiate changes on Display
void inputChanged(int channel, int val, uint8_t pressType) {

		lcdMenu(channel, val, pressType);
}
