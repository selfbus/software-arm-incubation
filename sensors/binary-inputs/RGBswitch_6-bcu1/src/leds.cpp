/*
 *  Copyright (c) 2018 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/core.h>
#include <sblib/ioports.h>
#include <sblib/spi.h>
#include <sblib/serial.h>
#include "leds.h"
#include "params.h"

//#define _DEBUG__

#define LEDS 18 // total number of LEDs (a RGB led has 3 LEDs inside) -> 6xRGB LED = 18 LEDs
#define TIMER32_0_STEP 10 //unit: ms Schrittweite der Dimmung (1..100)
#define SENSOR_TIMER_100MS 100/TIMER32_0_STEP //100ms/Timerschritte
#define LED_DIMM_TIME 500 //unit: ms Gesamtzeit der Dimmung
#define LED_DIMM_STEPS LED_DIMM_TIME/TIMER32_0_STEP //Anzahl Schritte der Dimmung
//#define MAX_LED_ON_TIME 3500 //unit: ms Zeit, die die LED nach Dimmen an sein soll

void spi_send(uint8_t *);

SPI spi(SPI_PORT_0);

int blinkPin = PIO0_7;

uint8_t rgb_buffer_ist[LEDS];	//aktueller Stand der LEDs
uint8_t rgb_buffer_soll[LEDS];  //kurzzeitige soll Vorgabe
uint8_t rgb_buffer_vorgabe[LEDS]; //von ETS eingestellte Vorgaben
uint8_t rgb_buffer_nachtlicht[3]; //von ETS eingestellte Vorgabe für Nachtlicht (eine Farbe)
bool function_leds_active = 0;
bool nightlight_configured = 0;
uint8_t brightness_leds;
uint8_t brightness_nightlight;
uint16_t time_leds_on = 0;
uint16_t time_period_blink = 0;
bool nightlight_active;
bool led_blink_state[6];
bool any_blink_led_active;
volatile int actual_period_time = 0;
volatile bool actual_blink_leds_on;

uint8_t led_lookup_table[LEDS/3] = {1,3,5,4,2,0};

struct dimmrampe {
	int16_t steigung;
	uint8_t nullstelle;
};

struct dimmrampe dimmrampen[LEDS];

volatile uint8_t flag_sens_pos = 0;
volatile uint8_t flag_sens_neg = 0;
volatile uint8_t flag_dimmen = 0;
volatile uint8_t flag_led_on = 0;
uint8_t flag_nightlight_on = 0;

volatile int differenz_median = 0;
volatile int timer32_0_cnt = 0;
volatile int led_on_time = 0;
volatile int value_without_LED = 0;
volatile int dimm_steps_cnt = 0;

#ifdef _DEBUG__
Serial Serial(PIO2_7, PIO2_8);
#endif

extern "C" void TIMER32_0_IRQHandler() {

	timer32_0.resetFlags(); /* clear MAT0 interrupt flag */

/*
 * dimming of RGB Leds
 */
	if (flag_dimmen == 1) {
		dimm_steps_cnt++;
		uint8_t flag_val_changed = 0;
		for (uint8_t i = 0; i < LEDS; i++) {
			if (rgb_buffer_ist[i] != rgb_buffer_soll[i]) {
				rgb_buffer_ist[i] = ((dimmrampen[i].steigung * dimm_steps_cnt) / 100) + dimmrampen[i].nullstelle;
				flag_val_changed = 1;
			}
		}
		if (flag_val_changed == 0) {
			flag_dimmen = 0;
			dimm_steps_cnt = 0;
		}
	}
	if (flag_led_on == 1) {
		led_on_time += TIMER32_0_STEP;
	}
	if (led_on_time == time_leds_on) {
		flag_sens_neg = 1;
		led_on_time = 0;
	}
	spi_send(rgb_buffer_ist);


