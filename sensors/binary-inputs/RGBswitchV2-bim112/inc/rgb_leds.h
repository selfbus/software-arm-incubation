/*
 *  Copyright (c) 2018 - 2025 Oliver Stefan <o.stefan252@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef LEDs_H_
#define LEDs_H_

/*
 * LED adress table in .vd
 *
 * Funktion 		Parameter				Adresse		Bit Offset		Größe (bit)
 *
 * Nachtlicht 		Nachtlicht aktiv		46A9	681		0			1
 * 					Helligkeit				46A9	681		1			7
 * 					Farbe					46AB	683		0			4
 *
 * Funktion-Info	Funktion-Info aktiv		46AA	682		0			1
 *					Helligkeit				46AA	682		1			7
 *					Taster 1 LED Farbe		46AB	683		4			4
 *					Taster 2 LED Farbe		46AC	684		0			4
 *					Taster 3 LED Farbe		46AC	684		4			4
 *					Taster 4 LED Farbe		46AD	685		0			4
 *					Taster 5 LED Farbe		46AD	685		4			4
 *					Taster 6 LED Farbe		46AE	686		0			4
 *					Zeit LED Aktivierung	46AE	686		4			4
 *					Periodenzeit blinken	46AF	687		0 			4
 *
 * Offset = 0 -> Bits 7..4 (bei 4 bit Breite)
 * Offset = 4 -> Bits 3..0 (bei 4 bit Breite)
 */

/*
 * COM Objekte in .vd
 *
 * COM Objekt			COM Objekt Nummer	Größe
 *
 * Nachtlicht aktiv		30					1 bit
 * Taster 1 blinken		31					1 bit
 * Taster 2 blinken		32					1 bit
 * Taster 3 blinken		33					1 bit
 * Taster 4 blinken		34					1 bit
 * Taster 5 blinken		35					1 bit
 * Taster 6 blinken		36					1 bit
 */

void initLEDs(void);
void LEDPeriod(void);


/*
 * aktiviert in Abhängigkeit der Programmierung die Nachtlichtfunktion
 */
void set_nightlight_state(int state);

/*
 * activate/deactivate blinking of LEDs with programmed time
 */
void set_blink_mode(int channel, int state);

/*
 * translates a ETS time value to milliseconds
 */
uint16_t translate_value_to_time(uint8_t value);

#define LEDS 18 // total number of LEDs (a RGB led has 3 LEDs inside) -> 6xRGB LED = 18 LEDs

struct rgb_led{
	uint8_t blau;
	uint8_t rot;
	uint8_t gruen;
};

enum farben { Rot, Gruen, Blau, Tuerkis, Gelb, Orange, Violett, Rosa, ANZAHL_FARBEN };

extern const byte* LEDparams;

const struct rgb_led farbauswahl[ANZAHL_FARBEN] = {
		{	0,		//Farbe Rot Anteil LED blau
			255,	//Farbe Rot Anteil LED rot
			0		//Farbe Rot Anteil LED gruen
		},
		{	0,		//Farbe Gruen Anteil LED blau
			0,		//Farbe Gruen Anteil LED rot
			255		//Farbe Gruen Anteil LED gruen
		},
		{	255,	//Farbe Blau Anteil LED blau
			0,		//Farbe Blau Anteil LED rot
			0		//Farbe Blau Anteil LED gruen
		},
		{	238,	//Farbe Tuerkis Anteil LED blau
			0,		//Farbe Tuerkis Anteil LED rot
			229		//Farbe Tuerkis Anteil LED gruen
		},
		{	0,		//Farbe Gelb Anteil LED blau
			255,	//Farbe Gelb Anteil LED rot
			60		//Farbe Gelb Anteil LED gruen
		},
		{	0,		//Farbe Orange Anteil LED blau
			255,	//Farbe Orange Anteil LED rot
			25		//Farbe Orange Anteil LED gruen
		},
		{	226,	//Farbe Violett Anteil LED blau
			255,	//Farbe Violett Anteil LED rot
			43		//Farbe Violett Anteil LED gruen
		},
		{	100,	//Farbe Rosa Anteil LED blau
			255,	//Farbe Rosa Anteil LED rot
			20		//Farbe Rosa Anteil LED gruen
		},
};

const struct rgb_led rgb_leds[6] = {
		{0,1,2},
		{3,4,5},
		{6,7,8},
		{9,10,11},
		{12,13,14},
		{15,16,17}
};

// Die Zuordnung der LEDs zu den Tastern (geordnet nach LED Ausgängen am WS2803)
// z.B. Kanal 1 ist an LED 2
const uint8_t led_channel_to_switch_table[LEDS/3] = {1,3,5,4,2,0};

// Die Zuordnung der LEDs zu den Tastern (geordnet nach Taster Kanälen)
// z.B. LED 6 ist bei Taster 1
const uint8_t switch_to_led_channel_table[LEDS/3] = {5,0,4,1,3,2};

#endif
