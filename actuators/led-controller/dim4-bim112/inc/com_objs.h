/*
 * com_objs.h
 *
 *  Created on: 10.01.2020
 *      Author: x
 *
 *      Kommunikationsobjekte & Speicheradressen für
 *      AKD-0424V.01 RGBW LED Controller  Vers. 1.4
 */

#ifndef COM_OBJS_H_
#define COM_OBJS_H_

/*
 *
 * Objekte
 *
 */

#define OFSCHANNELOBJECTS	15 // Objektnummer = (Kanal-1)*OFSCHANNELOBJECTS + Objektoffset
#define OBJ_SWITCH			0  // 1 bit   - Rx - Trigger - Das eigentliche "Switch"-Objekt, "1" Einschalten, "0" Ausschalten
#define OBJ_STAIRS			1  // 1 bit   - Rx - Trigger - Treppenlicht
#define OBJ_DIM_REL			2  // 4 bit   - Rx - Trigger - Dimmen Relativ
#define OBJ_DIM_ABS			3  // 1 byte  - Rx - Trigger - Dimmen Absolut
#define OBJ_STAT_SWITCH		4  // 1 bit   - Rd/Tx - Zustand - Status An/Aus
#define OBJ_STAT_DIM		5  // 1 byte  - Rd/Tx - Zustand - Dimmwert
#define OBJ_BLOCKING1		6  // 1 bit   - Rx - Trigger - Sperren 1
#define OBJ_BLOCKING2		7  // 1 bit   - Rx - Trigger - Sperren 2
#define OBJ_SCENE			8  // 1 byte  - Rx - Trigger - Szene
//#define OBJ_				9
//#define OBJ_ 				10
#define OBJ_AUTO_BASE		10 // OBJ = OBJ_AUTO_BASE + AUTO (1-4)
#define OBJ_AUTO_1			11 // 1 bit   - Rx - Trigger - Automatik 1
#define OBJ_AUTO_2			12 // 1 bit   - Rx - Trigger - Automatik 2
#define OBJ_AUTO_3			13 // 1 bit   - Rx - Trigger - Automatik 3
#define OBJ_AUTO_4			14 // 1 bit   - Rx - Trigger - Automatik 4

//Zentralfunktionen
#define OBJ_CENTRAL_BASE	60
#define OBJ_C_SWITCH		60 // 1 bit   - Rx - Trigger - Zentral Ein/Aus
#define OBJ_C_DIM_ABS		61 // 1 byte  - Rx - Trigger - Zentral Dimmen absolut
//#define OBJ_C_ERROR		62 // 1 bit   - Rd/Tx - Zustand - Gerätefehler
#define OBJ_RGBW_SWITCH		63 // 1 bit   - Rx - Trigger - LED RGBW Ein/Aus
#define OBJ_RGBW_COLOR		64 // 3 byte  - Rx - Trigger - LED RGBW Dimmen absolut
#define OBJ_HSV_COLOR		65 // 3 byte  - Rx - Trigger - LED HSV Dimmen absolut
#define OBJ_HUE_ABS			66 // 1 byte  - Rx - Trigger - LED HSV Farbwert absolut
#define OBJ_SAT_ABS			67 // 1 byte  - Rx - Trigger - LED HSV Sättigung absolut
#define OBJ_VAL_ABS			68 // 1 byte  - Rx - Trigger - LED HSV Helligkeit absolut
#define OBJ_HUE_REL			69 // 4 bit   - Rx - Trigger - LED HSV Farbwert relativ
#define OBJ_SAT_REL			70 // 4 bit   - Rx - Trigger - LED HSV Sättigung relativ
#define OBJ_VAL_REL			71 // 4 bit   - Rx - Trigger - LED HSV Helligkeit relativ
// #define OBJ_				72
#define OBJ_STAT_RGBW		73 // 3 byte  - Rd/Tx - Zustand - LED RGBW
#define OBJ_STAT_HSV		74 // 3 byte  - Rd/Tx - Zustand - LED HSV
#define OBJ_STAT_HUE		75 // 1 byte  - Rd/Tx - Zustand - LED Farbe
#define OBJ_STAT_SAT		76 // 1 byte  - Rd/Tx - Zustand - LED Sättigung
#define OBJ_STAT_VAL		77 // 1 byte  - Rd/Tx - Zustand - LED Helligkeit
#define OBJ_RGBW_SCENE		78 // 1 byte  - Rx - Trigger - LED Szene
#define OBJ_RGBW_SCENE1		79 // 1 bit   - Rx - Trigger - LED Szene1 1Bit
#define OBJ_RGBW_SCENE2		80 // 1 bit   - Rx - Trigger - LED Szene2 1Bit
// #define OBJ_				81
// #define OBJ_				82
#define OBJ_SEQ_BASE		82 //  OBJ = OBJ_SEQ_BASE + Seqenz Nr. 1-5
#define OBJ_SEQ_1			83 // 1 bit   - Rx - Trigger - Start Sequenz 1
#define OBJ_SEQ_2			84 // 1 bit   - Rx - Trigger - Start Sequenz 2
#define OBJ_SEQ_3			85 // 1 bit   - Rx - Trigger - Start Sequenz 3
#define OBJ_SEQ_4			86 // 1 bit   - Rx - Trigger - Start Sequenz 4
#define OBJ_SEQ_5			87 // 1 bit   - Rx - Trigger - Start Sequenz 5

