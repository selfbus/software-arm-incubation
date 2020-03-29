/*
 * config.h - This is the main configuration file for the firmware.
 *            Dies ist die Konfigurationsdatei für die Firmware.
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#include <sblib/core.h>
#include <sblib/eib.h>
/*
 * Project:
 * out-cs-bim112
 *
 *  Selfbus Output relay actuator - current sensing - BIM112
 * ==========================================================
 * This is the implementation of a current sensing relay actuator.
 * It has a full featured application with preset, scenes, threshold, logic, safety and time functions.
 * Due to a high resolution true RMS measurement even small loads can be accureately measured.
 *
 *  Selfbus Relais Aktor - strommessend - BIM112
 * ==============================================
 * Dies ist die Implementierung eines strommessenden Schaltaktors.
 * Er hat ein eine vollständige Applikation mit Preset, Szenen, Schwellwert, Logik und Zeitfunktionen.
 * Durch die hochauflösende Effektivwertmessung der Stroms können auch kleine Lasten genau gemessen werden.
 *
 * The configuration interface of this device is compatible to:
 * Das Konfigurationsinterface dieses Gerätes ist kompatibel zu:
 *
 *   SA/S x.16.6.1 series from ABB.
 *
 * x denotes the channel number and can be 2, 4, 8 or 12.
 * Use the application in version 3.2
 * Note that this implementation does not support the channel mode "Heating actuator".
 *
 * x steht für die Kanalzahl und kann 2, 4, 8 oder 12 sein.
 * Es sollte die Applikation in der Version 3.2 verwendet werden.
 * Bitte beachten: Der Kanalmodus "Heizungsaktor" wird nicht unterstützt.
 *
 * Important: The compiled firmware (Sblib + Application) must not exceed the size 0xEA00!
 * =========
 * As a result one or both of the Sblib or Application must be compiled with significant optimization.
 * (Tested with -O3)
 *
 * Wichtig: Die übersetzte Firmware (Sblib + Applikation) darf nicht größer als 0xEA00 werden!
 * =======
 * Das bedeutet, dass ein oder beide von Sblib und Applikation mit weitgehenden Optimierungen übersetzt werden müssen.
 * (Getestet mit -O3)
 */

/*Erweiterung für unterschiedliche Hardware mit/ohne Strommessung
 *===============================================================
 *
 *-DTS_ARM_2CH	2-Kanal Schaltaktor ohne Strommessung für Hardware TS-ARM 3.0.8 und 2out_16A_bi_TS-ARM_V1.0
 *				bistabile Relais direkt, also nicht über SPI angesteuert
 *				Konfiguration als ABB SA/S2.16.2.1
 *
 */


/*  Number of output channels of the actuator
 * ===========================================
 * Choose the number of channels for your actuator here.
 * Any number between 1 and 12 is allowed.
 *
 * The device will show up as an actuator with 2, 4, 8 or 12 channels. If the chosen CHANNELCNT does not
 * match any of these numbers the last channel(s) in the configuration tool ETS will be dummy channels
 * without any function.
 * For example if CHANNELCNT=6, only the first 6 channels in the ETS will be "real", the last 2 will be
 * function-less dummies.

 *  Anzahl der Ausgangskanäle des Aktors
 * ======================================
 * Dieser Parameter gibt die Anzahl der existierenden Relaisausgänge an. Jede
 * Anzahl zwischen 1 und 12 ist zulässig.
 *
 * Das Gerät wird als ein Aktor mit 2, 4, 8 oder 12 Kanälen auf dem Bus erscheinen. Wenn CHANNELCNT nicht einer
 * dieser Zahlen entspricht, werden der oder die letzten Kanäle in der ETS funktionslose "Dummies" sein.
 * Wird beispielsweise CHANNELCNT=6 gewählt, so sprechen die ersten 6 Kanäle in der ETS die realen Kanäle an,
 * die letzten zwei sind funktionslos.
 */
