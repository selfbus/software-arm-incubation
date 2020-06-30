/*
 * pwmout.cpp
 *
 *  Created on: 20.02.2020
 *      Author: X
 */

#include "pwmout.h"
#include <sblib/eib.h>
#include <com_objs.h>
#include "config.h"
#include <sblib/serial.h>  //debugging only

void pwmout::begin(int ch) {
	channel = ch;
	if (userEeprom.getUInt16(APP_PWM_O) == 0x600){	//0x600 = 600Hz ; 0xA00 = 1kHz
		pwmmax = PWM_MAX_600;						// 600Hz PWM Frequenz
	} else {
		pwmmax = PWM_MAX_1000;						// 1kHz PWM Frequenz
	}
	switch(channel){
	case 0:
// Auskommentiert weil PIO3_0 z.Zt. debug TX Pin ist
//		pinMode(PIO3_0, OUTPUT_MATCH);  // configure digital pin PIO3_0 to match MAT0 of timer16 #0  TS_ARM IO6
//		timer16_0.begin();
//		timer16_0.prescaler(PRESCALER);
//		timer16_0.matchMode(MAT0, SET);  // set the output of PIO3_0 to 1 when the timer matches MAT0
//		timer16_0.match(MAT0, pwmmax);      // match MAT0 when the timer reaches this value
//		timer16_0.pwmEnable(MAT0);       // enable PWM for match channel MAT0
//		timer16_0.matchMode(MAT3, RESET | INTERRUPT);	// Reset the timer when the timer matches MAT3 and generate an interrupt
//		timer16_0.match(MAT3, pwmmax);
//	    timer16_0.start();
		isactive = true;
		return;
		break;
	case 1:
		pinMode(PIO3_1, OUTPUT_MATCH);  // configure digital pin PIO3_1 to match MAT1 of timer16 #0  TS_ARM IO7
		timer16_0.begin();
		timer16_0.prescaler(PRESCALER);
	    timer16_0.matchMode(MAT1, SET);  // set the output of PIO3_1 to 1 when the timer matches MAT1
	    timer16_0.match(MAT1, pwmmax);      // match MAT1 when the timer reaches this value
	    timer16_0.pwmEnable(MAT1);       // enable PWM for match channel MAT1
	    timer16_0.matchMode(MAT3, RESET | INTERRUPT);	// Reset the timer when the timer matches MAT3 and generate an interrupt
	    timer16_0.match(MAT3, pwmmax);
	    timer16_0.start();
	    isactive = true;
	    return;
	    break;
	case 2:
		pinMode(PIO1_1, OUTPUT_MATCH);  // configure digital pin PIO1_1 to match MAT0 of timer32 #1  TS_ARM IO5
		timer32_1.begin();
	    timer32_1.prescaler(PRESCALER); //
	    timer32_1.matchMode(MAT0, SET);  // set the output of PIO1_1 to 1 when the timer matches MAT0
	    timer32_1.match(MAT0, pwmmax);      // match MAT0 when the timer reaches this value
	    timer32_1.pwmEnable(MAT0);       // enable PWM for match channel MAT0
	    timer32_1.matchMode(MAT3, RESET | INTERRUPT);	// Reset the timer when the timer matches MAT3 and generate an interrupt
	    timer32_1.match(MAT3, pwmmax);
	    timer32_1.start();
	    isactive = true;
	    return;
	    break;
	case 3:
		pinMode(PIO1_2, OUTPUT_MATCH);  // configure digital pin PIO1_2 to match MAT1 of timer32 #1  TS_ARM IO14
	    timer32_1.begin();
	    timer32_1.prescaler(PRESCALER); //
	    timer32_1.matchMode(MAT1, SET);  // set the output of PIO1_2 to 1 when the timer matches MAT1
	    timer32_1.match(MAT1, pwmmax);      // match MAT1 when the timer reaches this value
	    timer32_1.pwmEnable(MAT1);       // enable PWM for match channel MAT1
	    timer32_1.matchMode(MAT3, RESET | INTERRUPT);	// Reset the timer when the timer matches MAT3 and generate an interrupt
	    timer32_1.match(MAT3, pwmmax);
	    timer32_1.start();
	    isactive = true;
	    return;
	    break;
	default:
		return;
	}


}

void pwmout::setpwm(int value) {
	if (value < 0 || value > MAXOUTPUTVALUE) {
		return;
	}
	if (isactive) {
		switch (channel) {
			//int val = value*pwmmax/MAXVALUE;
			case 0:
				//timer16_0.match(MAT0, pwmmax-(value*pwmmax/MAXOUTPUTVALUE));      // match MAT0 when the timer reaches this value
				return;
				break;
			case 1:
				timer16_0.match(MAT1, pwmmax-(value*pwmmax/MAXOUTPUTVALUE));      // match MAT1 when the timer reaches this value
				return;
				break;
			case 2:
				timer32_1.match(MAT0, pwmmax-(value*pwmmax/MAXOUTPUTVALUE));      // match MAT0 when the timer reaches this value
				return;
				break;
			case 3:
				timer32_1.match(MAT1, pwmmax-(value*pwmmax/MAXOUTPUTVALUE));      // match MAT1 when the timer reaches this value
				return;
				break;
			default:
				return;
		}
	}
}
