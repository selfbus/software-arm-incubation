/*
 *  app_main.cpp - Application's main
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/platform.h>
#include <config.h>
#include <sblib/eib/sblib_default_objects.h>
#include <com_objs.h>
#include <AdcIsr.h>
#include <Relay.h>
#include <Appl.h>
#include <RelSpi.h>
#include <ManualCtrl.h>
#include <MemMapperMod.h>
#include <sblib/usr_callback.h>
#include <app_main.h>
#include <DebugFunc.h>
#include <string.h> /* for memcpy() */


// System time in milliseconds (from timer.cpp)
extern volatile unsigned int systemTime;

unsigned LastRelTime;
unsigned LastManCtrlTime;
bool ProcessingEnabled;
unsigned LastTimeFctTime;
unsigned OpStatesTime;
bool NextState = false;
AppOperatingStates AppOperatingState;
bool NoAppWasRunning = false;

// The last 2 Bytes of the Serial Number are now the Version number: 0x01 0x01 = V1.1
const byte SerialNumber[] = { 0x12, 0x34, 0x56, 0x00, 0x01, 0x01 };

/* Es gibt verschiedene Systemzustände, in denen Teile oder die ganze Funktionalität abgeschaltet ist:
 * - direkt nach dem Systemstart
 *   + keine Objektverarbeitung
 *   + eigentlich gar nix, keine manuelle Bedienung, keine Relaisansteuerung
 *   + Mindestwartezeit 1 Sekunde, in dem die Mindestbusspannung nie unterschritten werden darf
 * - "Applikation heruntergeladen"
 *   - Wartezeit vor Versenden von Objekten und vor Aufnahme der Relaisansteuerung (nach Start)
 *     + Laden des gespeicherten Systemzustands
 *     + Objektverarbeitung aktiv
 *     + jedoch noch kein Versenden von Objekten
 *     + keine Schaltaktionen und damit auch noch keine Handbedienung(!)
 *     + Beim Übergang zum Normalzustand Versenden von Statusobjekten
 *     + Verbleiben in diesem Zustand bis Mindestschaltenergie für das "Schaltaktionen-bei-Busspannungsausfall" gespeichert
 *       Das wird innerhalb der Relais-Unit überwacht, muss rückwärts in das Hauptprogramm übernommen werden.
 *   - Normalzustand
 *     + Objektverarbeitung aktiv
 *     + Versenden von Objekten aktiv
 *     + Relaisansteuerung aktiv
 *     + Handbedienung aktiv
 *   - Ausfall der Busspannung
 *     + Abspeichern des Systemzustandes
 *     + nix mehr, keine Objektverarbeitung, keine Schaltaktionen, keine Objekte mehr versenden
 *     + Übergang zu Systemstart
 * - "keine Applikation heruntergeladen"
 *     + Laden des Systemzustands, jedoch wird nur der Relaiszustand ausgewertet
 *     + Wartezeit, bis das Vermessen einer Schaltaktion beendet wurde, dann Handbedienung
 *
 */

/*
 * Der MemMapper bekommt einen 1kB Bereich ab 0xEA00, knapp unterhalb des UserMemory-Speicherbereichs ab 0xF000.
 * Damit lassen sich 3 Pages (je 256Byte) (und die allocTable die MemMappers) unterbringen.
 * Benötigt werden zwei Pages:
 * - für den Konfigurationsspeicher jenseits von 0x4B00. Für die SbLib endet der Konfigurationspeicher dort,
 *   unser Vorbild legt dort jedoch noch einige allgemeine Optionen ab.
 * - für die Systemzustände. Diese werden bei Busspannungsausfall und Neustart abgespeichert.
 */
MemMapperMod memMapper(0xea00, 0x400);

AppUsrCallback usrCallback;

/*
 * Initialize the application.
 */
