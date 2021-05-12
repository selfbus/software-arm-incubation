/*
 *  Copyright (c) 2018 Oliver Stefan <o.stefan252@gmail.com>
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
 * Funktion 		Parameter				Adresse		Offset		Größe (bit)
 *
 * Nachtlicht 		Nachtlicht aktiv		493			0			1
 * 					Helligkeit				493			1			7
 * 					Farbe					495			0			4
 *
 * Funktion-Info	Funktion-Info aktiv		494			0			1
 *					Helligkeit				494			1			7
 *					Taster 1 LED Farbe		495			4			4
 *					Taster 2 LED Farbe		496			0			4
 *					Taster 3 LED Farbe		496			4			4
 *					Taster 4 LED Farbe		497			0			4
 *					Taster 5 LED Farbe		497			4			4
 *					Taster 6 LED Farbe		498			0			4
 *					Zeit LED Aktivierung	498			4			4
 *					Periodenzeit blinken	499			0 			4
 *
 * Offset = 0 -> Bits 7..4 (bei 4 bit Breite)
 * Offset = 4 -> Bits 3..0 (bei 4 bit Breite)
 */

/*
 * COM Objekte in .vd
 *
 * COM Objekt			COM Objekt Nummer	Größe
 *
 * Nachtlicht aktiv		6					1 bit
 * Taster 1 blinken		7					1 bit
 * Taster 2 blinken		14					1 bit
 * Taster 3 blinken		15					1 bit
 * Taster 4 blinken		22					1 bit
 * Taster 5 blinken		23					1 bit
 * Taster 6 blinken		24					1 bit
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

struct rgb_led{
	uint8_t blau;
	uint8_t rot;
	uint8_t gruen;
};

enum farben { Rot, Gruen, Blau, Tuerkis, Gelb, Orange, Violett, Rosa, ANZAHL_FARBEN };

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



#endif