/*
 * blink mode calculation
 */
	if(any_blink_led_active){

		if(actual_period_time == time_period_blink){
			for(uint8_t i = 0; i < 6; i++){
				if(led_blink_state[i]){
					if(actual_blink_leds_on){	// shut off all LEDs
						rgb_buffer_ist[rgb_leds[i].blau] = 0;
						rgb_buffer_ist[rgb_leds[i].rot] = 0;
						rgb_buffer_ist[rgb_leds[i].gruen] = 0;
					}else{	//set back the value to LEDs
						rgb_buffer_ist[rgb_leds[i].blau]  = rgb_buffer_vorgabe[rgb_leds[i].blau];
						rgb_buffer_ist[rgb_leds[i].rot]   = rgb_buffer_vorgabe[rgb_leds[i].rot];
						rgb_buffer_ist[rgb_leds[i].gruen] = rgb_buffer_vorgabe[rgb_leds[i].gruen];
					}
				}
			}
			actual_blink_leds_on = !actual_blink_leds_on;
			actual_period_time = 0;
		}
		actual_period_time += TIMER32_0_STEP;
	}


/*
 * Annäherungssensor auslesen und auswerten
 * alle 100ms wird der ADC ausgelesen (5* -> Mittelwert)
 */
	if (timer32_0_cnt == SENSOR_TIMER_100MS / 2) { //alle 100ms die Sensorabfrage durchführen

		value_without_LED = 0;

		for (int i = 0; i < 5; i++) {
			value_without_LED += analogRead(AD0);
		}

		//digitalWrite(PIO3_2, 0); //IR LED ein

		value_without_LED /= 5;
	}
	if (timer32_0_cnt == (SENSOR_TIMER_100MS - 1) ){	//kurz vor der Messung die IR LED einschalten
		digitalWrite(PIO3_2, 0); //IR LED ein
	}
	if (timer32_0_cnt == SENSOR_TIMER_100MS) {

		timer32_0_cnt = 0; //100ms Sekungen Zähler rücksetzen

		int wert_mit_LED = 0;
		int differenz_temp = 0;

		for (int i = 0; i < 5; i++) {
			wert_mit_LED += analogRead(AD0);
		}

		digitalWrite(PIO3_2, 1); //IR LED aus

		wert_mit_LED /= 5;

		differenz_temp = wert_mit_LED - value_without_LED;

		differenz_median = differenz_median * 4 + differenz_temp;

		differenz_median /= 5;

#ifdef _DEBUG__
		serial.println(value_without_LED);
		serial.println(wert_mit_LED);
		serial.println(differenz_temp);
		serial.println("");
#endif

		if (differenz_temp - differenz_median > 60) {
			flag_sens_pos = 1;

#ifdef _DEBUG__
			serial.println("positive Flanke erkannt!");
#endif
		}
	}

	timer32_0_cnt++;

	return;
}

void set_nightlight_state(int state) {
	if (state == 1 && nightlight_configured == 1) {
		nightlight_active = 1;
	}
	if (state == 0) {
		nightlight_active = 0;
	}
}

void set_blink_mode(int channel, int state) {

	// setting the information for timer interrupt
	led_blink_state[channel] = state;

	// avoid staying LED on at switch off blink mode
	if(state == 0){
		rgb_buffer_ist[rgb_leds[channel].blau]  = rgb_buffer_soll[rgb_leds[channel].blau];
		rgb_buffer_ist[rgb_leds[channel].rot]   = rgb_buffer_soll[rgb_leds[channel].rot];
		rgb_buffer_ist[rgb_leds[channel].gruen] = rgb_buffer_soll[rgb_leds[channel].gruen];
	}

	//set flag if any blinking led is active
	any_blink_led_active = 0;
	for (uint8_t i = 0; i < 6; i++) {
		if (led_blink_state[i] == 1) {
			any_blink_led_active = 1;
		}
	}
}