void setup()
{
	for (unsigned ipno=0; ipno<8; ipno++)
 {
  // Allen Interrupts eine mittlere Priorität einräumen
  // (2, was "zweitniedrigst" bedeutet (Werte 3..0 möglich, klein höher).
  // Die Standardeinstellung mit 0 (höchste Prio für alle) hilft hier nicht weiter.
  // Die ADC-ISR bekommt später eine erhöhte Priorität.
  NVIC->IP[ipno]=0x80808080;
 }
// SystemCoreClockUpdate();
 adctimerSetup();
 analogSetup();
#ifdef SERIALCURRPRINTOUT
 // Aufgrund einer Pinüberschneidung (Rel-PWM ist RxD)
 // PWM danach initialisieren.
 SerialPrintSetup();
#endif
 pwmSetup();
 IsrSetup();
 LastTimeFctTime = LastRelTime = LastManCtrlTime = systemTime;
 AppOperatingState = AppOperatingStates::Startup;
 // //RelTestEnqueue();
 bcu.setProgPin(PIOPROGBTN);
 bcu.setProgPinInverted(true);
 // das Ding ist ein BIM112 "Maskenversion 0701"
 bcu.begin(MANUFACTURER, DEVICETYPE, APPVERSION);
 // _bcu und bcu sind das gleiche Objekt.
 // _bcu ist vom Typ BCU, während bcu vom Typ BcuBase ist.
 _bcu.setMemMapper((MemMapper *)&memMapper); // Der BCU wird hier der modifizierte MemMapper bekanntgemacht
 _bcu.setUsrCallback((UsrCallback *)&usrCallback);
 _bcu.enableGroupTelSend(false);
 memcpy(userEeprom.serial, SerialNumber, sizeof(SerialNumber));
 // 12 Bytes der Aktorkonfiguration werden ab 0x4B00 geschrieben. Das liegt bloederweise
 // genau jenseits des USER-EEPROM. Also mappen wir virtuellen Speicherbereich dorthin.
 memMapper.addRange(0x4b00, 0x100);
 memMapper.addRange(0x0, 0x100); // Zum Abspeichern/Laden des Systemzustands
 objectEndian(LITTLE_ENDIAN);
 userEeprom.commsTabAddr = 0x4400; // Diese Basisadresse wird nicht über die ETS runtergeschrieben, ist aber notwendig!
 setUserRamStart(0x3FC);
 appl.RecallAppData(RECALLAPPL_STARTUP);
 manuCtrl.StartManualCtrl();
#ifdef TS_ARM_2CH
 //Ausgänge für Relais
 pinMode(REL1ON, OUTPUT);
 pinMode(REL1OFF, OUTPUT);
 pinMode(REL2ON, OUTPUT);
 pinMode(REL2OFF, OUTPUT);
#endif
}

