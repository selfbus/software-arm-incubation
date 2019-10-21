/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef CONFIG_H_
#define CONFIG_H_


// select if the hardware has a IAQ Sensor
#define DEVICE_WITH_VOC	true	// with sensor = true, without sensor = false

// select if the hardware will control 1 or 2 heating circles (true = one circle, false = two circles)
#define EINHEIZKREIS true

// select if the value from an external temperature sensor in the floor is displayed
#define EXTERNAL_TEMP_SENS true

// time to get back automatically from menu to home screen [ms]
#define MENUTIME 8000

// time to shut off LCD backlight
#define LCDBACKLIGHTTIME 15000

#endif /* CONFIG_H_ */
