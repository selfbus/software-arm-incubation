/*
 * config.h
 *
 *  Created on: 27.01.2018
 *      Author: uwe223
 *
 *
 *
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <Appl.h>


#define MANUFACTURER 0x83		// MDT

typedef struct
{
    unsigned int noOfChannels;  //!> how many channels are supported with this hardware
    unsigned short deviceType;  //!> bcu.begin devicetype
    unsigned short appVersion;  //!> application version
    byte hardwareVersion[6];    //!> The hardware identification number (see selfbus wiki "Verwendung der sblib")
    const char *version;
} HardwareVersion;

extern const HardwareVersion * currentVersion;

const HardwareVersion hardwareVersion[3] =
{
{ 4, 0x003D, 0x14, { 0, 0, 0, 0, 0x02, 0x3D }, "AKD-0424V.01 RGBW LED Controller" },
{ 4, 0x003D, 0x22, { 0, 0, 0, 0, 0x03, 0x3D }, "AKD-0424V.02 RGBW LED Controller" },
{ 4, 0x001B, 0x13, { 0, 0, 0, 0, 0x00, 0x32 }, "AKD-0401.01 Dimmaktor REG 4fach" },
};

#ifdef LED4
#define HARDWARE_ID 0			// MDT AKD-0424V.01 RGBW LED Controller
#define PWM						// 4x PWM-Ausgang an PIO3_0, PIO3_1, PIO1_1, PIO1_2
#endif

#define TRIAC	PIO3_2			// Ausgang Triac
#define RELON	PIO2_3			// Ausgang bistabiles Relais einschalten
#define RELOFF	PIO1_5			// Ausgang bistabiles Relais ausschalten

/* Pulse Duration for the bistable Relais
 * ======================================
 * To get the max switching frequency, adjust to the used relay type -> see datasheet.
 * Otherwise use 50ms, the max value found for several relay types.
 *
 * 30ms Schrack RTX, Omron G5RL
 * 20ms Gruner 704L
 * 50ms HongFa HFE10 and HFE20
 * Unit of RELAYPULSEDURATION is milliseconds
 *
 * Pulslänge für die bistabilen Relais
 * ===================================
 * Um die max Umschaltfrequenz zu erreichen, muss der Wert an das verwendete Relais
 * angepasst werden -> siehe Datenblatt.
 * Andernfalls kann 50ms verwendet werden, der maximale Wert, der für verschiedene betrachtete Relais gilt.
 *
 * 30ms Schrack RTX, Omron G5RL
 * 20ms Gruner 704L
 * 50ms HongFa HFE10 and HFE20
 * Einheit von RELAYPULSEDURATION ist Millisekunden
*/
#define RELAYPULSEDURATION 50

//Verzögerungszeit bis das Relais abgeschaltet wird, nachdem alle Kanäle 0 sind in ms
#define RELAYOFFDELAY 2000

//Maximalwert bestimmt die Auflösung der Dimmstufen speziell für langsames auf-abdimmen, anstelle der von außen einstellbaren KNX-Dimmstufen 0-255
#define MAXOUTPUTVALUE 10000      //  entspricht Ausgang = 100%; bei Änderung DimmingCurves.cpp anpassen


#endif /* CONFIG_H_ */
