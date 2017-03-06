/*
 *  app_main.h - Application's main
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

extern MemMapperMod memMapper;

enum class AppOperatingStates : byte
{
 Startup,
 Stabilize,        // Es wird gewartet, dass die Versorgungsspannung eine bestimmte Zeit stabil anliegt
 AppStartup,       // Anscheinend ist ein Applikation geladen, die muss korrekt initialisiert werden
 AppRunning,
 AppShutdown,
 NoAppStartup,     // Übergänge von App<->NoApp finden immer über (No)AppShutdown -> (No)AppStartup statt
 NoAppRunning,
 NoAppShutdown,
 StoreData,
};

extern AppOperatingStates AppOperatingState;

/*
 * Application processing enabled
 * Die Applikation verarbeitet eingehende Objekte, evtl Zeitverarbeitung, Sicherheitsfuktionen etc sind aktiv.
 * Jedoch ist hier noch nicht sichergestellt, ob die Applikation Objekte versenden darf. Dafür siehe AppObjSendEnabled().
 */
static inline bool AppProcessingEnabled(void)
{
 return ((AppOperatingState == AppOperatingStates::AppStartup) || (AppOperatingState == AppOperatingStates::AppRunning));
}

/*
 * Application Object Sendign enabled
 * Die Applikation darf Objekte versenden.
 * Dann ist gleichzeitig das Schalten der Relais freigeschaltet.
 */
static inline bool AppObjSendEnabled(void)
{
 return (AppOperatingState == AppOperatingStates::AppRunning);
}

/*
 * Application Manual Operation with Object Processing enabled
 * Die Handbedienung im Kontext der normalen Applikationsausführung ist aktiv.
 */
static inline bool AppManualOpWithObjEnabled(void)
{
 return (AppOperatingState == AppOperatingStates::AppRunning);
}

/*
 * Application Manual Operation without Object Processing enabled
 * Die Handbedienung ist freigegeben, jedoch wird die Applikation nicht ausgeführt
 * (und auch keine Objekte verarbeitet oder versendet).
 */
static inline bool AppManualOpWOObjEnabled(void)
{
 return (AppOperatingState == AppOperatingStates::NoAppRunning);
}

class AppUsrCallback: public UsrCallback {
public:
 virtual void Notify(int type);
};


#endif /* APP_MAIN_H_ */