#define OBJ_C_OVERTEMP		88 // 1 bit   - Rd/Tx - Zustand - Übertemperaturalarm
#define OBJ_C_OVERCURR		89 // 1 bit   - Rd/Tx - Zustand - Übertstromalarm
#define OBJ_C_RELAY			90 // 1 bit   - Rx - Trigger - Relais schalten


/*
 *
 * EPROM-Adressen
 *
 */
#define APP_STARTADDR 0x4400

#define APP_NO_OFF_CHANNELS	0x45A0 // Anzahl Kanäle 1-4
#define APP_DIMM_CURVE      0x4762 // Dimmkurve quadratisch/0; halb-log/2; lin/7
#define APP_PWM_O           0x476C // PWM Frequenz 0x600/ 600Hz;  0xA00/ 1kHz
#define APP_STARTUP_TIMEOUT 0x46F6 // Geräteanlaufzeit in s
#define APP_FUNCTION_SEL	0x470E // 0/Dimmen 4fach; 1/RGB; 2/RGBW
#define APP_USE_RELAY		0x4764 // 0/extra Schaltkanal; 1/aus wenn alle Kanäle 0

// Kanäle A-D
#define APP_CH_OFFS			0x50  	// Offset
#define APP_ON_DELAY        0x45A4 	// Einschaltverzögerung in sek.
#define APP_OFF_DELAY		0x45A6 	// Ausschaltverzögerung in sek.
#define APP_STAIR_DUR		0x45A8	// Dauer Treppenlichtfunktion
#define APP_MIN_LIGHT		0x45AC	// minimale Helligkeit
#define APP_MAX_LIGHT		0x45AD	// maximale Helligkeit
#define APP_START_UP_VAL    0x45AE	// Einschalthelligkeit
#define APP_START_BEHAVIOUR	0x45AF	// Einschaltverhalten 0/Einschalthelligkeit; 1/letzter Wert
#define APP_START_BEHAVIOUR_M 0x08
#define APP_SW_OFF_REL		0x45AF	// Ausschalten bei relativem Dimmen
#define APP_SW_OFF_REL_M    0x02
#define APP_STAIR_DIM		0x45B1	// Treppenlicht bei Vorwarnung abdimmen auf ... %
#define APP_STAIR_PREWARN	0x45B2	// Treppenlicht Vorwarnzeit
#define APP_STAIR_EXTENSION	0x45AF  // Treppenlicht Verlängern
#define APP_STAIR_EXTENSION_M	0x10
#define APP_STAIR_DEACT		0x45AF  // Treppenlicht vorzeitig Ausschalten
#define APP_STAIR_DEACT_M	0x20


