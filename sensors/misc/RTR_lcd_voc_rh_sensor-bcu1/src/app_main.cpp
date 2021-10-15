/**



/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


/*
 * TODO!!!
 *
 * externen Temperatursensor implementieren (sensors.cpp checkTempSensors())
 *
 * zum speicher sparen Gradzeichen in Bitmap wandeln und Schrift auf reduziert stellen
 * entfernen der aktivierung des externen temp sensors aus der VD -> 2 Versionen in VD
 */


#include <sblib/eib/sblib_default_objects.h>
#include <sblib/core.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>
#include <sblib/serial.h>
#include <sblib/timeout.h>

#include <u8g_arm.h>
#include "lcd.h"
#include "params.h"
#include "pwm.h"
#include "app_temp_control.h"
#include "sensors.h"
#include "config.h"
#include "inputs.h"
#include "ext_eeprom.h"

#define _DEBUG__

#ifdef _DEBUG__
Serial Serial(PIO2_7, PIO2_8);
#endif

//SPI spi(SPI_PORT_0);

int blinkPin = PIO0_7;

// Timeout
Timeout timeout[NUM_TIMED_VALUES];

ExtEeprom extEeprom;

/*
* Der MemMapper bekommt einen 1kB Bereich ab 0xEA00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
* Damit lassen sich 3 Pages (je 256Byte) (und die allocTable die MemMappers) unterbringen.
* 0xEA00 = 59904 -> 59904/256=234 -> ab Page 234
*/
//MemMapper memMapper(0xea00, 0x400, false);

/*
 * Initialize the application.
 */
void setup() {

#if EINHEIZKREIS
	bcu.begin(76, 0x474, 2);  // we are a MDT temperatur controller, version 1.2
#else
	bcu.begin(76, 0x47E, 2); // we are a Selfbus room temperature and air controller Version 0.2
#endif

//	pinMode(blinkPin, OUTPUT);

//	memcpy(userEeprom.order, hardwareVersion, sizeof(hardwareVersion));

	// Enable the serial port with 19200 baud, no parity, 1 stop bit
#ifdef _DEBUG__
	serial.begin(19200);
	serial.println("UC1701x Display Taster TEST");
#endif

//	_bcu.setMemMapper(&memMapper);
//	memMapper.addRange(0x0, 0x100); // Zum Abspeichern/Laden des Systemzustands
//
//
//	if(memMapper.getUInt8(UF_INITIALIZED) != 1){
//		memMapper.setUInt8(UF_INITIALIZED, 1);
//		memMapper.setUInt32(UF_TEMP_AUTO_RESET_TIME, 500);
//		memMapper.setUInt32(UF_TEMP_SOLL_INTERN, 2000);
//		memMapper.setUInt32(UF_TEMP_SOLL_INTERN_LUFT, 2100);
//		memMapper.setUInt32(UF_TEMP_SOLL_EXTERN, 2200);
//		memMapper.setUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
//		memMapper.setUInt32(UF_LCD_BRIGHTNESS, 500);
//	}


	extEeprom.init_eeprom();

	if(extEeprom.eepromGetUInt8(UF_INITIALIZED) != 1){
		extEeprom.eepromSetUInt8(UF_INITIALIZED, 1);
		extEeprom.eepromSetUInt32(UF_TEMP_AUTO_RESET_TIME, 500);
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_INTERN, 2000);
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_INTERN_LUFT, 2100);
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_EXTERN, 2200);
		extEeprom.eepromSetUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
		extEeprom.eepromSetUInt32(UF_LCD_BRIGHTNESS, 500);
		extEeprom.write_to_chip();
	}

	init_inputs();

	u8g_InitComFn(&u8g, &u8g_dev_uc1701_mini12864_hw_spi, u8g_com_hw_spi_fn);

	u8g_SetDefaultForegroundColor(&u8g);

	initPWM();
//	int lcd_brightness = memMapper.getUInt32(UF_LCD_BRIGHTNESS);
	int lcd_brightness = extEeprom.eepromGetUInt32(UF_LCD_BRIGHTNESS);
	setPWM(lcd_brightness/100);

	initSensors();

	initApplication();

	timeout[LCD_BACKLIGHT].start(LCDBACKLIGHTTIME);
	BacklightOnFlag = true;

	debounceTime = 10;// userEeprom[EE_TIMING_PARAMS_BASE + 3]; //TODO: checken ob Parameter benötigt wird!
}

bool LCDdrawFlag = false; //a helper to get more runs through the complete loop
/*
 * The main processing loop.
 */
void loop() {
    int objno;

    if(!TempSensAvailable){
    	applicationBoardConnected = false;
    }


    //divide the LCD process in pieces for better scheduling
    if(LCDdrawFlag == false){
    	u8g_FirstPage(&u8g);
    	LCDdrawFlag = true;
    }else{
    	drawLCD();
    	if(u8g_NextPage(&u8g) == 0){ //returns 0 if the picture loop has been finished, 1 if another redraw of the picture is required
    		LCDdrawFlag = false;
    	}
    }

	checkLCDperiodic();

	handlePeriodic();

	handlePeriodicInputs();

	// Handle updated communication objects
	while ((objno = nextUpdatedObject()) >= 0)
	{
		objectUpdated(objno);
	}

}