#ifndef TS_ARM_2CH
#define CHANNELCNT 6
#else
#define CHANNELCNT 2
#endif

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

//#define RELAYKEEPTASKSTOGETHER // If the switching tasks of one main loop cycle should be executed together
// Important if the stored energy is not sufficient for switching all relais at once.
// Legt fest, ob solche Schaltauftraege, die in einem Hauptschleifendurchlauf erstellt worden sind, auch
// zusammen ausgefuehrt werden sollen. Wichtig, wenn die gespeicherte Energie nicht zum Schalten aller
// Relais gleichzeitig ausreicht.

#define RELAYPATTERNON  2 // Bit pattern for switching ON
#define RELAYPATTERNOFF 1 // Bit pattern for switching OFF

/*
 * AD Converter pin configuration
 * AD Wandler Pinkonfiguration
 */
#define AdcCtrlTmr timer16_0   // A timer is needed to periodically start the ADC (16 or 32 bit).
// Für das periodische Starten des ADC wird ein eigener Timer benötigt (16 oder 32 bit).

#ifndef TS_ARM_2CH
#define CHANALOWRANGE 2        // ADC Kanal Strommessung kleiner Messbereich / ADC channel // DEBUG vorher 0
#define PIOANALOWRANGE PIO1_1  // Analog input, current measurement low range              // DEBUG        PIO0_11

#define CHANAHIGHRANGE 1       // ADC Kanal Strommessung großer Messbereich / ADC channel
#define PIOANAHIGHRANGE PIO1_0 // Analog input, current measurement high range

#define CHANARAILVOLT 3       // ADC Kanal Spannungsmessung Speicherkondensatoren / ADC channel // DEBUG  vorher 2
#define PIOANARAILVOLT PIO1_2 // Analog input, storage rail voltage                             // DEBUG   PIO1_1

#define CHANABUSVOLT 7        // ADC Kanal Spannungsmessung KNX Busspannung / ADC channel
#define PIOANABUSVOLT PIO1_11 // Analog input, KNX bus voltage
#else
#define CHANALOWRANGE 3        // keine Strommessung, unbenutzte Pins
#define PIOANALOWRANGE PIO1_2  //

#define CHANAHIGHRANGE 1       // keine Strommessung, unbenutzte Pins
#define PIOANAHIGHRANGE PIO1_0 //

#define CHANARAILVOLT 2       // ADC Kanal Spannungsmessung Speicherkondensatoren / ADC channel // DEBUG  vorher 2
#define PIOANARAILVOLT PIO1_1 // Analog input, storage rail voltage                             // DEBUG   PIO1_1

#define CHANABUSVOLT 7        // ADC Kanal Spannungsmessung KNX Busspannung / ADC channel
#define PIOANABUSVOLT PIO1_11 // Analog input, KNX bus voltage
#endif
/*
 * Multiplexer Control pin configuration
 * Multiplexer Ansteuerung Pinkonfiguration
 */
#ifndef TS_ARM_2CH
#define PIOMUXPORT 3
#define PIOMUX0 PIO3_0    // The analog MUX address select signals must start at bit 0 of the selected port
#define PIOMUX1 PIO3_1
#define PIOMUX2 PIO3_2    // Bit 2 can be omitted if only 4 channels are selected
//#define PIOMUXENA0 PIO2_3 // The MUX enable bits can be omitted if the channel number is <= 8
//#define PIOMUXENA1 PIO1_5
#else
//wird nicht benötigt, unbenutzte Pins
#define PIOMUXPORT 0
#define PIOMUX0 PIO0_0    //
#define PIOMUX1 PIO0_1
#define PIOMUX2 PIO0_2    //
#endif
/*
 * Relay Driver PWM control configuration
 * See digital_pin.h at the definition of OUTPUT_MATCH for valid combinations of pins and timers.
 *
 * Relaistreiber PWM-Ansteuerung Konfiguration
 * In digital_pin.h sind bei der Definition von OUTPUT_MATCH gültige Kombnationen von Pins und Timern beschrieben.
 */