#define APP_SEND_DIM_VAL	0x45DA	// Dimmwert nach Änderung senden
#define APP_REL_SPEED		0x45B0	// Geschwindigkeit relativ Dimmen
#define APP_ABS_SPEED		0x45EA	// Geschwindigkeit absolut Dimmen
#define APP_ON_SPEED		0x45E2	// Einschaltgeschwindigkeit
#define APP_OFF_SPEED		0x45E3	// Ausschaltgeschwindigkeit
#define APP_SEND_DIM_END	0x45E8	// Dimmwert nach Dimmende senden; Maske=0x0080

#define APP_CENT_OBJ_ACT	0x46E5	// Zentrale Objekte für Kanal A aktiv + Kanaloffset: 0x05; Maske=0x0040

#define APP_BLOCK1_1_VAL	0x45B4	// Helligkeit bei Sperrobjekt1 = 1
#define APP_BLOCK1_0_VAL	0x45B5	// Helligkeit bei Sperrobjekt1 = 0
#define APP_BLOCK1_BEHAV	0x45B6	// Verhalten bei Sperrobjekt1; Wert=1 : Maske=0x00F0; Wert=0 : Maske=0x000F ;; 0/aus; 2/keine Änderung; 3/Helligkeitswert
#define APP_BLOCK2_1_VAL	0x45B8	// Helligkeit bei Sperrobjekt2 = 1
#define APP_BLOCK2_0_VAL	0x45B9	// Helligkeit bei Sperrobjekt2 = 0
#define APP_BLOCK2_BEHAV	0x45BA	// Verhalten bei Sperrobjekt2; Wert=1 : Maske=0x00F0; Wert=0 : Maske=0x000F ;; 0/aus; 2/keine Änderung; 3/Helligkeitswert

// RGB(W)-Mode
//#define APP_SEND_DIM_VAL	0x45DA	// Dimmwert nach Änderung senden
//#define APP_REL_SPEED		0x45B0	// Geschwindigkeit relativ Dimmen
#define APP_RGB_ABS_SPEED					0x4716	// Geschwindigkeit absolut Dimmen
#define APP_RGB_BEAVIOUR_AFTER_RESET		0x46E4	// Helligkeit nach Einschalten off/0 ,color value/3 ,last value/4
#define APP_RGB_BEAVIOUR_AFTER_RESET_M		0xF0	// TODO noch prüfen
#define APP_RGB_BEAVIOUR_AFTER_RESET_R_Val	0x46E6  // color value
#define APP_RGB_BEAVIOUR_AFTER_RESET_G_Val	0x46EB  // color value
#define APP_RGB_BEAVIOUR_AFTER_RESET_B_Val	0x46F0  // color value
#define APP_RGB_SEND_RGB_STATUS				0x470F  // not active/0 ,at dim end/2 ,during dim process and at dim end/3 ,
#define APP_RGB_SEND_HSV_STATUS				0x4710	// not active/0 ,at dim end/2 ,during dim process and at dim end/3 ,
#define APP_RGB_SEND_3_BYTE_STATUS			0x4711  // not active/0 ,at dim end/2 ,
#define APP_RGB_SEND_STATUS_SEQ				0x48E2  // Output the status while sequence run
#define APP_RGB_WHITE_BALANCE				0x4761  // Weißabgleich not active/0; active/1

//#define APP_OFF_SPEED		0x45E3	// Ausschaltgeschwindigkeit
//#define APP_SEND_DIM_END	0x45E8	// Dimmwert nach Dimmende senden; Maske=0x0080

#endif /* COM_OBJS_H_ */
