/*
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <Appl.h>
#include <sblib/eib.h>
#include <sblib/eib/user_memory.h>
#include <sblib/eib/sblib_default_objects.h>
#include <string.h> /* for memcpy() */
//#include "MemMapperMod.h"
//#include <sblib/usr_callback.h>
#include "config.h"
#include <sblib/serial.h>  //debugging only


//extern MemMapperMod memMapper;
///*
// * Der MemMapper bekommt einen 1kB Bereich ab 0xEA00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
// * Damit lassen sich 3 Pages (je 256Byte) (und die allocTable die MemMappers) unterbringen.
// * Benötigt werden zwei Pages:
// * - für den Konfigurationsspeicher jenseits von 0x4B00. Für die SbLib endet der Konfigurationspeicher dort,
// *   unser Vorbild legt dort jedoch noch einige allgemeine Optionen ab.
// * - für die Systemzustände. Diese werden bei Busspannungsausfall und Neustart abgespeichert.
// */
//MemMapperMod memMapper(0xea00, 0x400);
////AppUsrCallback usrCallback;

const HardwareVersion * currentVersion;
unsigned int pwmmax;		//  je nach Parametrierung PWM_MAX_600 oder PWM_MAX_1000

/**
 * Application setup
 */
void setup()
{
	serial.setTxPin(PIO3_0);	//debugging only
	serial.begin(115200);		//debugging only
	currentVersion = &hardwareVersion[HARDWARE_ID];
	bcu.begin(MANUFACTURER, currentVersion->deviceType, currentVersion->appVersion);
	// _bcu und bcu sind das gleiche Objekt.
	 // _bcu ist vom Typ BCU, während bcu vom Typ BcuBase ist.
	 //_bcu.setMemMapper((MemMapper *)&memMapper); // Der BCU wird hier der modifizierte MemMapper bekanntgemacht
	 //_bcu.setUsrCallback((UsrCallback *)&usrCallback);
	 //_bcu.enableGroupTelSend(false);
	 //memcpy(userEeprom.serial, SerialNumber, sizeof(SerialNumber));
	 // 12 Bytes der Aktorkonfiguration werden ab 0x4B00 geschrieben. Das liegt bloederweise
	 // genau jenseits des USER-EEPROM. Also mappen wir virtuellen Speicherbereich dorthin.
	 //serial.print(memMapper.addRange(0x4b00, 0x300));
	 //memMapper.addRange(0x0, 0x100); // Zum Abspeichern/Laden des Systemzustands
	 //objectEndian(LITTLE_ENDIAN);
	 //userEeprom.commsTabAddr = 0x4400; // Diese Basisadresse wird nicht über die ETS runtergeschrieben, ist aber notwendig!
	 //setUserRamStart(0x3FC);
	//// FIXME for new memory mapper
    memcpy(userEeprom.order, currentVersion->hardwareVersion,
            sizeof(currentVersion->hardwareVersion));
    initApplication();
}

/**
 * The application's main.
 */
void loop()
{
    int objno;
    // Handle updated communication objects
    while ((objno = nextUpdatedObject()) >= 0)
    {
        objectUpdated(objno);
    }

    checkPeriodic();

    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle())
        waitForInterrupt();
}