#define RelPwmTmr    timer32_0 // Relay PWM timer (16 or 32 bit)     // DEBUG timer32_1
#define PIORELPWM    PIO1_6    // Relay PWM output pin               // DEBUG PIO1_2
// PIO1_6 is connected to CT32B0_MAT0, so it can be used as PWM output via counter MAT0.
// An different counter has to be assigned as period generator. Here MAT1 is selected.
// PIO1_6 ist verbunden mit CT32B0_MAT0, kann also als PWM Ausgang ueber den Zaehler MAT0 verwendet werden.
// Ein anderer Zähler muss die Funktion des Periodengenerators uebernehmen, hier frei gewaehlt: MAT1
#define RELPWMPRDCH  MAT1      // Relay PWM timer period channel     // DEBUG MAT0
#define RELPWMDCCH   MAT0      // Relay PWM timer duty cycle channel // DEBUG MAT1

#ifndef TS_ARM_2CH
/*
 * SPI-Interface
 */
#define SPI0               // Change to SPI1 if SPI-Interface 1 is needed
#define PIOSPISCK  PIO2_11
#define PIOSPIMOSI PIO0_9
#define PIOSPIMISO PIO0_8  // Can be omitted if there is nothing to read back from the SPI bus
//#define SPICSEMULATION     // Define this if the Chip Select/Slave Select pin is not the hardware SS pin
#define PIOSPICS   PIO0_2 // This pin can be the hardware Slave-Select pin or any other pin // DEBUG PIO1_10
#else
/*
 * SPI-Interface -- wird nicht benutzt, Relais direkt angeschlossen
 */
#define SPI0               // Change to SPI1 if SPI-Interface 1 is needed
#define PIOSPISCK  PIO2_11
#define PIOSPIMOSI PIO0_9
#define PIOSPIMISO PIO0_8  // Can be omitted if there is nothing to read back from the SPI bus
//#define SPICSEMULATION     // Define this if the Chip Select/Slave Select pin is not the hardware SS pin
#define PIOSPICS   PIO0_2 // This pin can be the hardware Slave-Select pin or any other pin // DEBUG PIO1_10

//Relais Ausgänge für   2out_16A_bi_TS-ARM
#define REL1ON		PIO2_2
#define REL1OFF		PIO0_9
#define REL2ON		PIO2_11
#define REL2OFF		PIO3_0
#endif

#ifndef TS_ARM_2CH
#define PIOPROGBTN PIO2_8  // Programming button for LPC-4TE-TOP  (PROG2  S10)
#else
#define PIOPROGBTN PIO2_0  // Programming button for TS-ARM
#endif

/*
 * Definitions for the manual control and display LEDs
 * The Spi-Control allows to connect these as shift registers. In this implementation, this is not used and
 * not supported - they are directly connected to the micro controller.
 * Definitionen fuer die Handbedienung und LED Anzeigen
 * Die SPI-Ansteuerung erlaubt es, diese als Schieberegister anzuschliessen. In dieser Implementierung wird
 * das nicht benoetigt und nicht unterstuetzt, sie sind direkt am Mikrocontroller angeschlossen.
 */
#ifndef TS_ARM_2CH
#define SPILEDBYTES 0      // Number of the LED driver bytes in the SPI chain
// Anzahl der LED-Steuerbytes in der SPI-Kette.
#define SPIBUTTONBYTES 0   // Number of the button readback bytes in the SPI chain
// Anzahl der Bytes mit Tasten-Informationen in der SPI-Kette.