void AppOperatingStateMachine(unsigned referenceTime, bool AppValid)
{
 switch (AppOperatingState)
 {
 case AppOperatingStates::Startup:
  relay.Stop();
  OpStatesTime = referenceTime + STARTUPWAITTIME;
  AppOperatingState = AppOperatingStates::Stabilize;
  break;
 case AppOperatingStates::Stabilize:
  if (GetBusVoltage()<(int)MINUBUSVOLTAGERISING)
  {
   OpStatesTime = referenceTime + STARTUPWAITTIME;
  } else {
   if ((signed int)(referenceTime - OpStatesTime) > 0)
   {
    if (AppValid)
    {
     appl.StartupGlobSafetyStartTime(referenceTime);
     _bcu.setGroupTelRateLimit(appl.ReadTelRateLimit());
     appl.InitialChannelSwitch(referenceTime);
     OpStatesTime = referenceTime + ReadStartDelayObjSendAndSwitching()*1000; // Wartezeit nach Konfig
     AppOperatingState = AppOperatingStates::AppStartup;
    } else {
     AppOperatingState = AppOperatingStates::NoAppStartup;
    }
   }
  }
  break;
 case AppOperatingStates::AppStartup:
  if (AppValid)
  {
   if ((signed int)(referenceTime - OpStatesTime) > 0)
   {
    appl.StartupSafetyAndForcedPos(); // Versendet noch keine Statusobjekte, da noch nicht AppRunning
    AppOperatingState = AppOperatingStates::AppRunning;
    NoAppWasRunning = false;
    appl.ConfigRelayStart();
    appl.UpdateAllStatusObjects();
    _bcu.enableGroupTelSend(true); // Aktivieren des Gruppentelegrammversands
   }
  } else {
   // Obwohl das System mit "AppValid" gestartet wurde, ist nun das Flag auf einmal nicht mehr gesetzt -> Abbruch, Neustart
   relay.Stop();
   AppOperatingState = AppOperatingStates::AppShutdown;
  }
  break;
 case AppOperatingStates::AppRunning:
  if (AppValid)
  {
   if (not BusVoltageCheck())
   {
    // Die Busspannung ist nicht ausreichend, die Routine BusVoltageCheck hat in diesem Fall bereits die
    // Relay-Unit von dem Ausfall informiert.
    AppOperatingState = AppOperatingStates::StoreData;
    _bcu.enableGroupTelSend(false);
   }
  } else {
   // Obwohl das System mit "AppValid" gestartet wurde, ist nun das Flag auf einmal nicht mehr gesetzt -> Abbruch, Neustart
   // Dieser Fall kommt vor, wenn ein Download über die ETS angestartet wird.
   relay.Stop();
   AppOperatingState = AppOperatingStates::AppShutdown;
   _bcu.enableGroupTelSend(false);
  }
  break;
 case AppOperatingStates::AppShutdown:
  // Brownout wurde zwar schon festgestellt, es muss jedoch noch auf die Relais-Unit gewartet werden.
  if (not relay.IsOperating())
  {
   AppOperatingState = AppOperatingStates::Startup;
  }
  break;
 case AppOperatingStates::NoAppStartup:
  relay.Start(0, 0);
  AppOperatingState = AppOperatingStates::NoAppRunning;
  NoAppWasRunning = true;
  break;
 case AppOperatingStates::NoAppRunning:
  if (not AppValid)
  {
   if (not BusVoltageCheck())
   {
    // Die Busspannung ist nicht ausreichend, die Routine BusVoltageCheck hat in diesem Fall bereits die
    // Relay-Unit von dem Ausfall informiert.
    AppOperatingState = AppOperatingStates::StoreData;
    // Auch im NoApp-Fall muss der Systemzustand gespeichert werden, sonst ginge der aktuelle Relaiszustand verloren.
   }
  } else {
   // Obwohl das System mit "AppValid = false" gestartet wurde, ist nun das Flag auf einmal gesetzt -> Abbruch, Neustart
   // Dieser Fall kommt vor, wenn der Download über die ETS erfolgreich abgeschlossen worden ist.
   relay.Stop();
   AppOperatingState = AppOperatingStates::NoAppShutdown;
  }
  break;
 case AppOperatingStates::NoAppShutdown:
  // Brownout wurde zwar schon festgestellt, es muss jedoch noch auf die Relais-Unit gewartet werden.
  if (not relay.IsOperating())
  {
   AppOperatingState = AppOperatingStates::Startup;
  }
  break;
 case AppOperatingStates::StoreData:
  if (not relay.IsOperating())
  {
   appl.StoreApplData(STOREAPPL_BUSVFAIL);
   AppOperatingState = AppOperatingStates::Startup;
  }
  break;
 }
}

// Ist wahr, wenn das Gerät einen sicheren Betriebszustand mit ausreichend Energieversorgung erreicht hat.
// Wird im Grunde benutzt um festzustellen, ob die LEDs an oder zwecks Energiesparens aus sein sollten.
// (Bei Busspannungsausfall muss Energie für das Abspeichern des Systemzustands gespeichert werden.)
bool AppOrNoAppProcessingEnabled(void)
{
 return ((AppOperatingState == AppOperatingStates::AppStartup) ||
   (AppOperatingState == AppOperatingStates::AppRunning) ||
   (AppOperatingState == AppOperatingStates::NoAppStartup) ||
   (AppOperatingState == AppOperatingStates::NoAppRunning));
}

