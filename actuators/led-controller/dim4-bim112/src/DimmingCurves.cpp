/*
 * DimmingCurves.cpp
 *
 *  Created on: 06.03.2020
 *      Author: x
 */

#include "DimmingCurves.h"
#include "config.h"
#include <math.h>	// include before sblib's math.h via eib.h
#include <sblib/eib.h>
#include <com_objs.h>
#include "pwmout.h"



#ifdef PWM
extern pwmout pwmout[];
#endif


void setOutput(int ch, int value) {
	int outputValue;
	if (value < 1) {
		outputValue = 0;
	} else {
		/* Hier wird Minimal- und Maximalhelligkeit begrenzt. Das Dimmen und die Rückmeldung laufen jedoch weiter.
		 * Wie sich das Original genau verhält ist aus dem Handbuch nicht genauer ersichtlich
		 */
		int max = (2*MAXOUTPUTVALUE*userEeprom.getUInt8(APP_MAX_LIGHT + ch * APP_CH_OFFS)/255+1)/2;  // Maximalhelligkeit in 0-MAXOUTPUTVALUE
		int min = (2*MAXOUTPUTVALUE*userEeprom.getUInt8(APP_MIN_LIGHT + ch * APP_CH_OFFS)/255+1)/2;  // Minimalhelligkeit in 0-MAXOUTPUTVALUE
		if (value > max) {
			value = max;
		} else if (value < min) {
			value = min;
		}
		switch (userEeprom.getUInt8(APP_DIMM_CURVE)) {
		case 0:		//quadratisch
			outputValue = value*value*0.0001;		//TODO anpassen bei MAXOUTPUTVALUE<>10000
			break;
		case 2:		//halb-logarithmisch
			outputValue = 0.0000000000009*pow(value, 4)+0.1*value;		//TODO anpassen bei MAXOUTPUTVALUE<>10000
			break;
		default:	//case 7:    linear
			outputValue = value;
			break;
		}
	}
	#ifdef PWM
	pwmout[ch].setpwm(outputValue);
	#endif
}