// Pin definitions for the manual control
// Pin Definitionen fuer die Handbedienung
#define BUTTONLEDCNT  8 // last 2 LEDs used for status signaling
#define BUTTONLEDCH1  PIO0_11
#define BUTTONLEDCH2  PIO1_10
#define BUTTONLEDCH3  PIO3_4
#define BUTTONLEDCH4  PIO2_5
#define BUTTONLEDCH5  PIO2_4
#define BUTTONLEDCH6  PIO0_3
#define BUTTONLEDCH7  PIO3_5
#define BUTTONLEDCH8  PIO2_1
#define BUTTONLEDCOM  PIO2_3
#else
#define SPILEDBYTES 0      // Number of the LED driver bytes in the SPI chain
// Anzahl der LED-Steuerbytes in der SPI-Kette.
#define SPIBUTTONBYTES 0   // Number of the button readback bytes in the SPI chain
// Anzahl der Bytes mit Tasten-Informationen in der SPI-Kette.

// Pin definitions for the manual control
// Pin Definitionen fuer die Handbedienung
#define BUTTONLEDCNT  4 // last 2 LEDs used for status signaling
#define BUTTONLEDCH1  PIO1_10		// TS-ARM IO11
#define BUTTONLEDCH2  PIO0_11		// TS-ARM IO12
#define BUTTONLEDCH3  PIO3_4
#define BUTTONLEDCH4  PIO2_5
#define BUTTONLEDCOM  PIO0_8		// TS-ARM IO10
#endif

/* ---------------------------------------------------------------
 * Additional internal stuff, change only if you know what you do!
 * Weitere interne Konfigurationsoptionen, nur ändern wenn du weißt, was du tust!
 */

#define PIODBGISRFLAG PIO2_9 // IO4 Pin 10

//#define OMITCURRFCT // Omit current functions to free some flash mem (for debugging purposes only)
//#define SERIALCURRPRINTOUT // Current printout through the serial port (for debugging purposes only)

#define MANUFACTURER 2
#define APPVERSION 0x32

#if CHANNELCNT <= 2

#ifndef TS_ARM_2CH
// mit Strommessung
#define DEVICETYPE 0xA05B // SA/S2.16.6.1
#define SPIRELDRIVERBYTES 1
#define ADCCHANNELCNT 4
#else
//ohne Strommessung
#define DEVICETYPE 0xA080 // SA/S2.16.2.1
#define SPIRELDRIVERBYTES 1
#define ADCCHANNELCNT 4
#define OMITCURRFCT // Omit current functions to free some flash mem
#endif

#elif CHANNELCNT <= 4

#define DEVICETYPE 0xA05C // SA/S4.16.6.1
#define SPIRELDRIVERBYTES 1
#define ADCCHANNELCNT 4
// 10 Wandlungen je Loop, 50kHz Samplefreq

#elif CHANNELCNT <= 8

#define DEVICETYPE 0xA05D // SA/S8.16.6.1
#define SPIRELDRIVERBYTES 2
#define ADCCHANNELCNT 8
// 20 Wandlungen je Loop, 100kHz Samplefreq

#elif CHANNELCNT <= 12

#define DEVICETYPE 0xA05E // SA/S12.16.6.1
#define SPIRELDRIVERBYTES 3
#define ADCCHANNELCNT 8
// 30 Wandlungen je Loop, 96kHz Samplefreq

#endif

#ifdef SPI0
#define SPISSP LPC_SSP0
#endif
#ifdef SPI1
#define SPISSP LPC_SSP1
#endif

#define SPICHAINLEN (SPIRELDRIVERBYTES+SPILEDBYTES+SPIBUTTONBYTES)

#define STARTUPWAITTIME 1000 // Stabilisierungszeit in Millisekunden direkt nach Systemstart

#define STAIRCASEWARNBLINKTIME 700

#define REF_V 3.397 // Als Referenzspannung für die ADCs wird die Versorgungsspannung benutzt. Der beim Schaltregler eingestellte
                    // Wert ist deutlich höher als 3,3V, fast 3,4V. Wird das nicht eingerechnet, leidet die Genauigkeit