void RelayAndSpiProcessing(void)
{
 unsigned referenceTime = systemTime;
 unsigned SpiRelData;
 // TODO Passt diese Rasterung bei den Relais? Die Pulsdauer verlängert sich, wenn mal der exakte Zeitpunkt verpasst wurde.
 if ((referenceTime - LastRelTime) >= 5)
 {
  LastRelTime = referenceTime;
  relay.DoSwitching(LastRelTime, SpiRelData); // Erzeugt in SpiRelData die aktuellen Ansteuerdaten für die Relais
#ifndef TS_ARM_2CH
  relspi.ReadRx(); // Liest die Daten aus dem Empfangspuffer der Schnittstelle
  // Die Anzahl der gelesenen Bytes wird verworfen, die Daten nie abgerufen
  unsigned OneSpiByte;
  for (int cnt=SPIRELDRIVERBYTES-1; cnt >= 0; cnt--)
  {
   OneSpiByte = (SpiRelData >> (8*cnt)) & 0xff;
   relspi.SetTxData(&OneSpiByte, 1);  // Setzt die Relaistreiber
  }
  relspi.StartTransfer();            // Startet den Transfer (SetTxData geht in den FIFO des SPI-Interfaces)
#else
  //Relais direkt an Ausgangspins angeschlossen
  digitalWrite(REL1ON, (SpiRelData >> 0) & RELAYPATTERNON);
  digitalWrite(REL1OFF, (SpiRelData >> 0) & RELAYPATTERNOFF);
  digitalWrite(REL2ON, (SpiRelData >> 2) & RELAYPATTERNON);
  digitalWrite(REL2OFF, (SpiRelData >> 2) & RELAYPATTERNOFF);
#endif
 }
}

void LedProcessing(void)
{
 // Die ETS5.6 programmiert merkwürdigerweise eine ganz andere Adresse,
 // das muss korrigiert werden.
 if (userEeprom.commsTabAddr != 0x4400)
  userEeprom.commsTabAddr = 0x4400;

 unsigned OutputState;
 if (AppOrNoAppProcessingEnabled()) // LEDs nur dann, wenn kein Strom gespart werden muss
 {
  OutputState = relay.GetTrgState() & 0x3f;
  // Debugausgaben bei den beiden "oberen" LEDs
  if (bcu.applicationRunning()) //(AppOperatingState == AppOperatingStates::AppStartup)
   OutputState |= 0x40;
  if (AppOperatingState == AppOperatingStates::AppRunning)
   OutputState |= 0x80;
 } else {
  OutputState = 0;
 }
 manuCtrl.UpdateLeds(OutputState);
}

void MainProcessingRoutine(bool AppValid)
{
 unsigned referenceTime = systemTime; // In verschiedenen Bereichen der Routine ist es wichtig, dass von einer konsistenten Zeit ausgegangen wird

 // Die Verwaltung der Operating States der Applikation
 // ====================================================
 AppOperatingStateMachine(referenceTime, AppValid);

 // Objektverarbeitung
 //====================
 int objno;
 if (AppProcessingEnabled())
 {
  while ((objno = nextUpdatedObject()) >= 0)
  { // Empfangene Objekte verarbeiten
   appl.objectUpdated(objno, referenceTime);
  }
  relay.DoEnqueue(); // Erzeugt aus evtl. bei objectUpdates aufgelaufenen Schaltaufträgen den eingentlichen Eintrag in der Warteschlange
 } else {
  // Objektverarbeitung inaktiv
  while ((objno = nextUpdatedObject()) >= 0); // Empfangene Objekte aus der Warteschlange löschen
 }

 // Zeitverarbeitung
 //====================
 if (LastTimeFctTime != referenceTime)
 { // Aufruf im Millisekundenraster
  LastTimeFctTime = referenceTime;
  if (AppProcessingEnabled())
  {
   appl.TimeFunctionsTimeRelated(referenceTime);
   relay.DoEnqueue();
   appl.GlobalSafetyTimeRelated(referenceTime);
   relay.DoEnqueue();
  }
 }

 // Stromwertverarbeitung
 //====================
 if (AdcIsrNewDataAvail())
 {
  AdcIsrProcOffset();
#ifndef OMITCURRFCT
  AdcIsrCurrFilt();
#ifdef SERIALCURRPRINTOUT
  SerialPrintCurrents();
#endif
  if (AppProcessingEnabled())
  {
   appl.CurrentFunctions(referenceTime);
   relay.DoEnqueue();
  }
#endif
 }

 // Handbedienung
 //==============
 if ((referenceTime - LastManCtrlTime) >= 10)
 {
  LastManCtrlTime = referenceTime;
  unsigned ButtonDownEvents = manuCtrl.DoManualCtrl();
  if ((ButtonDownEvents != 0) && (AppManualOpWithObjEnabled() || AppManualOpWOObjEnabled()))
  {
   appl.ManualControl(ButtonDownEvents);
   relay.DoEnqueue();
  }
 }

 // Relais & SPI-Verarbeitung
 //==========================
 RelayAndSpiProcessing();

// LED Ausgabe
 //============
 LedProcessing();

 //if (bus.idle()) // alle 10µs kommt ein INT, also ist das Sleep hier eher theoretischer Natur.
   waitForInterrupt();
}

