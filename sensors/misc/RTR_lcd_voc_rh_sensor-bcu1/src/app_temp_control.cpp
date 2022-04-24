/*
 *  Copyright (c) 2016 Oliver Stefan <os252@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/eib/bcu1.h>
#include <sblib/timeout.h>
#include <sblib/eib/com_objects.h>
#include "app_temp_control.h"
#include "params.h"
#include "sensors.h"
#include "lcd.h"
#include "config.h"

extern BCU1 bcu;

const byte* functionsParams;
const byte* TimingParams;

bool applicationBoardConnected = true;

/*
 * Funktion zur Umrechnung der Zeitbasis und der Einheit zu einem Wert in Millisekunden
 */
unsigned int factortime_to_ms(unsigned int startaddress) {
	uint8_t time_factor, time_unit;
	unsigned int result = 1000; //start at 1s = 1000ms
	time_factor = ((*(bcu.userEeprom))[startaddress] & 0xFC) >> 2; // Bits 2..7 are the factor
	time_unit   = (*(bcu.userEeprom))[startaddress] & 0x03; // Bits 0..1 are the unit of time (0=sec, 1=min, 2=hours)

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

    functionsParams = bcu.userEeprom->userEepromData + (EE_FUNCTIONS_PARAMS_BASE - bcu.userEeprom->userEepromStart);
    TimingParams = bcu.userEeprom->userEepromData + (EE_TIMING_PARAMS_BASE - bcu.userEeprom->userEepromStart);
	unsigned char eepromParams = (*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE];

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & TEMPERATURE_FUNCTION_ACTIVE) {
		temp.functionActive = true;
		temp.sendInterval = factortime_to_ms(EE_TIMING_PARAMS_BASE);
//		temp.autoResetTime = factortime_to_ms(EE_TIMING_PARAMS_BASE + 1);
		timeout[TEMPERATURES_KO].start(temp.sendInterval);
		timeout[TEMPERATURES_LCD].start(1000);
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & FLOOR_TEMP_SHOW) {
		temp.floorTempShow = true;
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & CONN_EXT_TEMP_SENS) {
		temp.ExtTempSensSource = ExtTempAtBoard;
	}else{
		temp.ExtTempSensSource = ExtTempOverKNX;
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & AIR_QUALITY_ACTIVE) {
		air_quality.functionActive = true;
		air_quality.sendIntervall = factortime_to_ms(EE_TIMING_PARAMS_BASE + 2);
		timeout[AIR_QUALITY_KO].start(air_quality.sendIntervall);
		timeout[AIR_QUALITY_LCD].start(1000);
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & AIR_HUMIDITY_ACTIVE) {
		air_humidity.functionActive = true;
		air_humidity.sendIntervall = factortime_to_ms(EE_TIMING_PARAMS_BASE + 4);
		timeout[AIR_HUMIDITY_KO].start(air_humidity.sendIntervall);
		timeout[AIR_HUMIDITY_LCD].start(1000);
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & DISPLAY_WINDOW_OPEN) {
		window_ventilation.show_window_state = true;
	}

	if ((*(bcu.userEeprom))[EE_FUNCTIONS_PARAMS_BASE] & DISPLAY_AIR_VENTILATION) {
		window_ventilation.show_ventilation_state = true;
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
//		memMapper.setUInt32(UF_TEMP_SOLL_EXTERN, dptFromFloat((objectRead(objno) & 0xFFFF)));
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_EXTERN, dptFromFloat(bcu.comObjects->objectRead(objno) & 0xFFFF));
//		memMapper.doFlash();
		extEeprom.write_to_chip();
//		temp.TempSollExtern = dptFromFloat((objectRead(objno) & 0xFFFF));
//		temp.hekaTempSollExtern = dptFromFloat( temp.TempSollExtern );
	}

	if(objno == REC_EXT_TEMP && temp.floorTempShow && temp.ExtTempSensSource == ExtTempOverKNX){
		temp.tempExtern = dptFromFloat(bcu.comObjects->objectRead(objno) & 0xFFFF);
	}
}

// send values periodic (temperature internal, temperature external, temperature set value, air quality, air humidity)
void handlePeriodic(void) {

	if (timeout[TEMPERATURES_KO].started() && timeout[TEMPERATURES_KO].expired()) {

		// send temperature internal, temperature external, temperature set value
	    bcu.comObjects->objectWriteFloat(SEND_INTERN_TEMP, temp.tempIntern);

//		int SollTempFlag = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		int SollTempFlag = extEeprom.eepromGetUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		if(SollTempFlag == SollTempIntern){
//			objectWriteFloat(SEND_SET_TEMP, dptToFloat(memMapper.getUInt32(UF_TEMP_SOLL_INTERN)));
		    bcu.comObjects->objectWriteFloat(SEND_SET_TEMP, extEeprom.eepromGetUInt32(UF_TEMP_SOLL_INTERN));
		}else if (SollTempFlag == SollTempExtern){
//			objectWriteFloat(SEND_SET_TEMP, dptToFloat(memMapper.getUInt32(UF_TEMP_SOLL_EXTERN)));
		    bcu.comObjects->objectWriteFloat(SEND_SET_TEMP, extEeprom.eepromGetUInt32(UF_TEMP_SOLL_EXTERN));
		}
		timeout[TEMPERATURES_KO].start(temp.sendInterval);
	}

	if (timeout[TEMPERATURES_LCD].started() && timeout[TEMPERATURES_LCD].expired()) {
		//read all temperatures from sensors
		checkTempSensors();
		timeout[TEMPERATURES_LCD].start(1000);
	}

#if DEVICE_WITH_VOC
	if (timeout[AIR_QUALITY_KO].started() && timeout[AIR_QUALITY_KO].expired()) {
		// send air quality
	    bcu.comObjects->objectWriteFloat(SEND_AIR_QUALITY, (air_quality.AirCO2*100));
		timeout[AIR_QUALITY_KO].start(1000);
	}

	if (timeout[AIR_QUALITY_LCD].started() && timeout[AIR_QUALITY_LCD].expired()) {
		checkAirQuality();
		timeout[AIR_QUALITY_LCD].start(1000);
	}
#endif

	if(timeout[AIR_HUMIDITY_KO].started() && timeout[AIR_HUMIDITY_KO].expired()) {
		//send air humidity
	    bcu.comObjects->objectWriteFloat(SEND_AIR_HUMIDITY, air_humidity.AirRH);
		timeout[AIR_HUMIDITY_KO].start(air_humidity.sendIntervall);
	}

	if(timeout[AIR_HUMIDITY_LCD].started() && timeout[AIR_HUMIDITY_LCD].expired()) {
		checkAirHumidity();
		timeout[AIR_HUMIDITY_LCD].start(air_humidity.sendIntervall);
	}

	//handle periodic if the setpoint temperature is changed from internal to external
	handleSetTempSourcePeriodic();
}

// initiate changes on Display
void inputChanged(int channel, int val, uint8_t pressType) {

		lcdMenu(channel, val, pressType);
}