uint16_t translate_value_to_time(uint8_t value) {

	uint16_t ret_val = 0;

	switch (value) {					//translate ETS setting to ms
	case 0:
		ret_val = 100;
		break;
	case 1:
		ret_val = 250;
		break;
	case 2:
		ret_val = 500;
		break;
	case 3:
		ret_val = 750;
		break;
	case 4:
		ret_val = 1000;
		break;
	case 5:
		ret_val = 1500;
		break;
	case 6:
		ret_val = 2000;
		break;
	case 7:
		ret_val = 3000;
		break;
	case 8:
		ret_val = 4000;
		break;
	case 9:
		ret_val = 5000;
		break;
	case 10:
		ret_val = 6000;
		break;
	case 11:
		ret_val = 7000;
		break;
	case 12:
		ret_val = 8000;
		break;
	case 13:
		ret_val = 9000;
		break;
	case 14:
		ret_val = 10000;
		break;
	}
	return ret_val;
}

/*
 * Initialize the application.
 */
void initLEDs() {
	pinMode(blinkPin, OUTPUT);

	pinMode(PIO0_2, OUTPUT | SPI_SSEL);
	pinMode(PIO0_9, OUTPUT | SPI_MOSI);
	pinMode(PIO0_6, OUTPUT | SPI_CLOCK);

	spi.setClockDivider(128);
	spi.begin();

	analogBegin();
	pinMode(PIO0_11, INPUT_ANALOG); //LDR

	pinMode(PIO3_2, OUTPUT); //IR LED
	digitalWrite(PIO3_2, 0);

	// Enable the serial port with 19200 baud, no parity, 1 stop bit
#ifdef _DEBUG__
	serial.begin(19200);
	serial.println("WS2803 Taster TEST");
#endif
	for (int i = 0; i < LEDS; i++) {
		rgb_buffer_ist[i] = 0;
	}

	// LED base + byte 2 bis 6 (bit 0-4) beinhalten die Farbvorgaben
	function_leds_active = LEDparams[1] >> 7;

	if (function_leds_active) {
		uint8_t led_color_param;

		brightness_leds = LEDparams[1] & 0x7F;
		time_leds_on = translate_value_to_time(LEDparams[5] & 0x0F);
		time_period_blink = translate_value_to_time(LEDparams[6] >> 4);

		uint8_t LEDnumber;

		for (uint8_t i = 0; i < LEDS / 3; i++) {

			LEDnumber = led_lookup_table[i];

			if (LEDnumber % 2 == 0) {  //gerade Zahlen
				led_color_param = LEDparams[2 + ((LEDnumber + 1) / 2)] & 0x0F;
			} else {
				led_color_param = LEDparams[2 + ((LEDnumber + 1) / 2)] >> 4;
			}


			rgb_buffer_vorgabe[rgb_leds[i].blau] = (farbauswahl[led_color_param].blau * brightness_leds) / 100;
			rgb_buffer_vorgabe[rgb_leds[i].rot] = (farbauswahl[led_color_param].rot * brightness_leds) / 100;
			rgb_buffer_vorgabe[rgb_leds[i].gruen] = (farbauswahl[led_color_param].gruen * brightness_leds) / 100;
		}
	}

	nightlight_configured = LEDparams[0] >> 7;

	if (nightlight_configured) {

		brightness_nightlight = LEDparams[0] & 0x7F;

		rgb_buffer_nachtlicht[rgb_leds[0].blau] = (farbauswahl[LEDparams[2] >> 4].blau * brightness_nightlight) / 100;
		rgb_buffer_nachtlicht[rgb_leds[0].rot] = (farbauswahl[LEDparams[2] >> 4].rot * brightness_nightlight) / 100;
		rgb_buffer_nachtlicht[rgb_leds[0].gruen] = (farbauswahl[LEDparams[2] >> 4].gruen * brightness_nightlight) / 100;
	}

	// Enable the timer interrupt
	enableInterrupt(TIMER_32_0_IRQn);

	// Begin using the timer
	timer32_0.begin();

	// Let the timer count milliseconds
	timer32_0.prescaler((SystemCoreClock / 1000) - 1);

	// On match of MAT0, generate an interrupt and reset the timer
	timer32_0.matchMode(MAT0, RESET | INTERRUPT);

	// Match MAT0 when the timer reaches this value (in milliseconds)
	timer32_0.match(MAT0, TIMER32_0_STEP); //10ms period

	// set Timer priority lower than normal, because sblib interrupts have to be served with highest priority (prio = 0)
	NVIC_SetPriority(TIMER_32_0_IRQn, 1);

	timer32_0.start();

}