/*
 * Diese loop-Variante wird aufgerufen, wenn keine Applikation geladen ist.
 */
void loop_noapp()
{
 MainProcessingRoutine(false);
}

/*
 * Die Hauptverarbeitungsroutine wenn die Applikation gültig ist.
 */
void loop()
{
 MainProcessingRoutine(true);
}

/*
 * USR_CALLBACK_RESET
 * Wird aufgerufen direkt vor einem µC-Reset
 * - auf Relay warten, Relay sperren
 * - Runtime Daten abspeichern
 *   - es werden Defaultwerte abgespeichert (natürlich nicht für den Relaiszustand, der wird weitergeführt)
 * USR_CALLBACK_FLASH
 * Wird aufgerufen allgemein wenn Änderungen am User_Eeprom vorgenommen worden sind.
 * Konkrete Fälle: Download einer Applikation, aber auch Entladen einer Applikation. Unterscheiden der Fälle durch
 * AppRunning vorher/nachher
 * - auf Relay warten, Relay sperren
 * - Applikation auf Shutdown setzen (dadurch wird sie später geordnet neu gestartet)
 * - Runtime Daten abspeichern
 * USR_CALLBACK_BCU_END
 * Kommt eigentlich nicht vor, wird bislang nicht mal bei einem Busspannungsausfall aufgerufen.
 * Es können aber die Schritte wie bei den anderen Fällen ausgeführt werden.
 */
void AppUsrCallback::Notify(int type)
{
 if ((type == USR_CALLBACK_RESET) || (type == USR_CALLBACK_FLASH) || (type == USR_CALLBACK_BCU_END))
 {
  relay.Stop();
  while (relay.IsOperating())
  {
   RelayAndSpiProcessing();
  }
  if ((AppOperatingState == AppOperatingStates::AppStartup) || (AppOperatingState == AppOperatingStates::AppRunning))
  {
   AppOperatingState = AppOperatingStates::AppShutdown;
  }
  if ((AppOperatingState == AppOperatingStates::NoAppStartup) || (AppOperatingState == AppOperatingStates::NoAppRunning))
  {
   AppOperatingState = AppOperatingStates::NoAppShutdown;
  }
  bool ReasonAppDownload = NoAppWasRunning && bcu.applicationRunning();
  if ((type == USR_CALLBACK_FLASH) && (ReasonAppDownload))
   type = STOREAPPL_DOWNLOAD;
  // App Download -> die Funktionen "Nach Appl Download" ausführen
  // App Entladen -> Zustände beibehalten, für einen späteren Download, Kanal könnte ja unverändert sein
  // Download failure
  // Reset
  // Bus Voltage Fail mit BusVoltageRecovery (wird nicht hier behandelt)
  // Bei einem BusVolFail/Reset/Entladen muss der Zustand gespeichert werden, schließlich könnte nach dem letzten Start
  // noch die App aktiv gewesen sein. Außerdem gibt es die Handbedienung
  appl.StoreApplData(type);
 }
}
