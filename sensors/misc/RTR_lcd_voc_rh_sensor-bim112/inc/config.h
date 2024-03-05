/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_


/*
 * Diese defines müssen vor dem Kompilieren ausgewählt werden
 * Leider können nicht alle Kombinationen kompiliert werden, da der Speicherplatz des LPC1115 nicht ausreicht.
 */

// Hardwarekonfiguration der Sensoren festlegen
// hierdurch werden die Schriftgrößen und Anordnungen auf dem LCD verändert
#define TEMP_SENSOR_INSTALLED		true //!< Set to false to debug without attached i2c temperature sensor
#define VOC_SENSOR_INSTALLED		true //!< Set to false to debug without attached i2c VOC sensor
#define HUMIDITY_SENSOR_INSTALLED	true
#define EXTERNAL_TEMP_SENS 			false

// select if the hardware will control 1 or 2 heating circles (true = one circle, false = two circles)
#define EINHEIZKREIS true

// select if the value from an external temperature sensor in the floor is displayed
//#define EXTERNAL_TEMP_SENS true

// time to get back automatically from menu to home screen [ms]
#define MENUTIME 8000

// time to shut off LCD backlight
#define LCDBACKLIGHTTIME 15000

#endif /* CONFIG_H_ */
