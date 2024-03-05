/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
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

#include <sblib/eibMASK0701.h>
#include <sblib/core.h>
#include <sblib/ioports.h>
#include <sblib/serial.h>
#include <sblib/timeout.h>

#include <u8g_arm.h>
#include "lcd.h"
#include "params.h"
#include "pwm.h"
#include "sensors.h"
#include "config.h"
#include "inputs.h"
#include "knxprodHeader.h"
#include "app_rtr.h"

//#define _DEBUG__

#ifdef _DEBUG__
Serial Serial(PIO2_7, PIO2_8);
#endif

//SPI spi(SPI_PORT_0);

int blinkPin = PIO0_7;

// Timeout
Timeout timeout[NUM_TIMED_VALUES];

MASK0701 bcu = MASK0701();

APP_VERSION("SBrtrLcd", "1", "11")

/*
* Der MemMapper bekommt einen 256 Byte großen Bereich ab 0xEE00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
* Damit lassen sich 1 Page (256Byte) (und die allocTable die MemMappers (256 Byte)) unterbringen.
* 0xEE00 = 60928 -> 60928/256=238 -> ab Page 238
*/
MemMapper memMapper(UF_BASE_ADDRESS, UF_SIZE, false);

/*
 * Initialize the application.
 */
BcuBase* setup() {

	bcu.setProgPin(PIO2_11);

	bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION); // see in knxprodHeader.h for Device information


//	pinMode(blinkPin, OUTPUT);

//	memcpy(userEeprom.order, hardwareVersion, sizeof(hardwareVersion));

	// Enable the serial port with 19200 baud, no parity, 1 stop bit
#ifdef _DEBUG__
	serial.begin(19200);
	serial.println("UC1701x Display Taster TEST");
#endif

    bcu.setMemMapper(&memMapper);
    if (memMapper.addRange(UF_BASE_ADDRESS, UF_SIZE) != MEM_MAPPER_SUCCESS) { // Zum Abspeichern/Laden des Systemzustands
        fatalError();
    }

    if(memMapper.getUInt8(UF_INITIALIZED) != 1){
        memMapper.setUInt8(UF_INITIALIZED, 1);
        memMapper.setUInt32(UF_TEMP_AUTO_RESET_TIME, 500);
        memMapper.setUInt32(UF_TEMP_SOLL_INTERN, 2000);
        memMapper.setUInt32(UF_TEMP_SOLL_INTERN_LUFT, 2100);
        memMapper.setUInt32(UF_TEMP_SOLL_EXTERN, 2200);
        memMapper.setUInt32(UF_TEMP_SOLL_TEMP_FLAG, SollTempExtern);
        memMapper.setUInt32(UF_LCD_BRIGHTNESS, 500);
        memMapper.doFlash();
    }

	init_inputs();

	u8g_InitComFn(&u8g, &u8g_dev_uc1701_mini12864_hw_spi, u8g_com_hw_spi_fn);

	u8g_SetDefaultForegroundColor(&u8g);

	initPWM();
	int lcd_brightness = memMapper.getUInt32(UF_LCD_BRIGHTNESS);
	setPWM(lcd_brightness/100);

	initSensors();

	initApplication();

	init_lcd();

	timeout[LCD_BACKLIGHT].start(LCDBACKLIGHTTIME);
	BacklightOnFlag = true;

	debounceTime = 10;// userEeprom[EE_TIMING_PARAMS_BASE + 3]; //TODO: checken ob Parameter benötigt wird!
	return &bcu;
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
	while ((objno = bcu.comObjects->nextUpdatedObject()) >= 0)
	{
		objectUpdated(objno);
	}
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
