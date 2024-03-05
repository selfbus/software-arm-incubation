/*
 *  Copyright (c) 2016-2021 Oliver Stefan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <u8g_arm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sblib/timeout.h>
#include "app_rtr.h"
#include "lcd.h"
#include "sensors.h"
#include "config.h"
#include "params.h"
#include "pwm.h"
#include "inputs.h"

uint8_t draw_state = 0;

u8g_t u8g;

//unsigned int LCDBrightness = 500;

struct lcd_home_screen window_ventilation;

bool KeyPressedFlag;
bool BacklightOnFlag;

enum TEXT_PARAM {
	parameter, text
};

int sollTempMem = 0;

char const *HandAuto[2][5] = { "Hand", "Auto" };
char const *GradCelsius[1][3] = { "\260C" };
char const *Minuten[1][8] = { " min" };
char const *dummy[1][1] = { "" };
//enum enumHandAuto {Auto, Hand};

struct menu_entrys {
	bool text_param;
	char const *text;
	int userFlashPointer;
	char const **unit;
	int ParMin;
	int ParMax;
	int stepSize;
} menu_entry[] = {
#if EINHEIZKREIS
	{ parameter, "Solltemp.: ", 	UF_TEMP_SOLL_INTERN, 		*GradCelsius, 	0, 3500,  50},
#else
	{parameter,	"Temp. Luft: ", 	UF_TEMP_SOLL_INTERN, 		*GradCelsius, 	0, 3500,  50},
	{parameter, "Temp. Boden: ",	UF_TEMP_SOLL_INTERN_LUFT,	*GradCelsius,	0, 3500,  50},
#endif

	{ parameter, "Helligkeit: ", 	UF_LCD_BRIGHTNESS, 			*dummy, 		0, 1000,  100},
	{ text, 	 "Soll-Temp: ", 	UF_TEMP_SOLL_TEMP_FLAG, 	*HandAuto, 		0, 1,     1},
	{ parameter, "Hand Reset: ",	UF_TEMP_AUTO_RESET_TIME, 	*Minuten, 		0, 72000, 100} //TODO 1000, 72000, 1000 =10min, 720min, 10min
};

#if EINHEIZKREIS
#define ALL_MENU_ITEMS 4
#else
#define ALL_MENU_ITEMS 5
#endif

uint8_t menu_items;

enum MENU_ENTRY_NAMES {
	sollTemperatur,
#if not EINHEIZKREIS
	temperatur_boden,
#endif
	helligkeit, sollTemperaturModus, return_to_auto
};

void init_lcd(){
	menu_items = ALL_MENU_ITEMS;

	// wenn keine externe Solltemperatur verwendet wird, wird auch keine Umschatung zwischen Hand/Auto benötigt
	if(!use_external_target_temperature){
		menu_items -= 2;
	}

}

unsigned int menu_current;
uint8_t currentScreen;
bool screen_redraw_required;
bool modifyMenu;

uint8_t draw_float(uint8_t x_start, uint8_t y_height, int fak_100_val) {
	char string[3];
	u8g_uint_t width_vork, width_komma, width_nachk;

	itoa(fak_100_val / 100, string, 10);
	width_vork = u8g_DrawStr(&u8g, x_start, y_height, string);
	width_komma = u8g_DrawStr(&u8g, x_start + width_vork, y_height, ",");
	itoa(fak_100_val % 100 / 10, string, 10);
	width_nachk = u8g_DrawStr(&u8g, x_start + width_vork + width_komma, y_height, string);
	return x_start + width_vork + width_komma + width_nachk;
}

void draw_home_screen(void) {
	u8g_SetDefaultForegroundColor(&u8g);

#if VOC_SENSOR_INSTALLED
	//draw temperatures and air quality
#if TEMP_SENS_INSTALLED
	uint8_t x_next;
#if not EXTERNAL_TEMP_SENS
	u8g_SetFont(&u8g, u8g_font_fub20);
	x_next = draw_float(0, 20, temp.tempIntern);
	char temp_string[2];
	temp_string[0] = 176;
	temp_string[1] = 'C';
	u8g_DrawStr(&u8g, x_next, 20, temp_string);

#else // device with two heating circles
	u8g_uint_t strwidth;
	u8g_SetFont(&u8g, u8g_font_fub14);
	x_next = draw_float(8, 14, temp.tempIntern);
	char string[2];
	string[0]= 176;
	string[1] = 'C';
	u8g_DrawStr(&u8g, x_next, 14, string);
	if(temp.floorTempShow){
		u8g_SetFont(&u8g, u8g_font_6x13);
		strwidth = u8g_DrawStr(&u8g, 0, 27, "Boden: ");
		x_next = draw_float(strwidth, 27, temp.tempExtern);
		u8g_DrawStr(&u8g, x_next, 27, string);
	}
#endif // EXTERNAL_TEMP_SENS
#endif // TEMP_SENS_INSTALLED

#else // device without VOC sensor

	//draw temperatures and air quality
#if TEMP_SENS_INSTALLED
	uint8_t x_next;
#if not EXTERNAL_TEMP_SENS
	u8g_SetFont(&u8g, u8g_font_fub20);
	x_next = draw_float(0, 20, temp.tempIntern);
	char string[2];
	string[0]= 176;
	string[1] = 'C';
	u8g_DrawStr(&u8g, x_next, 20, string);
#else // device with two heating circles
	u8g_uint_t strwidth;
	u8g_SetFont(&u8g, u8g_font_fub14);
	x_next = draw_float(8, 14, temp.tempIntern);
	char string[2];
	string[0]= 176;
	string[1] = 'C';
	u8g_DrawStr(&u8g, x_next, 14, string);

	if(temp.floorTempShow){
		u8g_SetFont(&u8g, u8g_font_6x13);
		strwidth = u8g_DrawStr(&u8g, 0, 27, "Boden: ");
		x_next = draw_float(strwidth, 27, temp.tempExtern);
		u8g_DrawStr(&u8g, x_next, 27, string);
	}
#endif // TEMP_SENS_INSTALLED
#endif // EXTERNAL_TEMP_SENS
#endif // VOC_SENSOR_INSTALLED

#if HUMIDITY_SENSOR_INSTALLED
	uint8_t hum_x_next;
	u8g_SetFont(&u8g, u8g_font_fub14r);
	hum_x_next = draw_float(0, 44, air_humidity.AirRH);
	u8g_DrawStr(&u8g, hum_x_next, 44, "%RH");
#endif // HUMIDITY_SENSOR_INSTALLED

#if VOC_SENSOR_INSTALLED
	u8g_SetFont(&u8g, u8g_font_profont10r);
	u8g_DrawStr(&u8g, 94, 6, "Air");
	u8g_DrawStr(&u8g, 94, 13, "Quality");
	u8g_DrawXBMP(&u8g, 95, 15, smily_base_width, smily_base_height, smily_base_bits);

	if (air_quality.IAQcondition == IAQgood) {
		u8g_DrawXBMP(&u8g, 102, 32, mouth_happy_width, mouth_happy_height, mouth_happy_bits);
	} else if (air_quality.IAQcondition == IAQneutral) {
		u8g_DrawXBMP(&u8g, 103, 36, mouth_neutral_width, mouth_neutral_height, mouth_neutral_bits);
	} else if (air_quality.IAQcondition == IAQbad) {
		u8g_DrawXBMP(&u8g, 102, 31, mouth_sad_width, mouth_sad_height, mouth_sad_bits);
	}

	char voc_string[3];
	itoa(air_quality.AirCO2, voc_string, 10);
	u8g_DrawStr(&u8g, 88, 63, voc_string);

	u8g_uint_t d;
	d = u8g_GetStrWidth(&u8g, voc_string);
	u8g_DrawStr(&u8g, 88 + d, 63, " ppm");

#endif // VOC_SENSOR_INSTALLED

	if (window_ventilation.show_ventilation_state) {
		u8g_DrawXBMP(&u8g, 5, 50, luefter_width, luefter_height, luefter_bits);
		u8g_SetFont(&u8g, u8g_font_profont10r);
		char ventilation_state_string[6];
		itoa(window_ventilation.ventilation_level, ventilation_state_string, 10);
		u8g_DrawStr(&u8g, 22, 63, ventilation_state_string);
	}
	if (window_ventilation.show_window_state) {
		if (window_ventilation.window_state == true) {
			u8g_DrawXBMP(&u8g, 33, 50, window_open_width, window_open_height, window_open_bits);
		} else {
			u8g_DrawXBMP(&u8g, 36, 50, window_closed_width, window_closed_height, window_closed_bits);
		}
	}
	if (window_ventilation.show_heating_state){
		u8g_DrawXBMP(&u8g, 57, 50, heating_width, heating_height, heating_bits);
		u8g_SetFont(&u8g, u8g_font_profont10r);

		int soll_temp_flag = memMapper.getUInt32(UF_TEMP_SOLL_TEMP_FLAG);
		if (soll_temp_flag == SollTempIntern) {
			u8g_DrawStr(&u8g, 73, 63, "H");
		} else {
			u8g_DrawStr(&u8g, 73, 63, "A");
		}
	}
}

void checkLCDperiodic(void) {
	if (timeout[LCD_HOME_SCREEN].expired()) {
		currentScreen = HOMESCREEN;
		modifyMenu = false;
		memMapper.doFlash();

		// wenn der Menüpunkt "Solltemperatur" verlassen wurde
		if(menu_current == sollTemperatur){
			sendTargetTemperature();
		}
	}
	if (timeout[LCD_BACKLIGHT].expired()) {
		setPWM(0);
		BacklightOnFlag = false;
	}
}

void drawLCD(void) {
	if (currentScreen == HOMESCREEN) {
		draw_home_screen();
	} else if (currentScreen == MENU) {
		if (screen_redraw_required) {
			draw_menu();
		}
	}
}

void lcdMenu(int inputChannel, int value, uint8_t pressType) {

	if (BacklightOnFlag) {

		timeout[LCD_HOME_SCREEN].start(MENUTIME);

		if ((pressType == LONG_PRESS) && (currentScreen == MENU)) {
			if (modifyMenu == false) {
				modifyMenu = true;
			} else {
				modifyMenu = false;
				// wenn der Menüpunkt "Solltemperatur" verlassen wurde
				if(menu_current == sollTemperatur){
					sendTargetTemperature();
				}
			}
			KeyPressedFlag = true;
			screen_redraw_required = true;
		}

		if ((value == 1) && (modifyMenu == false) && (currentScreen == MENU) && (KeyPressedFlag == false)) {
			if (inputChannel == 1) {
				menu_current++;
				if (menu_current >= menu_items)
					menu_current = 0;
				screen_redraw_required = 1;
			} else if (inputChannel == 0) {
				if (menu_current == 0)
					menu_current = menu_items;
				menu_current--;
				screen_redraw_required = 1;
			}
		}
		if ((value == 1) && (modifyMenu == true) && (currentScreen == MENU) && (KeyPressedFlag == false)) {
			int uf_parameter;
			uf_parameter = memMapper.getUInt32(menu_entry[menu_current].userFlashPointer);
			if (inputChannel == 0) {
				if (uf_parameter == menu_entry[menu_current].ParMax) {
					uf_parameter = menu_entry[menu_current].ParMin;
				} else if (uf_parameter < menu_entry[menu_current].ParMax) {
					uf_parameter = uf_parameter + menu_entry[menu_current].stepSize;
				}
				screen_redraw_required = 1;
			} else if (inputChannel == 1) {
				if (uf_parameter == menu_entry[menu_current].ParMin) {
					uf_parameter = menu_entry[menu_current].ParMax;
				} else if (uf_parameter > menu_entry[menu_current].ParMin) {
					uf_parameter = uf_parameter - menu_entry[menu_current].stepSize;
				}
				screen_redraw_required = 1;
			}
			memMapper.setUInt32(menu_entry[menu_current].userFlashPointer, uf_parameter);

			if (menu_current == helligkeit) {
				setPWM(memMapper.getUInt32(UF_LCD_BRIGHTNESS)/100);
			}
#if EINHEIZKREIS
			if (menu_current == sollTemperatur) { //wenn an der Soll-Temperatur etwas verändert wird, wird auf Handbetrieb geschaltet
#else
			if(menu_current == temperatur || menu_current == temperatur_boden){ //wenn an der Soll-Temperatur etwas verändert wird, wird auf Handbetrieb geschaltet
#endif
				int soll_temp_flag = SollTempIntern;
				memMapper.setUInt32(UF_TEMP_SOLL_TEMP_FLAG, soll_temp_flag);
			}

		}

		if (value == 1) { //sobald eine Taste gedrückt wird, wird das Menü angezeigt
			currentScreen = MENU;
			screen_redraw_required = 1;
		}

		if (value == 1) { //Hilfsflag, um unerwünschte Aktionen beim erstmaligen drücken einer Taste zu unterbinden
			KeyPressedFlag = false;
		}
	}

	if (value == 1) {
		timeout[LCD_BACKLIGHT].start(LCDBACKLIGHTTIME);
		setPWM(memMapper.getUInt32(UF_LCD_BRIGHTNESS)/100);
		BacklightOnFlag = true;
	}
}

void draw_menu(void) {
	uint8_t i, h;
	u8g_uint_t w, d, d2;
	char string[5];
	u8g_SetFont(&u8g, u8g_font_6x13);
	u8g_SetFontRefHeightText(&u8g);
	u8g_SetFontPosTop(&u8g);
	h = u8g_GetFontAscent(&u8g) - u8g_GetFontDescent(&u8g);
	w = u8g_GetWidth(&u8g);
	for (i = 0; i < menu_items; i++) {        // draw all menu items

		// wenn keine externe Solltemperatur verwendet wird, wird auch keine Umschaltung zwischen Hand/Auto benötigt
		if((i == return_to_auto || i == sollTemperaturModus) && !use_external_target_temperature){
			continue; // Schleifendurchlauf überspringen
		}

		int uf_parameter = memMapper.getUInt32(menu_entry[i].userFlashPointer);
		d = u8g_GetStrWidth(&u8g, menu_entry[i].text);
		itoa(uf_parameter, string, 10);
		if (menu_entry[i].text_param == parameter) {
			d2 = u8g_GetStrWidth(&u8g, string);
		} else {
			d2 = u8g_GetStrWidth(&u8g, menu_entry[i].unit[uf_parameter]);
		}
		if (uf_parameter == 0) {
			d2 = d2 + 12;
		}
		u8g_SetDefaultForegroundColor(&u8g); //setze schwarz
		if (i == menu_current) {               // current selected menu item
			if (modifyMenu) {
				u8g_DrawBox(&u8g, d - 1, i * h + 1, d2, h);     // draw cursor bar
				u8g_DrawStr(&u8g, 0, i * h, menu_entry[i].text);
				u8g_SetDefaultBackgroundColor(&u8g); //setze weiß
				if (menu_entry[i].text_param == parameter) {
					draw_float(d, i * h, uf_parameter);
					u8g_SetDefaultForegroundColor(&u8g); //setze schwarz
					u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[0]);
				} else {
					d2 = 0;
					u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[uf_parameter]);
				}

			} else {
				u8g_DrawBox(&u8g, 0, i * h + 1, w, h);     // draw cursor bar
				u8g_SetDefaultBackgroundColor(&u8g); //setze weiß
				u8g_DrawStr(&u8g, 0, i * h, menu_entry[i].text);
				if (menu_entry[i].text_param == parameter) {
					draw_float(d, i * h, uf_parameter);
					u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[0]);
				} else {
					d2 = 0;
					u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[uf_parameter]);
				}
			}
		} else {
			u8g_DrawStr(&u8g, 0, i * h, menu_entry[i].text);
			if (menu_entry[i].text_param == parameter) {
				draw_float(d, i * h, uf_parameter);
				u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[0]);
			} else {
				d2 = 0;
				u8g_DrawStr(&u8g, d + d2, i * h, menu_entry[i].unit[uf_parameter]);
			}
		}
	}
}