#ifndef TS_ARM_2CH
#define MAXURAIL (REF_V*13) // 44,16V Ein LSB sind dann ca 43mV; Spannungsteiler 36k/3k
#else
#define MAXURAIL (REF_V*10.1) // 34,31V Ein LSB sind dann ca 33,5mV; Spannungsteiler 91k/10k
#endif

#define MAXCURRLOWRANGE (REF_V/82*1500/2/19) // in Ampere
#define MAXCURRHIGHRANGE (REF_V/82*1500/2)

#define ADC12VOLTS (unsigned(12.0/MAXURAIL*1023+0.99)) // Der 12V ADC-Wert, aufgerundet
#define ADC12VOLTSQR (ADC12VOLTS*ADC12VOLTS)           // Der 12V ADC-Wert, aufgerundet

#ifndef TS_ARM_2CH
#define ADCRAILVOLTAGELOSS (unsigned(3.5/MAXURAIL*1023+0.5)) // Spannungsverlust durch die Konstantstromladeschaltung
#define MINURAILINITVOLTAGE (unsigned(15.0/MAXURAIL*1023+0.5)) // Spannung, bis zu der die Speicherkondensatoren aufgeladen sein müssen, bis die Initialisierung startet.
#define MINUBUSVOLTAGEFALLING (unsigned(14.0/MAXURAIL*1023+0.5)) // Busspannung, unterhalb der jeder Betrieb eingestellt wird.
#define MINUBUSVOLTAGERISING (unsigned(18.0/MAXURAIL*1023+0.5))  // Busspannung, oberhalb der der Aktor gestartet wird.
#else
// 24V Relais
#define ADCRAILVOLTAGELOSS (unsigned(0.5/MAXURAIL*1023+0.5)) // Spannungsverlust durch die Konstantstromladeschaltung
#define MINURAILINITVOLTAGE (unsigned(17.0/MAXURAIL*1023+0.5)) // Spannung, bis zu der die Speicherkondensatoren aufgeladen sein müssen, bis die Initialisierung startet.
#define MINUBUSVOLTAGEFALLING (unsigned(17.0/MAXURAIL*1023+0.5)) // Busspannung, unterhalb der jeder Betrieb eingestellt wird.
#define MINUBUSVOLTAGERISING (unsigned(17.5/MAXURAIL*1023+0.5))  // Busspannung, oberhalb der der Aktor gestartet wird.
#endif

// Der Idle-Detector der Relay-Unit detektiert Idle als: Wenn die Ladekondensatoren innerhalb einer Zeit weniger als
// eine bestimmte Spannung geladen werden, dann sind die Elkos wohl voll und nichts los.
#define IDLEDETECTTIME  2000
#define IDLEDETECTVOLT  (signed(2.0/MAXURAIL*1023+0.5))

#define RELAYREPPULSEDELAY     10000 // 10 Sekunden nach dem Schaltpuls wird noch mal einer nachgelegt
//#define RELAYREPPULSEDELAYLONG 20000 // zu Debugzwecken
#define RELAYREPPULSEDELAYLONG (unsigned(10*24*3600*1000)) // Im Grunde nur sehr, sehr lang

#define RELPWMFREQ 40000  // Frequenz der PWM der Relaisansteuerung. Muss noch empirisch angepasst werden.
#define COREFREQ 48000000 // Es gibt mit SystemCoreClock zwar eine Systemvariable mit der allgemeinen Taktfrequenz,
                          // benötigt wird aber eine Konstante, die zur Compile-Zeit bekannt ist.
#define RELPWMPRD (COREFREQ / RELPWMFREQ)
#define RELPWMDC50 (unsigned(0.5*RELPWMPRD)) // Für Debugzwecke ist der Dutycycle bei Systemstart 0,5

#endif /* CONFIG_H_ */
