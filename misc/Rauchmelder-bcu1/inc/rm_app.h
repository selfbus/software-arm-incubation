/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Original written for LPC922:
 *  Copyright (c) 2015-2017 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef rm_app_h
#define rm_app_h

#include <sblib/ioports.h>



// OPTIONS
// Enable to activate device ID checking
#define DEVICE_ID_CHECK


// Current Firmware Version
#define FW_VER_MAJ  0x02    // Major
#define FW_VER_MIN  0x42    // Minor


// IO Pin assignments
#define RM_COMM_ENABLE	PIO3_5


/**
 * Den Zustand der Alarme bearbeiten.
 */
extern void process_alarm_stats();

/**
 * Com-Objekte bearbeiten deren Wert gesendet werden soll.
 */
extern void process_objs();

/**
 * Der Timer ist übergelaufen.
 */
//extern void timer_event();

/**
 * Alle Applikations-Parameter zurücksetzen.
 */
extern void initApplication(); //restart_app();

/**
 * Zähler für die Zeit die auf eine Antwort vom Rauchmelder gewartet wird.
 * Ist der Zähler 0 dann wird gerade auf keine Antwort gewartet.
 */
extern unsigned char answerWait;

/**
 * Anhand der Kommunikationsobjektnummer die passenden Daten herausgeben
 */
unsigned long read_obj_value(unsigned char objno);

/**
 * Empfangene Kommunikationsobjekte verarbeiten
 */
void objectUpdated(int objno);

#endif /*rm_app_h*/