void spi_send(uint8_t *value) {
	for (int Out = 0; Out < LEDS; Out++) {
		spi.transfer(value[Out]);
	}
	delayMicroseconds(600);
}

/*
 * The main processing loop.
 */
void LEDPeriod() {
	if (flag_sens_pos && !flag_dimmen) {
		for (uint8_t i = 0; i < LEDS / 3; i++) {
			rgb_buffer_soll[rgb_leds[i].blau] = rgb_buffer_vorgabe[rgb_leds[i].blau];
			rgb_buffer_soll[rgb_leds[i].rot] = rgb_buffer_vorgabe[rgb_leds[i].rot];
			rgb_buffer_soll[rgb_leds[i].gruen] = rgb_buffer_vorgabe[rgb_leds[i].gruen];
		}

		for (uint8_t i = 0; i < LEDS; i++) {
			dimmrampen[i].steigung = ((rgb_buffer_soll[i] - rgb_buffer_ist[i]) * 100)
					/ (LED_DIMM_TIME / TIMER32_0_STEP);
			dimmrampen[i].nullstelle = rgb_buffer_ist[i];
		}
		flag_sens_pos = 0;
		flag_dimmen = 1;
		flag_led_on = 1;
	}

	if (flag_sens_neg && !flag_dimmen && !nightlight_active) {
		for (uint8_t i = 0; i < LEDS; i++) {
			rgb_buffer_soll[i] = 0x00;
		}

		for (uint8_t i = 0; i < LEDS; i++) {
			dimmrampen[i].steigung = ((rgb_buffer_soll[i] - rgb_buffer_ist[i]) * 100)
					/ (LED_DIMM_TIME / TIMER32_0_STEP);
			dimmrampen[i].nullstelle = rgb_buffer_ist[i];
		}
		flag_sens_neg = 0;
		flag_dimmen = 1;
		flag_led_on = 0;
	}

	if (nightlight_active && !flag_dimmen && (!flag_led_on || flag_sens_neg)) {
		for (uint8_t i = 0; i < LEDS / 3; i++) {
			rgb_buffer_soll[rgb_leds[i].blau] = rgb_buffer_nachtlicht[rgb_leds[0].blau];
			rgb_buffer_soll[rgb_leds[i].rot] = rgb_buffer_nachtlicht[rgb_leds[0].rot];
			rgb_buffer_soll[rgb_leds[i].gruen] = rgb_buffer_nachtlicht[rgb_leds[0].gruen];
		}

		for (uint8_t i = 0; i < LEDS; i++) {
			dimmrampen[i].steigung = ((rgb_buffer_soll[i] - rgb_buffer_ist[i]) * 100)
					/ (LED_DIMM_TIME / TIMER32_0_STEP);
			dimmrampen[i].nullstelle = rgb_buffer_ist[i];
		}
		flag_dimmen = 1;
		flag_sens_neg = 0;
		flag_nightlight_on = 1;
	}

	if (!nightlight_active && flag_nightlight_on && !flag_dimmen && !flag_led_on) {
		for (uint8_t i = 0; i < LEDS; i++) {
			rgb_buffer_soll[i] = 0x00;
		}

		for (uint8_t i = 0; i < LEDS; i++) {
			dimmrampen[i].steigung = ((rgb_buffer_soll[i] - rgb_buffer_ist[i]) * 100)
					/ (LED_DIMM_TIME / TIMER32_0_STEP);
			dimmrampen[i].nullstelle = rgb_buffer_ist[i];
		}
		flag_dimmen = 1;
		flag_nightlight_on = 0;
	}
}

