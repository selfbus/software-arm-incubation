/*
 *  Appl.cpp - Application logic
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/platform.h>
#include <config.h>
#include <com_objs.h>
#include <appl.h>
#include <relay.h>
#include <MemMapperMod.h>
#include <app_main.h>
#include <AdcIsr.h>
#include <crc8.h>

// System time in milliseconds (from timer.cpp)
extern volatile unsigned int systemTime;

Appl appl;

inline byte ReadChConfigByte(int chno, int confaddr)
{
 byte retval = userEeprom[APP_STARTADDR+APP_CHOFFS*chno+confaddr];
 return retval;
}

inline unsigned short ReadChConfigUInt16(int chno, int confaddr)
{
 //return userEeprom.getUInt16(APP_STARTADDR+APP_CHOFFS*chno+confaddr); Beim ABB ist so etwas Little Endian!
 return (unsigned short)ReadChConfigByte(chno, confaddr) + ((unsigned short)ReadChConfigByte(chno, confaddr+1) << 8);
}

/* Leider liegen die nicht-kanalabhängigen Konfigurationen bei diesem Aktor
 * jenseits des als userEeprom gemappten Speicherbereichs. Um Änderungen
 * in user_memory.cpp/.h zu vermeiden, wird das Problem hier mit einem
 * Memory-Mapper gelöst. Zugriffe, die ins "Jenseits" gehen würden,
 * werden auf memMapper umgeleitet.
*/
byte ReadConfigByte(int confaddr)
{
 if (confaddr >= (USER_EEPROM_END-APP_STARTADDR))
 {
  return memMapper.getUInt8(APP_STARTADDR+confaddr);
 } else {
  return userEeprom.getUInt8(APP_STARTADDR+confaddr);
 }
}

unsigned short ReadConfigUInt16(int confaddr)
{
 byte retval_low = ReadConfigByte(confaddr);
 byte retval_high = ReadConfigByte(confaddr+1) << 8;
 return (unsigned short)retval_low + (unsigned short)retval_high;
// if (confaddr >= (USER_EEPROM_END-APP_STARTADDR))
// {
//  return memMapper.getUInt16(APP_STARTADDR+confaddr);
// } else {
//  return userEeprom.getUInt16(APP_STARTADDR+confaddr);
// }
}

unsigned ReadStartDelayObjSendAndSwitching(void)
{
 return ReadConfigByte(APP_SENDSWDELAYPO_O);
}

inline void ChObjectUpdate(int chno, unsigned int objofs, unsigned int value)
{
 objectUpdate(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+objofs, value);
}

inline void ChObjectWrite(int chno, unsigned int objofs, unsigned int value)
{
 objectWrite(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+objofs, value);
}

inline unsigned int ChObjectRead(int chno, unsigned int objofs)
{
 return objectRead(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+objofs);
}

//unsigned int Appl::ChObjectRead(int chno, int objno)
//{
// return objectRead(objno + chno*SPACINGCHANNELOBJECTS + OFSCHANNELOBJECTS);
//}

/*
 * Aktualisiert das Treppenlichtwarnobjekt bei Beginn und Ende der Warnzeit.
 * Wenn das Versenden von Objekten freigeschaltet ist, wird das Objekt auch versendet.
 */
inline void StairSendWarnObject(int chno, bool WarningActive)
{
 ChObjectWrite(chno, OBJ_WARNSTAIRL, WarningActive ? 1 : 0);
}

void Appl::ConfigRelayStart(void)
{
 unsigned short BusVFailureMask=0, BusVFailureData=0;
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   switch ((ReadChConfigByte(chno, APP_REACTPFAIL_O) & APP_REACTPFAIL_M))
   { // Was tun bei Busspannungsausfall?
   case 0: // Ausschalten
    BusVFailureMask |= 1 << chno;
    if (ReadChConfigByte(chno, APP_SW_NONC_O) & APP_SW_NONC_M)
    { // Evtl Ausgangsinvertierung muss hier bereits eingearbeitet werden
     BusVFailureData |= 1 << chno;
    }
    break;
   case 1: // Einschalten
    BusVFailureMask |= 1 << chno;
    if ((ReadChConfigByte(chno, APP_SW_NONC_O) & APP_SW_NONC_M) == 0)
    { // Evtl Ausgangsinvertierung muss hier bereits eingearbeitet werden
     BusVFailureData |= 1 << chno;
    }
    break;
   default:
    break;
   }
  }
 }
 relay.Start(BusVFailureMask, BusVFailureData);
}

Appl::Appl(void)
{
 ActuatorSafety = 0;
 RestartSkipBvrMask = 0;
}

void Appl::StartupSafetyAndForcedPos(void)
{
 /* Diese Routine kann nach dem Start die gespeicherten Safety-Zustände wiederherstellen.
  * Ausgangslage: ChannelStates[chno].Safety und ChannelStates[chno].ForcedPos sind initialisiert (bzw rekonstruiert)
  * ActuatorSafety ist initialisiert (bzw rekonstruiert).
  * Beim ABB ist die geräteweite Safety nach einem Neustart immer in "freigeschaltet", also inaktiv.
  * Das vereinfacht das Ganze
  * Zwangsstellung:
  * Wenn vor Neustart aktiv und nach Neustart nicht, überprüfung auf Ende aller Sicherheitsfkt
  * Wenn vor Neustart aktiv und nach Neustart aktiv, Wiederherstellen der Zwangsposition
  * Wenn vor Neustart nicht aktiv und nach Neustart aktiv, Zwangsposition durchführen
  */
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   if ((ReadChConfigByte(chno, APP_ENAFUNCSAFETY_O) & APP_ENAFUNCSAFETY_M) && ((RestartSkipBvrMask & (1 << chno)) == 0))
    // Kanalweise Safety Funktionalität aktiviert & Bei diesem Kanal soll nicht die Neuinitialisierung übersprungen werden?
   {
    TStateAndTrigger trigger = {false, false, false, false};
    int value = ReadChConfigByte(chno, APP_SW_FORCEDOVBVR_O); // Der Wert kann direkt als Objektwert verwendet werden.
    unsigned SafetyChanges = ChannelStates[chno].Safety;
    // Als SafetyChanges wird der gespeicherte Wert von Channel.Safety genutzt.
    // Damit werden evtl. Sicherheitsfunktionen wiederhergestellt und auch ein Übergang zu
    // "letzte Sicherheitsfunktion beendet" erkannt.
    // Das 0x100 teilt ProcessChSafety mit, den value << 12 als Objektwert zu betrachten.
    ProcessChSafety(trigger, SafetyChanges, OBJ_FORCEDOP | 0x100 | (value << 12), chno);
    UpdateStatusObjekt(chno, trigger);
    ChannelTrigger2RelaySwitch(chno, trigger); // Berücksichtigt evtl Invertierung
   }
  }
 }
 RestartSkipBvrMask = 0;
}

void Appl::InitialChannelSwitch(unsigned referenceTime)
{
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  TStateAndTrigger trigger = {false, false, false, false};
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   if  ((RestartSkipBvrMask & (1 << chno)) == 0)
   {
    // Bei diesem Kanal soll nicht die Neuinitialisierung übersprungen werden.
    // (Wäre der Fall bei Download ohne Änderungen in diesem Kanal)
    byte Conf = ((ReadChConfigByte(chno, APP_SW_ABVR_O) & APP_SW_ABVR_M ) >> APP_SW_ABVR_B);
    if (Conf < 2)
    {
     trigger.Sw = true;
     trigger.SwOnOff = (Conf != 0);
     PostProcessSingleSwitchObject(chno, trigger, -1, referenceTime);
    }
   }
  }
 }
}

int Appl::ReadTelRateLimit(void)
{
 return ReadConfigByte(APP_TELRATELIMIT_O);
}

//void Appl::ApplInit(unsigned referenceTime)
void Appl::StartupGlobSafetyStartTime(unsigned referenceTime)
{
 unsigned SafetyChanges = 0;
 // Evtl deaktivierte Sicherheiten löschen und weiter unten die Deaktivierung an die Kanäle weitergeben
 if ((ReadConfigByte(APP_SAFPRIO1FKT_O) > 1) && (ActuatorSafety & 1))
 {
  ActuatorSafety &= 6;
  SafetyChanges |= 1;
 }
 if ((ReadConfigByte(APP_SAFPRIO2FKT_O) > 1) && (ActuatorSafety & 2))
 {
  ActuatorSafety &= 5;
  SafetyChanges |= 1;
 }
 if ((ReadConfigByte(APP_SAFPRIO3FKT_O) > 1) && (ActuatorSafety & 4))
 {
  ActuatorSafety &= 3;
  SafetyChanges |= 1;
 }
 if ((ActuatorSafety & 1) == 0)
  ActuatorSafetyTripTime[0] = referenceTime + (unsigned)ReadConfigUInt16(APP_SAFPRIO1TIM_O)*1000;
 if ((ActuatorSafety & 2) == 0)
  ActuatorSafetyTripTime[1] = referenceTime + (unsigned)ReadConfigUInt16(APP_SAFPRIO2TIM_O)*1000;
 if ((ActuatorSafety & 4) == 0)
  ActuatorSafetyTripTime[2] = referenceTime + (unsigned)ReadConfigUInt16(APP_SAFPRIO3TIM_O)*1000;
 AliveTargetTime = referenceTime-65536000; // Damit das Telegramm quasi sofort nach Start gesendet wird
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {

   // Die Zeiten in der Strommessfunktion werden in "Strommessperioden" gerechnet,
   // ermöglicht kleinere Zählerbreiten.
   // Dort werden sie in jeder Periode dekrementiert, bei 0 ist also eine Aktion fällig.
   // Bei Initialisierung werden in alle Zähler pauschal "2 Sekunden" geschrieben, damit
   // noch keine Auswertungen vorgenommen werden, bevor gültige Werte vorliegen.
   // Auch ohne diesen Kniff würden noch keine Telegramme verschickt werden, aufgrund der
   // konfigurierbaren Startzeit des Geräts.
   ChannelStates[chno].CFStatusTime = RMSCURRENTVALUESPERSECOND*2;
   ChannelStates[chno].CFContCloseBlanking = RMSCURRENTVALUESPERSECOND*2;
   ChannelStates[chno].CFContFailBlanking = RMSCURRENTVALUESPERSECOND*2;
   ChannelStates[chno].CFLastSentValue = -100;
   ChannelStates[chno].CurrFctStates = 0;
  } else {
   // Heizungsaktor
   // vorerst KEINE FUNKTIONALITÄT!
  }
 }
 if (SafetyChanges)
 {
  ProcessSafetyChanges(SafetyChanges);
 }

}

/*
 * ModifyAfterDownload
 * Funktion, die auf die ChannelStates eines Kanals angewendet wird, bevor diese in das Flash abgespeichert werden.
 * Einerseits gibt es Konfigurationsoptionen, die dann verschiedene Zustände beeinflussen.
 * Andererseits sollen bei einem umkonfigurierten Kanal die Initialzustände aller Optionen (bis auf Relaiszustand) wieder
 * hergestellt werden. Dies wird bereits in StoreApplData vorgenommen.
 * In der Praxis bedeutet dies, dass ein Kanal, der nicht geändert wurde, ähnlich wie bei einem Busspannungsausfall,
 * anschließend einfach "weiterläuft", mit geringsten Auswirkungen des Downloadvorgangs.
 */
void Appl::ModifyChStateAfterDownload(int chno)
{
 if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  if ((ReadChConfigByte(chno, APP_INIAFTERDNL_O) & APP_INIAFTERDNL_M) == 1) // Es die Initialierung nach Download gewählt
  {
   ChannelStates[chno].Preset =
     ReadChConfigByte(chno, APP_SW_PRESET1VAL0_O) | (ReadChConfigByte(chno, APP_SW_PRESET2VAL1_O) << 4);
   ChObjectUpdate(chno, OBJ_CHGTHRESH1, ReadChConfigUInt16(chno, APP_SW_THCONST1_O)); // Laut ETS "value on bus voltage recovery", oder soll der Wert PowerCycles überleben?
   unsigned cnf;
   // Es können zwar nur 5 Szenen vorkonfiguriert werden
   ChannelStates[chno].ScenesVal = 0;
   for (unsigned scenememno=0; scenememno<5; scenememno++)
   {
    cnf = ReadChConfigByte(chno, APP_SW_SCEPRESET1_O+scenememno);
    if ((cnf & APP_SW_SCENEUNUSED_M) == 0)
     if (cnf & APP_SW_SCEPRESET1_M)
      ChannelStates[chno].ScenesVal |= 1 << scenememno;
   }
  }
}

/*
 * ModifyBeforeReset
 * Funktion, die auf die ChannelStates eines Kanals angewendet wird, bevor diese in das Flash abgespeichert werden.
 * Ein Reset stellt jetzt den Initialzustand wieder her.
 */
void Appl::ModifyChStateBeforeReset(int chno)
{
 SetIniChannelState(chno);
}

/*
 * ModifyAfterBusVoltageRecovery
 * Funktion, die auf die ChannelStates eines Kanals angewendet wird, nachdem diese bei Systemstart aus dem Flash geladen worden sind.
 * Je nach gewählter Konfiguration werden verschiedene Zustände beeinflusst.
 */
void Appl::ModifyChStateAfterBusVoltageRecovery(int chno)
{
 if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
 {
  ChObjectUpdate(chno, OBJ_THRESHOLD, ReadChConfigUInt16(chno, APP_SW_THVABVR_O)); // Wert des Threshold Objekts.
  ChannelStates[chno].LogicObjVals = 0;
  if (ReadChConfigByte(chno, APP_SW_LOG1OVLCABV_O) & APP_SW_LOG1OVLCABV_M)
   ChannelStates[chno].LogicObjVals |= 1;
  if (ReadChConfigByte(chno, APP_SW_LOG2OVLCABV_O) & APP_SW_LOG2OVLCABV_M)
   ChannelStates[chno].LogicObjVals |= 4;
  ChannelStates[chno].IntSwitchStates &= ~STDISTIMEFCT_M;
  if ((ReadChConfigByte(chno, APP_SW_VODTFABVR_O) & APP_SW_VODTFABVR_M) != 0) // Value object disable time function after bus voltage recovery
   ChannelStates[chno].IntSwitchStates |= STDISTIMEFCT_M;
 }
}

// Manche Telegrammtypen müssen noch angepasst werden. Obwohl die ETS die Objektkonfiguration
// ebenfalls überträgt, so ist dies nur die konstante Defaultkonfiguration.
//   if ((ReadChConfigByte(chno, APP_TYPCURRMEAS_O) & APP_TYPCURRMEAS_M) == 5) // Datentyp Strommesswert
//   { // 4 Byte Float com_objects.h
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_CURRENT, FLOAT);
//   } else { // 2 Byte Counter
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_CURRENT, BYTE_2);
//   }
//   if ((ReadChConfigByte(chno, APP_SW_THTYPE_O) & APP_SW_THTYPE_M) == 3) // Datentyp Threshold
//   { // 2 Byte Wert
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_THRESHOLD, BYTE_2);
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_CHGTHRESH1, BYTE_2);
//   } else { // 1 Byte Wert
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_THRESHOLD, BYTE_1);
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_CHGTHRESH1, BYTE_1);
//   }
//   if ((ReadChConfigByte(chno, APP_SW_FORCEDPOS_O) & APP_SW_FORCEDPOS_M) == 7) // Datentyp Zwangsstellungsobjekt
//   { // 2 Bit Wert
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_FORCEDOP, BIT_2);
//   } else { // 1 Bit Wert
//    objectSetType(OFSCHANNELOBJECTS + chno*SPACINGCHANNELOBJECTS + OBJ_FORCEDOP, BIT_1);
//   }
// Für die endgültige Implementierung gibt es die Möglichkeit, dass selbst ein Neudownload die mittels Objekt
// gespeicherte Schaltschwelle 1 unangetastet lässt. Dabei muss der Sonderfall berücksichtigt werden, das der
// Objekttyp geändert wurde. Zumindest Überprüfung auf einen ungültigen, abgespeicherten Wert. (Sättigen?)
// ReadChConfigByte(chno, APP_SW_THTYPE_O)
//   ChannelStates[chno].OldTreshVal = ReadChConfigUInt16(chno, APP_SW_THVABVR_O); // Wert des Threshold Objekts.

void Appl::SetIniChannelState(int chno)
{
 if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
 {
  ChannelStates[chno].IntSwitchStates = STPREPRESET2_M; // Die IntSwitchStates löschen
  ChannelStates[chno].Preset =
    ReadChConfigByte(chno, APP_SW_PRESET1VAL0_O) | (ReadChConfigByte(chno, APP_SW_PRESET2VAL1_O) << 4);
  ChObjectUpdate(chno, OBJ_CHGTHRESH1, ReadChConfigUInt16(chno, APP_SW_THCONST1_O)); // Laut ETS "value on bus voltage recovery", oder soll der Wert PowerCycles überleben?
  // Der Threshold1-Wert wird direkt im Objekt gespeichert und bei Bedarf auch aus dem Objekt wieder gelesen.
  ChObjectUpdate(chno, OBJ_THRESHOLD, ReadChConfigUInt16(chno, APP_SW_THVABVR_O)); // Wert des Threshold Objekts.
  // Der Threshold-Wert wird direkt im Objekt gespeichert und bei Bedarf auch aus dem Objekt wieder gelesen.
  ChannelStates[chno].OldThreshState = IniValueThresholdState(chno);
  ChannelStates[chno].LogicObjVals = 0;
  if (ReadChConfigByte(chno, APP_SW_LOG1OVLCABV_O) & APP_SW_LOG1OVLCABV_M)
   ChannelStates[chno].LogicObjVals |= 1;
  if (ReadChConfigByte(chno, APP_SW_LOG2OVLCABV_O) & APP_SW_LOG2OVLCABV_M)
   ChannelStates[chno].LogicObjVals |= 4;
  if ((ReadChConfigByte(chno, APP_SW_VODTFABVR_O) & APP_SW_VODTFABVR_M) != 0) // Value object disable time function after bus voltage recovery
   ChannelStates[chno].IntSwitchStates |= STDISTIMEFCT_M;
  unsigned int DurationStaircase = ReadChConfigUInt16(chno, APP_SW_TIMDURATION_O)*60 + ReadChConfigByte(chno, APP_SW_TIMDURASEC_O);
  // Die konfigurierte Zeit wird direkt im Objekt gespeichert
  ChObjectUpdate(chno, OBJ_TIMDURATION, DurationStaircase);
  ChannelStates[chno].TFState = TimeFctStates::Idle;
  ChannelStates[chno].Safety = 0;
  ChannelStates[chno].ForcedPos = 0;
  if (ReadChConfigByte(chno, APP_ENAFUNCSAFETY_O) & APP_ENAFUNCSAFETY_M)
  {
   if (ReadChConfigByte(chno, APP_SW_FORCEDPR1_O) < 3)
    ChannelStates[chno].Safety |= 0x10;
   if (ReadChConfigByte(chno, APP_SW_FORCEDPR2_O) < 3)
    ChannelStates[chno].Safety |= 0x20;
   if (ReadChConfigByte(chno, APP_SW_FORCEDPR3_O) < 3)
    ChannelStates[chno].Safety |= 0x40;
   unsigned zw = ReadChConfigByte(chno, APP_SW_FORCEDPOS_O);
   if ((zw < 3) || (zw == 7))
    ChannelStates[chno].Safety |= 0x80;
   if (zw == 1)
    ChannelStates[chno].ForcedPos = 1;
   if (zw == 2)
    ChannelStates[chno].ForcedPos = 2;
  }
  unsigned cnf;
  // Es können zwar nur 5 Szenen vorkonfiguriert werden
  ChannelStates[chno].ScenesVal = 0;
  for (unsigned scenememno=0; scenememno<5; scenememno++)
  {
   cnf = ReadChConfigByte(chno, APP_SW_SCEPRESET1_O+scenememno);
   if ((cnf & APP_SW_SCENEUNUSED_M) == 0)
    if (cnf & APP_SW_SCEPRESET1_M)
     ChannelStates[chno].ScenesVal |= 1 << scenememno;
  }
 }
}
/*
byte IntSwitchStates; // 7bit, gespeicherte Schaltzustände verschiedener Funktionsblöcke. Enhalten sind:
// PrePreset2State, 2bit
// SwitchObject, 1bit
// PreBlinkState, 1bit
// ChannelStateAfterTimeFunction, 1bit
// DisableTimeFunction, 1bit
// PermanentOn, 1bit
byte Preset; // unteres Nibble: Preset1, oberes Nibble: Preset2
byte BlinkDnCnt; // 1 Byte
byte ScenesVal; // Werte für die 5 möglichen Szenen, d.h. 5 bit belegt
//==> unsigned short Threshold1; // Da dieser Wert über den Bus geändert werden kann, wird er hier gespeichert
//==> unsigned short OldTreshVal; // Der Objektwert Threshold wird direkt aus diesem gelesen, eine Ablage hier ist unnötig
byte OldThreshState; // 3 bit belegt: 0: noch keine Thresholdüberschreitung festgestellt, 1: oberer Threshold, 2: unterer Threshold
byte LogicObjVals; // 4 bit belegt
// Bit 0 steht für den Objektwert Logik 1, Bit 1 für eine aktuelle Änderung Logik 1,
// Bit 2 für den Objektwert Logik 2, Bit 3 für eine Änderung Logik 2
byte Safety; // 8 bit belegt
// Bit 7..4: Maske der freigeschalteten Sicherheitsoptionen für diesen Kanal
// Bit 3..0: Aktueller Aktivierungsstand der Sicherheitsoptionen, soweit für diesen Kanal freigeschaltet
byte ForcedPos; // 2 bit belegt
// Bit 1: Bei 1 Schalthandlungen blockieren, ansonsten: Bit 0: Zielschaltzustand
// Wenn Zielzustand variabel, wird dieses Bit auch zwischendurch aktualisiert
TimeFctStates TFState; // 1 Byte
unsigned int TFTargetTime; // Zielzeit der nächsten Aktion bei einer Zeitfunktion
//==> unsigned short DurationStaircase; // Die Zeitdauer der Treppenlichtfunktion in Sekunden (wird im Kommunikationsobjekt selbst gespeichert)
*/

void Appl::StoreChannelState(int chno, byte* ptr, unsigned referenceTime)
{
 *ptr++ = ChannelStates[chno].IntSwitchStates;
 *ptr++ = ChannelStates[chno].Preset;
 *ptr++ = ChannelStates[chno].BlinkDnCnt;
 *ptr++ = ChannelStates[chno].ScenesVal;
 PtrWrUint16(ptr, ChObjectRead(chno, OBJ_THRESHOLD)); // Etwas komplizierter, da die Adressen unaligned sein können
 ptr+=2;
 PtrWrUint16(ptr, ChObjectRead(chno, OBJ_CHGTHRESH1));
 ptr+=2;
 *ptr++ = ChannelStates[chno].OldThreshState;
 //Folgende Werte würden bei BusVoltageRecovery eh überschrieben
 //*ptr++ = ChannelStates[chno].LogicObjVals;
 //*ptr++ = ChannelStates[chno].Safety;
 //*ptr++ = ChannelStates[chno].ForcedPos;
 *ptr++ = (byte)ChannelStates[chno].TFState;
 PtrWrUint32(ptr, ChannelStates[chno].TFTargetTime - referenceTime);
 ptr+=4;
 PtrWrUint16(ptr, ChObjectRead(chno, OBJ_TIMDURATION));
 ptr+=2;
} // 16 Byte

void Appl::RecallChannelState(int chno, byte* ptr, unsigned referenceTime)
{
 unsigned data;
 data = *ptr++; // IntSwitchStates
 data |= STPREPRESET2_M; // kein PrePreset, der wird immer zurückgesetzt
 ChannelStates[chno].IntSwitchStates = data;
 data = *ptr++; // Preset
 // Könnte man gegenprüfen, jedoch wird das beim Lesen aus dem Applikationsspeicher auch nicht gemacht.
 // Das Programm würde falsche Werte auch tolerieren.
 //if ((((data & 0xf) == 0) || ((data & 0xf) == 1) || ((data & 0xf) == 4) || ((data & 0xf) == 6)) && (((data & 0xf0) == 0) || ((data & 0xf0) == 0x10)))
 ChannelStates[chno].Preset = data;
 data = *ptr++; // BlinkDownCount
 ChannelStates[chno].BlinkDnCnt = data;
 data = *ptr++; // ScenesVal
 //if ((data & 0xe0) == 0)
 ChannelStates[chno].ScenesVal = data;
 data = PtrRdUint16(ptr); // Threshold - data value
 ptr+=2;
 if ((data < 0x100) || (((ReadChConfigByte(chno, APP_SW_THTYPE_O) >> APP_SW_THTYPE_B) & APP_SW_THTYPE_M) == 3))
  ChObjectWrite(chno, OBJ_THRESHOLD, data);
 data = PtrRdUint16(ptr); // Threshold1 value
 ptr+=2;
 if ((data < 0x100) || (((ReadChConfigByte(chno, APP_SW_THTYPE_O) >> APP_SW_THTYPE_B) & APP_SW_THTYPE_M) == 3))
  ChObjectWrite(chno, OBJ_CHGTHRESH1, data);
 data = *ptr++; // OldThreshState
 if (data < 3)
  ChannelStates[chno].OldThreshState = data;
 TimeFctStates Tfs = (TimeFctStates)*ptr++; // Time Function state
 switch ((ReadChConfigByte(chno, APP_SW_TIMEFUNC_O) & APP_SW_TIMEFUNC_M) >> APP_SW_TIMEFUNC_B)
 {
 case 1: // Treppenlicht
  if ((Tfs < TimeFctStates::StairOn) || (Tfs > TimeFctStates::StairBack2On))
   Tfs = TimeFctStates::Idle;
  break;
 case 2: // Ein-/Ausverzögerung
  if ((Tfs != TimeFctStates::DelayOff) && (Tfs != TimeFctStates::DelayOn))
   Tfs = TimeFctStates::Idle;
  break;
 case 3: // Blinken
  if ((Tfs != TimeFctStates::BlinkOn) && (Tfs != TimeFctStates::BlinkOff))
   Tfs = TimeFctStates::Idle;
  break;
 default:
  Tfs = TimeFctStates::Idle;
 }
 ChannelStates[chno].TFState = Tfs;
 int StairConfig = (ReadChConfigByte(chno, APP_SW_TIMWARNING_O) & APP_SW_TIMWARNING_M) >> APP_SW_TIMWARNING_B;
 // Falls sich das Treppenlicht in der Warnphase befindet und Warnung über Objekt aktiv ist -> Objekt senden
 if (((StairConfig & 1) != 0) && ((Tfs == TimeFctStates::StairWarn1) || (Tfs == TimeFctStates::StairWarn2)))
  StairSendWarnObject(chno, true);
 ChannelStates[chno].TFTargetTime = PtrRdUint32(ptr) + referenceTime;
 ptr+=4;
 ChObjectUpdate(chno, OBJ_TIMDURATION, PtrRdUint16(ptr));
 ptr+=2;
}
// USR_CALLBACK_RESET
// USR_CALLBACK_FLASH
// USR_CALLBACK_BCU_END
// STOREAPPL_DOWNLOAD
// STOREAPPL_BUSVFAIL
// RECALLAPPL_INIT

// Wird nur nach Systemstart aufgerufen, genau genommen ist type=RECALLAPPL_STARTUP also immer gegeben.
void Appl::RecallAppData(int type)
{
 byte* StoragePtr;
 unsigned referenceTime = systemTime;
 StoragePtr = memMapper.memoryPtr(0, false);
 byte StorageReason = *StoragePtr++;
 if ((StorageReason != 0) && (StorageReason != 255)) // Ansonsten würde da etwas gar nicht stimmen (Sektor leer zB)
 {
  for (int chno = 0; chno < CHANNELCNT; chno++)
  {
   if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
   {
    // Für den Teil der Daten, die nicht gespeichert werden, wird hier
    // erst mal die Default-Konfiguration hergestellt.
    SetIniChannelState(chno);
    if (*StoragePtr++ == 0x5A) // 1 Byte
    {
     StoragePtr++; // 1 Byte  Die CRC wird beim Lesen nicht ausgewertet
     RecallChannelState(chno, StoragePtr, referenceTime); // 16 Byte
     StoragePtr += 16;
    } else {
     // Die korrekte Signatur am Anfang der Daten fehlt. Vielleicht war der Kanal als
     // Heizungsaktor konfiguriert -> wird nicht unterstützt.
     StoragePtr += 17;
    }
    if (type == RECALLAPPL_STARTUP)
    {
     ModifyChStateAfterBusVoltageRecovery(chno);
    }
   }
  }
  StoragePtr += relay.SetData((void *)StoragePtr);
 } else {
  // Es wird keine Konfiguration gelesen, nur Defaultzustände wiederherstellen
  for (int chno = 0; chno < CHANNELCNT; chno++)
  {
   if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
   {
    // Für den Teil der Daten, die nicht gespeichert werden, wird hier
    // erst mal die Default-Konfiguration hergestellt.
    SetIniChannelState(chno);
    if (type == RECALLAPPL_STARTUP)
    {
     ModifyChStateAfterBusVoltageRecovery(chno);
    }
   }
  }
 }
}

void Appl::StoreApplData(int callbacktype)
{
 byte* StoragePtr;
 unsigned referenceTime = systemTime;
 // Kann der Mapper überhaupt die Seite 0 mappen? Checken!
 memMapper.writeMem(0, 0); // writeMem() aktiviert die passende Speicherseite, entgegen zu memoryPtr()
 StoragePtr = memMapper.memoryPtr(0, false);
 *StoragePtr++ = callbacktype; // Der Grund für das Speichern wird auch abgelegt. Vielleicht ganz nützlich.
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   *StoragePtr++ = 0x5A; // 1 Byte
   byte OldCrc = *StoragePtr;
   byte NewCrc = crc_calc(userMemoryPtr(APP_STARTADDR+APP_CHOFFS*chno), APP_CHOFFS);
   if (callbacktype == STOREAPPL_DOWNLOAD)
   {
    if (OldCrc != NewCrc)
    {
     SetIniChannelState(chno);
     OldCrc = NewCrc;
    } else {
     RestartSkipBvrMask |= 1 << chno;
    }
    ModifyChStateAfterDownload(chno);
   }
   if (callbacktype == USR_CALLBACK_RESET)
   {
    ModifyChStateBeforeReset(chno);
   }
   *StoragePtr++ = OldCrc; // 1 Byte
   StoreChannelState(chno, StoragePtr, referenceTime); // 16 Byte
   StoragePtr += 16;
  } else {
   // Nicht als Schaltausgang konfiguriert, Dummydaten abspeichern
   for (int i=0;i<18;i++) // 18 Byte
    *StoragePtr++ = 0;
  }
 }
 StoragePtr += relay.GetData((void *)StoragePtr);
 memMapper.doFlash();
}

// Zustand des Kanals, noch vor einer evtl. gewählten Ausgangsinvertierung
// Holt sein Info aus dem Relaiszustand und rechnet rück.
bool Appl::GetSwitchStatus(int chno)
{
 bool RelState = (relay.GetTrgState() & (1 << chno)) != 0;
 if (ReadChConfigByte(chno, APP_SW_NONC_O) & APP_SW_NONC_M)
 {
  RelState = not RelState;
 }
 return RelState;
}

//void Appl::WriteOrUpdateObject(int objno, unsigned int value)
//{
// if (AppObjSendEnabled())
// {
//  objectWrite(objno, value);
// } else {
//  objectUpdate(objno, value);
// }
//}

void Appl::UpdateAllStatusObjects(void)
{
 for (int chno = 0; chno < CHANNELCNT; chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   bool ActRelStatus = GetSwitchStatus(chno); // Berücksichtigt bereits eine evtl Ausgangsinvertierung
   if (ReadChConfigByte(chno, APP_STSINV_O) & APP_STSINV_M)
   { // Status invertieren
    ActRelStatus = not ActRelStatus;
   }
   if (ReadChConfigByte(chno, APP_SNDSTSAT_O) & APP_SNDSTSAT_M)
    // Status versenden ist freigeschaltet
    ChObjectWrite(chno, OBJ_STATESW, ActRelStatus ? 1 : 0);
   else
    ChObjectUpdate(chno, OBJ_STATESW, ActRelStatus ? 1 : 0);
  }
 }
}

void Appl::UpdateStatusObjekt(int chno, TStateAndTrigger &trigger) // wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein Vergleich mit dem alten Zustand stattfinden
{
 if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
 {
  bool Send = false;
  bool ActRelStatus = GetSwitchStatus(chno); // Berücksichtigt bereits eine evtl Ausgangsinvertierung
  unsigned conf = (ReadChConfigByte(chno, APP_SNDSTSAT_O) & APP_SNDSTSAT_M) >> APP_SNDSTSAT_B;
  if (conf == 3)
  { // Status immer senden, dann reicht "Evaluated"
   Send = trigger.Evaluated || trigger.Sw; // Letzte Bedingung sollte eigentlich nie ohne erste Bedingung auftreten
  } else { // Status nur bei Änderung senden
   if (trigger.Sw)
   {
    Send = ActRelStatus ^ trigger.SwOnOff;
   }
  }
  if (Send)
  {
   if (trigger.Sw)
   {
    ActRelStatus = trigger.SwOnOff;
   }
   if (ReadChConfigByte(chno, APP_STSINV_O) & APP_STSINV_M)
   { // Status invertieren
    ActRelStatus = not ActRelStatus;
   }
   if (conf != 0)
   {
    ChObjectWrite(chno, OBJ_STATESW, ActRelStatus ? 1 : 0);
    //objectWrite(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+OBJ_STATESW, ActRelStatus ? 1 : 0);
   } else {
    ChObjectUpdate(chno, OBJ_STATESW, ActRelStatus ? 1 : 0);
    //objectUpdate(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+OBJ_STATESW, ActRelStatus ? 1 : 0);
   }
  }
 }
}

void Appl::ChannelTrigger2RelaySwitch(int chno, TStateAndTrigger &trigger)
{
 if (trigger.Sw)
 {
  bool RelState = trigger.SwOnOff;
  if (ReadChConfigByte(chno, APP_SW_NONC_O) & APP_SW_NONC_M)
  {
   RelState = not RelState;
  }
  relay.Switch(chno, RelState);
 }
}

byte Appl::IniValueThresholdState(int chno)
{
 unsigned short ThLow = ReadChConfigUInt16(chno, APP_SW_THCONST2_O);
 unsigned short ThHigh;
 unsigned short ObjThreshVal = ChObjectRead(chno, OBJ_THRESHOLD);
 unsigned short Threshold1 = ChObjectRead(chno, OBJ_CHGTHRESH1);

 // Die Thresholds müssen noch sortiert werden
 if (Threshold1 > ThLow)
 {
  ThHigh = Threshold1;
 } else {
  ThHigh = ThLow;
  ThLow = Threshold1;
 }
 if (ObjThreshVal > ThHigh) // Überschreitung oberer Hysteresepunkt
 {
  return 1;
 }
 if (ObjThreshVal < ThLow) // Unterschreitung unterer Hysteresepunkt
 {
  return 2;
 }
 return 0;
}

void Appl::ProcessThresholds(int chno, TStateAndTrigger &Trigger)
{
 //ReadChConfigByte(chno, APP_SW_THTYPE_O) // Datentyp der Schwellwerte (1Byte/2Byte) sollte egal sein, objectRead liest beides unterschiedlos.
 // Es wird nicht zwischen 1Byte/2Byte Funktionalität unterschieden. Conf-Werte & Co sollten immer für beides passen
 // Jedoch muss der Objekttyp in der Objekttabelle bei Systemstart angepasst werden.
 unsigned ThAsHyst = ReadChConfigByte(chno, APP_SW_THISHYST_O) & APP_SW_THISHYST_M;
 unsigned short ThLow = ReadChConfigUInt16(chno, APP_SW_THCONST2_O);
 unsigned short ThHigh;
 unsigned short ObjThreshVal = ChObjectRead(chno, OBJ_THRESHOLD);
 unsigned short Threshold1 = ChObjectRead(chno, OBJ_CHGTHRESH1);
 // Die Thresholds müssen noch sortiert werden
 if (Threshold1 > ThLow)
 {
  ThHigh = Threshold1;
 } else {
  ThHigh = ThLow;
  ThLow = Threshold1;
 }
 if (ThAsHyst == 1) // Schwellwerte bilden Hysteresepunkte
 {
  if (ObjThreshVal > ThHigh) // Überschreitung oberer Hysteresepunkt
  {
   // Die Aktionen sollen nur ausgeführt werden, wenn die Schaltschwelle erstmalig überschritten worden ist.
   if ((ChannelStates[chno].OldThreshState & 3) != 1)
   {
    ChannelStates[chno].OldThreshState &= 0xfc;
    ChannelStates[chno].OldThreshState |= 1;
    unsigned Action = ReadChConfigByte(chno, APP_SW_THREAKTHIGH_O) & APP_SW_THREAKTHIGH_M;
    if (Action == 0) // Ausschalten
    {
     Trigger.Sw = true;
     Trigger.SwOnOff = false;
     Trigger.Evaluated = true;
    } else if (Action == 1) // Einschalten
    {
     Trigger.Sw = true;
     Trigger.SwOnOff = true;
     Trigger.Evaluated = true;
    }
   }
  } else if (ObjThreshVal < ThLow) // Unterschreitung unterer Hysteresepunkt
  {
   if ((ChannelStates[chno].OldThreshState & 3) != 2)
   {
    ChannelStates[chno].OldThreshState &= 0xfc;
    ChannelStates[chno].OldThreshState |= 2;
    unsigned Action = ReadChConfigByte(chno, APP_SW_THREAKTLOW_O) & APP_SW_THREAKTLOW_M;
    if (Action == 0) // Ausschalten
    {
     Trigger.Sw = true;
     Trigger.SwOnOff = false;
     Trigger.Evaluated = true;
    } else if (Action == 1) // Einschalten
    {
     Trigger.Sw = true;
     Trigger.SwOnOff = true;
     Trigger.Evaluated = true;
    }
   }
  }
 } else if (ThAsHyst == 2) // Schwellwerte bilden keine Hysteresepunkte, es wird der Aufenthalt in drei Bereichen kontrolliert
 {
  if (ObjThreshVal > ThHigh) // Wert im oberen Bereich
  {
   unsigned Action = ReadChConfigByte(chno, APP_SW_THREAKTHIGH_O) & APP_SW_THREAKTHIGH_M;
   if (Action == 0) // Ausschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = false;
    Trigger.Evaluated = true;
   } else if (Action == 1) // Einschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = true;
    Trigger.Evaluated = true;
   }
  } else if (ObjThreshVal < ThLow) // Wert im unteren Bereich
  {
   unsigned Action = ReadChConfigByte(chno, APP_SW_THREAKTLOW_O) & APP_SW_THREAKTLOW_M;
   if (Action == 0) // Ausschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = false;
    Trigger.Evaluated = true;
   } else if (Action == 1) // Einschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = true;
    Trigger.Evaluated = true;
   }
  } else { // Wert in der Mitte
   unsigned Action = ReadChConfigByte(chno, APP_SW_THREAKTMID_O) & APP_SW_THREAKTMID_M;
   if (Action == 0) // Ausschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = false;
    Trigger.Evaluated = true;
   } else if (Action == 1) // Einschalten
   {
    Trigger.Sw = true;
    Trigger.SwOnOff = true;
    Trigger.Evaluated = true;
   }
  }
 }
}

TStateAndTrigger Appl::ProcessSwitchObj(int objno, int chno)
{
 TStateAndTrigger Result = {false, false, false, false};
 unsigned value = ChObjectRead(chno, objno);
 switch (objno)
 {
  //=================
 case OBJ_SWITCH: // Einfach nur schalten...
  //=================
  Result.Sw = true;
  Result.SwOnOff = (value != 0);
  Result.Evaluated = true;
  break;
 //=================
 case OBJ_CALLPRESET: // Einen Preset aufrufen (je nach Konf kann dies auch lediglich einen Preset selber verändern)
  //=================
  // Preset-Funktion freigeschaltet?
  if (ReadChConfigByte(chno, APP_ENAFUNCPRESET_O) & APP_ENAFUNCPRESET_M)
  {
   if (value) // Ist es Preset 2? (Wert 1 -> Preset 2)
   {
    switch (ChannelStates[chno].Preset >> 4)
    {
    case 0:
     Result.Sw = true;
     Result.SwOnOff = false;
     Result.Evaluated = true;
     break;
    case 1:
     Result.Sw = true;
     Result.SwOnOff = true;
     Result.Evaluated = true;
     break;
    }
    // Bei Aufruf von Preset 2 wird der Vorzustand immer abgespeichert, je nach Konf muss der später wiederhergestellt werden können
    ChannelStates[chno].IntSwitchStates &= ~STPREPRESET2_M;
    if (GetSwitchStatus(chno))
     ChannelStates[chno].IntSwitchStates |= STPREPRESET2_ON;
   } else { // Preset 1
    switch (ChannelStates[chno].Preset & 0xf)
    {
    case 0: // Ausschalten
     Result.Sw = true;
     Result.SwOnOff = false;
     Result.Evaluated = true;
     break;
    case 1: // Einschalten
     Result.Sw = true;
     Result.SwOnOff = true;
     Result.Evaluated = true;
     break;
    case 4: // Zustand vor Preset 2 wiederherstellen
     if ((ChannelStates[chno].IntSwitchStates & STPREPRESET2_M) < 2)
     {
      Result.Sw = true;
      Result.SwOnOff = ((ChannelStates[chno].IntSwitchStates & STPREPRESET2_M) != 0);
      Result.Evaluated = true;
      // Der PrePreset-Zustand kann nur einmal wiederhergestellt werden, daher jetzt löschen:
      ChannelStates[chno].IntSwitchStates |= STPREPRESET2_M;
     }
     break;
    case 6: // Ursprüngliche Konfiguration von Preset 2 wiederherstellen.
     ChannelStates[chno].Preset =
       (ChannelStates[chno].Preset & 0x0f) | ((ReadChConfigByte(chno, APP_SW_PRESET2VAL1_O) & 0xf) << 4);
     break;
    }
   }
  }
  break;
  //=================
 case OBJ_SETPRESET: // Setzen der Presetwerte mittels Bustelegramm
  //=================
  // Löst also als Reaktion nie einen Schaltvorgang aus
  // Preset-Funktion freigeschaltet?
  if (ReadChConfigByte(chno, APP_ENAFUNCPRESET_O) & APP_ENAFUNCPRESET_M)
  {
   if (value) // Ist es Preset 2? (Wert 1 -> Preset 2)
   {
    //ChannelStates[chno].Preset2 = GetSwitchStatus(chno) ? 1 : 0;
    ChannelStates[chno].Preset = (ChannelStates[chno].Preset & 0x0f) | (GetSwitchStatus(chno) ? 0x10 : 0);
   } else { // Preset 1
    if ((ChannelStates[chno].Preset & 0x0f) <= 1)
    { // Preset 1 über Bus ändern geht nur, wenn der Preset vorher nicht in einer Sonderfunktion war
     //ChannelStates[chno].Preset1 = GetSwitchStatus(chno) ? 1 : 0;
     ChannelStates[chno].Preset = (ChannelStates[chno].Preset & 0xf0) | (GetSwitchStatus(chno) ? 1 : 0);
    }
   }
  }
  break;
  //=================
 case OBJ_SCENE: // Szene aufrufen oder speichern
  //=================
  // value: Bit 7: "0" aufrufen, "1" speichern. Bit 6: don't care. Bit 5..0: Szenenummer-1
  // Szenen-Funktion freigeschaltet?
  if (ReadChConfigByte(chno, APP_ENAFUNCSCENE_O) & APP_ENAFUNCSCENE_M)
  {
   bool store = (value & 0x80);
   value &= 63;
   unsigned sceneno;
   byte cnf;
   for (byte scenememno=0; scenememno<5; scenememno++)
   {
    cnf = ReadChConfigByte(chno, APP_SW_SCEPRESET1_O+scenememno);
    if ((cnf & APP_SW_SCENEUNUSED_M) == 0)
    {
     sceneno = (cnf & APP_SW_SCENUMBER1_M) >> APP_SW_SCENUMBER1_B;
     if (sceneno == value)
     {
      if (store) // Abspeichern
      {
       if (GetSwitchStatus(chno))
        ChannelStates[chno].ScenesVal |= (1 << scenememno);
       else
        ChannelStates[chno].ScenesVal &= ~(1 << scenememno);
      } else { // Aufrufen
       Result.Sw = true;
       Result.SwOnOff = (ChannelStates[chno].ScenesVal & (1 << scenememno));
       Result.Evaluated = true;
      }
     }
    }
   }
  }
  break;
  //=================
 case OBJ_THRESHOLD:
  //=================
  // Threshold-Funktion freigeschaltet?
  if (ReadChConfigByte(chno, APP_ENAFUNTHRESH_O) & APP_ENAFUNTHRESH_M)
  {
   //ChannelStates[chno].OldTreshVal = value; Wert wird direkt aus dem Objekt gelesen
   ProcessThresholds(chno, Result);
  }
  break;
  //=================
 case OBJ_CHGTHRESH1:
  //=================
  // Threshold-Funktion freigeschaltet?
  if (ReadChConfigByte(chno, APP_ENAFUNTHRESH_O) & APP_ENAFUNTHRESH_M)
  {
   //ChannelStates[chno].Threshold1 = value; Wert wird direkt aus dem Objekt gelesen
   ProcessThresholds(chno, Result);
  }
  break;
 }
 if (Result.Sw)
 {
  ChannelStates[chno].IntSwitchStates &= ~STSWITCHOBJ_M;
  if (Result.SwOnOff)
   ChannelStates[chno].IntSwitchStates |= STSWITCHOBJ_M;
 }
 return Result;
}

void Appl::OneLogicFunction(byte LogicNo, TStateAndTrigger &trigger, int chno)
{
 if (ReadChConfigByte(chno, APP_SW_LOG1ENA_O+LogicNo*2) & APP_SW_LOG1ENA_M) // Logikfunktion Nummer "LogicNo"+1 freigegeben?
 {
  switch (ReadChConfigByte(chno, APP_SW_LOG1FCT_O+LogicNo*2) & APP_SW_LOG1FCT_M) // Welcher Logikmodus
  {
  case 1: // Und-Verknüpfung
   trigger.OnOff = trigger.OnOff && ((ChannelStates[chno].LogicObjVals & (1 << (2*LogicNo))) != 0);
   if (trigger.Sw || ((ChannelStates[chno].LogicObjVals & (2 << (2*LogicNo))) != 0))
   {
    trigger.Sw = true;
    trigger.SwOnOff = trigger.OnOff;
   }
   break;
  case 2: // Oder-Verknüpfung
   trigger.OnOff = trigger.OnOff || ((ChannelStates[chno].LogicObjVals & (1 << (2*LogicNo))) != 0);
   if (trigger.Sw || ((ChannelStates[chno].LogicObjVals & (2 << (2*LogicNo))) != 0))
   {
    trigger.Sw = true;
    trigger.SwOnOff = trigger.OnOff;
   }
   break;
  case 3: // XOR-Verknüpfung
   trigger.OnOff = trigger.OnOff != ((ChannelStates[chno].LogicObjVals & (1 << (2*LogicNo))) != 0);
   if (trigger.Sw || ((ChannelStates[chno].LogicObjVals & (2 << (2*LogicNo))) != 0))
   {
    trigger.Sw = true;
    trigger.SwOnOff = trigger.OnOff;
   }
   break;
  case 4: // Gate
   // Wenn "Deaktivieren bei Objektwert 0" ist identisch mit "Objektwert ist 0" gilt,
   // ist das Gate offen, d.h. ein Schaltauftrag kommt nicht durch und wird gelöscht.
   if (((ReadChConfigByte(chno, APP_SW_LOG1GATEDISVAL_O+LogicNo*2) & APP_SW_LOG1GATEDISVAL_M) == 0) == // Deaktivieren bei Objektwert 0
      ((ChannelStates[chno].LogicObjVals & (1 << 2*LogicNo)) == 0)) // Objektwert ist 0
    trigger.Sw = false;
   break;
  }
  ChannelStates[chno].LogicObjVals &= ~(2 << (2*LogicNo));
  if (ReadChConfigByte(chno, APP_SW_LOG1XORINV_O+LogicNo*2) & APP_SW_LOG1XORINV_M) // Ergebnis noch invertieren?
  {
   trigger.SwOnOff = not trigger.SwOnOff;
   trigger.OnOff = not trigger.OnOff;
  }
 }
}

void Appl::LogicFunction(TStateAndTrigger &trigger, int objno, int chno)
{
 // Rein für die Logikfunktionen müsste beim Empfang der Logikobjekte überhaupt nicht
 // geprüft werden, ob die Logik freigeschaltet ist. Jedoch gibt es den Fall, dass
 // bei Empfang der Logikobjekte u.U. Status-Telegramme verschickt werden sollen.
 // Also wird doch fen säuberlich geprüft.
 if (ReadChConfigByte(chno, APP_ENAFUNCLOGIC_O) & APP_ENAFUNCLOGIC_M)
 {
  trigger.OnOff = ((ChannelStates[chno].IntSwitchStates & STSWITCHOBJ_M) != 0);
  ChannelStates[chno].LogicObjVals &= 0xf5; // Änderungsflags löschen
  switch (objno)
  {
   //=================
  case OBJ_LOGIC1: // Logikobjekt 1
   //=================
   if (ReadChConfigByte(chno, APP_SW_LOG1ENA_O) & APP_SW_LOG1ENA_M)
   {
    unsigned value = ChObjectRead(chno, objno);
    ChannelStates[chno].LogicObjVals &= 0xfe;
    if (value)
     ChannelStates[chno].LogicObjVals |= 1;
    ChannelStates[chno].LogicObjVals |= 2;
    trigger.Evaluated = true;
    // ===> Bei Bedarf noch Status-Senden
    if ((ReadChConfigByte(chno, APP_SNDSTSAT_O) & APP_SNDSTSAT_M) == 3)
    {
     trigger.Evaluated = true;
    }
   }
   break;
  //=================
  case OBJ_LOGIC2: // Logikobjekt 2
   //=================
   unsigned value = ChObjectRead(chno, objno);
   ChannelStates[chno].LogicObjVals &= 0xfb;
   if (value)
    ChannelStates[chno].LogicObjVals |= 4;
   ChannelStates[chno].LogicObjVals |= 8;
   trigger.Evaluated = true;
   // ===> Bei Bedarf noch Status-Senden
   if ((ReadChConfigByte(chno, APP_SNDSTSAT_O) & APP_SNDSTSAT_M) == 3)
   {
    trigger.Evaluated = true;
   }
   break;
  }
  //if (trigger.Sw) // Die ganze Logik wird nur bei einem anliegenden Schaltauftrag ausgewertet
  {
   OneLogicFunction(0, trigger, chno); // Logikfunktion 1
   OneLogicFunction(1, trigger, chno); // Logikfunktion 2
  }
 }
}

// Die Bearbeitung der Zeitfunktion teilt sich in zwei verschiedene Funktionen:
// - Die Reaktion auf Schaltobjekte etc
// - Die Reaktion abhängig von abgelaufenen Zeiten
bool Appl::OneTimeFunctionsTimeRelated(TStateAndTrigger &trigger, int chno, unsigned referenceTime)
{
 trigger.Sw = false;
 if (ChannelStates[chno].TFState != TimeFctStates::Idle) // Zeitabhängige Aktion aktiv
 {
  // Es braucht nicht überprüft werden, welche Zeitfunktion aktiv ist (Treppenlicht, Verzögerung, Blinken).
  // Dies wird durch die Zustände TFState bereits abgebildet.
  if ((signed int)(referenceTime - ChannelStates[chno].TFTargetTime) > 0) // Zeit abgelaufen
  {
   int StairConfig = (ReadChConfigByte(chno, APP_SW_TIMWARNING_O) & APP_SW_TIMWARNING_M) >> APP_SW_TIMWARNING_B;
   int WarnTime = (int)ReadChConfigUInt16(chno, APP_SW_TIMWARNTIME_O)*1000;
   switch (ChannelStates[chno].TFState)
   {
   case TimeFctStates::StairOn: // Treppenlichtzeit abgelaufen
    if (StairConfig == 0) // Keine Vorwarnung
    {
     ChannelStates[chno].TFState = TimeFctStates::Idle;
     trigger.Sw = true;
     trigger.SwOnOff = false;
     trigger.Evaluated = true;
    } else {
     if (StairConfig & 1) // Warnung über Objekt
     {
      StairSendWarnObject(chno, true);
      ChannelStates[chno].TFState = TimeFctStates::StairWarn2; // Direkter Übergang zu Warn2 ohne Schaltaktion
      ChannelStates[chno].TFTargetTime = referenceTime + WarnTime; // Neue Endzeit
      // Einige Optionen werden u.U. gleich wieder überschrieben, wenn Warnung über Blinken aktiviert ist
     }
     if (StairConfig & 2) // Warnung über Blinken
     {
      trigger.Sw = true;
      trigger.SwOnOff = false;
      trigger.Evaluated = true;
      ChannelStates[chno].TFState = TimeFctStates::StairWarn1;
      ChannelStates[chno].TFTargetTime = referenceTime + STAIRCASEWARNBLINKTIME; // Neue Endzeit
     }
    }
    break;
   case TimeFctStates::StairWarn1: // Zeit des kurzen Aus-Blinkens zur Vorwarnung
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.Evaluated = true;
    ChannelStates[chno].TFState = TimeFctStates::StairWarn2;
    ChannelStates[chno].TFTargetTime = referenceTime + WarnTime; // Neue Endzeit
    break;
   case TimeFctStates::StairWarn2: // Die Vorwarnzeit ist abgelaufen, also ausschalten
    ChannelStates[chno].TFState = TimeFctStates::Idle;
    trigger.Sw = true;
    trigger.SwOnOff = false;
    trigger.Evaluated = true;
    if (StairConfig & 1) // Warnung über Objekt
    {
     StairSendWarnObject(chno, false);
    }
    break;
   case TimeFctStates::StairBack2On: // Rückkehr zum Ein-Zustand bei Retriggerung während StairWarn1
   {
    ChannelStates[chno].TFState = TimeFctStates::StairOn;
    unsigned StaircDurationInMs = ChObjectRead(chno, OBJ_TIMDURATION)*1000; //ChannelStates[chno].DurationStaircase*1000;
    ChannelStates[chno].TFTargetTime = referenceTime + StaircDurationInMs;
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.Evaluated = true;
   }
    break;
   case TimeFctStates::DelayOn: // Verzögerungsfunktion zum Einschalten
    ChannelStates[chno].TFState = TimeFctStates::Idle;
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.Evaluated = true;
    break;
   case TimeFctStates::DelayOff: // Verzögerungsfunktion zum Ausschalten
    ChannelStates[chno].TFState = TimeFctStates::Idle;
    trigger.Sw = true;
    trigger.SwOnOff = false;
    trigger.Evaluated = true;
    break;
   case TimeFctStates::BlinkOn: // Blinkfunktion, On-Teil beendet
   {
    ChannelStates[chno].TFState = TimeFctStates::BlinkOff;
    unsigned Duration = (unsigned)ReadChConfigUInt16(chno, APP_SW_FLASHOFFMIN_O)*60 + ReadChConfigByte(chno, APP_SW_FLASHOFFSEC_O);
    ChannelStates[chno].TFTargetTime = referenceTime + Duration*1000;
    trigger.Sw = true;
    trigger.SwOnOff = false;
    trigger.Evaluated = true;
   }
    break;
   case TimeFctStates::BlinkOff: // Blinkfunktion, Off-Teil beendet
   {
    bool End = false;
    if (ChannelStates[chno].BlinkDnCnt != 0)
    {
     End = (--ChannelStates[chno].BlinkDnCnt == 0);
    }
    if (End)
    {
     BlinkDeactivate(trigger, chno);
    } else {
     ChannelStates[chno].TFState = TimeFctStates::BlinkOn;
     unsigned Duration = (unsigned)ReadChConfigUInt16(chno, APP_SW_FLASHONMIN_O)*60 + ReadChConfigByte(chno,  APP_SW_FLASHONSEC_O);
     ChannelStates[chno].TFTargetTime = referenceTime + Duration*1000;
     trigger.Sw = true;
     trigger.SwOnOff = true;
     trigger.Evaluated = true;
    }
   }
    break;
   default:
    break;
   }
  }
 }
 return trigger.Sw;
}

// Die Bearbeitung der Zeitfunktion teilt sich in zwei verschiedene Funktionen:
// - Die Reaktion auf Schaltobjekte etc
// - Die Reaktion abhängig von abgelaufenen Zeiten
void Appl::OneTimeFunctionsObjRelated(TStateAndTrigger &trigger, int objno, int chno, unsigned referenceTime)
{
 // Zeitfunktionen in der Konfiguration aktiviert?
 if (ReadChConfigByte(chno, APP_ENAFUNCTIME_O) & APP_ENAFUNCTIME_M)
 {
  switch (objno)
  {
  //=================
  case OBJ_DISTIMEFCT: // Zeitfunktion über ein Objekt deaktivieren
   //=================
  {
   unsigned value = ChObjectRead(chno, objno);
   if (value != 0)
   {
    // Laufende Zeitfunktion doch nicht abbrechen...
    // Im Falle Treppenlicht
    //if ((ChannelStates[chno].TFState == TimeFctStates::StairWarn1) || (ChannelStates[chno].TFState == TimeFctStates::StairWarn2))
    // StairSendWarnObject(chno, false);
    //ChannelStates[chno].TFState = TimeFctStates::Idle;
    // Dies gilt sowohl für Treppenlicht, als auch für Verzögerung und Blinken
    ChannelStates[chno].IntSwitchStates |=  STDISTIMEFCT_M;
   } else {
    ChannelStates[chno].IntSwitchStates &=  ~STDISTIMEFCT_M;
   }
  }
  break;
  //=================
  case OBJ_TIMDURATION: // Zeitdauer über ein Objekt ändern
   //=================
   // Bei einem aktuell laufendes Treppenlicht wird die Restzeit nicht nachträglich geändert!
   // Da dieser Wert bei Bedarf direkt aus dem Objekt gelesen wird, ist hier keinerlei Aktion notwendig.
   //ChannelStates[chno].DurationStaircase = ChObjectRead(chno, objno);
   break;
  }

  // Die ganzen nachfolgenden Zeitfunktionen müssen nur abgearbeitet werden, wenn ein Schaltauftrag "von oben"
  // hereinkommt. Andernfalls wird nichts geändert.
  // Das hat auch zur Folge, dass ein gerade aktiviertes "Disables time function" erst mal nicht zu einem
  // Abbruch einer Zeitfunktion führt, erst nach einem Schaltobjekt.
  if (trigger.Sw)
  {
   if ((ChannelStates[chno].IntSwitchStates & STDISTIMEFCT_M) == 0) // Zeitfunktionen nicht per Objekt deaktiviert?
   {
    switch ((ReadChConfigByte(chno, APP_SW_TIMEFUNC_O) & APP_SW_TIMEFUNC_M) >> APP_SW_TIMEFUNC_B)
    {
    case 1: // Treppenlicht
    {
     // Erst mal das ankommende Schaltobjekt untersuchen und anhand der Konfiguration
     // auswählen, ob das Treppenlicht aktiviert oder deaktiviert werden soll.
     int OnOffObj = 0; // Steht für keine Aktion
     int OnOffConfig = (ReadChConfigByte(chno, APP_SW_TIMSWOFFEN_O) & APP_SW_TIMSWOFFEN_M) >> APP_SW_TIMSWOFFEN_B;
     if (OnOffConfig == 3)
     {
      OnOffObj = 2; // Einschalten
     } else {
      if (trigger.SwOnOff)
      {
       OnOffObj = 2; // Einschalten
      } else {
       if (OnOffConfig == 1)
       {
        OnOffObj = 1; // Ausschalten
       }
      }
     }
     trigger.Sw = false; // Ursprünglichen Trigger erst mal löschen, wird bei Bedarf unten neu erzeugt
     // Funktion ist Idle?
     if (ChannelStates[chno].TFState == TimeFctStates::Idle)
     {
      if (OnOffObj == 1) // Ausschalten
      { // Obwohl der Idle-Zustand aktiv ist, wird nochmal ausgeschaltet.
       trigger.Sw = true;
       trigger.SwOnOff = false;
       trigger.Evaluated = true;
      } else if (OnOffObj == 2) // Einschalten
      {
       ChannelStates[chno].TFTargetTime = referenceTime + ChObjectRead(chno, OBJ_TIMDURATION)*1000;//ChannelStates[chno].DurationStaircase*1000;
       ChannelStates[chno].TFState = TimeFctStates::StairOn;
       trigger.Sw = true;
       trigger.SwOnOff = true;
       trigger.Evaluated = true;
      }
     } else { // Die Treppenlichtfunktion ist bereits aktiv
      if (OnOffObj == 1) // Ausschalten
      {
       switch (ChannelStates[chno].TFState)
       {
       case TimeFctStates::StairOn:
        ChannelStates[chno].TFState = TimeFctStates::Idle;
        trigger.Sw = true;
        trigger.SwOnOff = false;
        trigger.Evaluated = true;
        break;
       case TimeFctStates::StairWarn2: // Ausgang in der Warnphase nach dem kurzen Aus-Tasten (wenn gewählt)
        ChannelStates[chno].TFState = TimeFctStates::Idle;
        trigger.Sw = true;
        trigger.SwOnOff = false;
        trigger.Evaluated = true;
        StairSendWarnObject(chno, false);
        break;
       case TimeFctStates::StairWarn1: // Ausgang gerade Warn-Aus-Blinkend
       case TimeFctStates::StairBack2On: // Ursprünglich sollte zum Ein-Zustand zurückgekehrt werden
        ChannelStates[chno].TFState = TimeFctStates::Idle;
        trigger.Sw = true;
        trigger.SwOnOff = false;
        trigger.Evaluated = true;
        StairSendWarnObject(chno, false);
        break;
       default:
        ChannelStates[chno].TFState = TimeFctStates::Idle;
        break;
       }
       //StairOn, StairWarn1, StairWarn2, StairBack2On
      } else if (OnOffObj == 2) // Retrigger/Pumpen
      {
       int RetriggerConfig = (ReadChConfigByte(chno, APP_SW_TIMPUMPUP_O) & APP_SW_TIMPUMPUP_M) >> APP_SW_TIMPUMPUP_B;
       if ((RetriggerConfig >= 1) && (RetriggerConfig <= 5))
       {
        if (ChannelStates[chno].TFState == TimeFctStates::StairBack2On)
        {
         // Sonderfall, während der kurzen Back2On-Phase soll gepumpt werden.
         // Eigentlich müsste die Dauer bis zum Abschalten vergrößert werden. Da sich das in diesem Zustand nicht
         // abbilden lässt (Abschaltzeit wird erst nach Back2On gesetzt), wird dieser sehr unwahrscheinliche
         // Fall für's erste ignoriert.
        } else {
         if (ChannelStates[chno].TFState == TimeFctStates::StairWarn1)
         {
          // Sonderfall, es ist gerade die Warn-Blink-Aus-Phase. Jetzt kann keine Zeit neu gesetzt werden, erst
          // muss dieses Blinken beendet werden.
          ChannelStates[chno].TFState = TimeFctStates::StairBack2On; // Das Warn-Blink wird noch fertig ausgeführt, danach geht's wieder zu "StairOn"
          StairSendWarnObject(chno, false); // Warnphase beenden
         } else {
          unsigned StaircDurationInMs = ChObjectRead(chno, OBJ_TIMDURATION)*1000;//ChannelStates[chno].DurationStaircase*1000;
          if ((RetriggerConfig == 1) || (ChannelStates[chno].TFState == TimeFctStates::StairWarn2))
          {
           // Bei Retrigger wird einfach nur die Zielzeit neu gesetzt, als wenn gerade das Treppenlicht erst eingeschaltet worden wäre
           // Wenn gerade Warnzeit war: Es kann keine Zeit zur Warnzeit hinzugepumpt werden.
           ChannelStates[chno].TFTargetTime = referenceTime + StaircDurationInMs;
          } else {
           ChannelStates[chno].TFTargetTime += StaircDurationInMs;
           StaircDurationInMs *= RetriggerConfig; // Der Konfig-Wert ist bei > 1 direkt: Erlaubtes Vielfaches Pumpen
           if ((ChannelStates[chno].TFTargetTime - referenceTime) > StaircDurationInMs) // nicht umsortieren wegen evtl Überläufen
            ChannelStates[chno].TFTargetTime = referenceTime + StaircDurationInMs;
          }
          if (ChannelStates[chno].TFState == TimeFctStates::StairWarn2)
          { // Ausgang in der Warnphase nach dem kurzen Aus-Tasten (wenn gewählt)
           ChannelStates[chno].TFState = TimeFctStates::StairOn; // Zustand ist wieder "StairOn"
           StairSendWarnObject(chno, false); // Warnphase beenden
          }
          // Braucht an sich nicht gesetzt zu werden, ist nur
          // bei Handbedienung zwischendurch relevant.
          trigger.Sw = true;
          trigger.SwOnOff = true;
          trigger.Evaluated = true;
         }
        }
       }
      }
     }
    }
    break;
    case 2: // Ein-/Ausverzögerung
     // Die Verzögerungsfunktion ist recht einfach gestaltet: Ein Ein-/Aus-Objekt startet eine konfigurierbare
     // Schalt-Verzögerung - allerdings nur wenn der Zielzustand sich vom aktuellen Zustand unterscheidet.
     // Wenn die Verzögerungsfunktion bereits aktiv war, gilt: Die Funktion neu gestartet, bzw. abgebrochen
     // wenn eine gegenteiliges Schaltobjekt empfangen wurde.
     trigger.Sw = false; // Ursprünglichen Trigger erst mal löschen
     unsigned DurationDelay;
     if (trigger.SwOnOff)
     {
      if (trigger.OnOff)
      { // Einschalten - aber der Kanal ist bereits eingeschaltet.
       ChannelStates[chno].TFState = TimeFctStates::Idle; // falls Delay-Off aktiv war, hier löschen
      } else {
       ChannelStates[chno].TFState = TimeFctStates::DelayOn;
       DurationDelay = (unsigned)ReadChConfigUInt16(chno, APP_SW_DELONTIME_O)*60 + ReadChConfigByte(chno, APP_SW_DELONTSEC_O);
       ChannelStates[chno].TFTargetTime = referenceTime + DurationDelay*1000;
      }
     } else {
      if (trigger.OnOff)
      {
       ChannelStates[chno].TFState = TimeFctStates::DelayOff;
       DurationDelay = (unsigned)ReadChConfigUInt16(chno, APP_SW_DELOFFTIME_O)*60 + ReadChConfigByte(chno, APP_SW_DELOFFTSEC_O);
       ChannelStates[chno].TFTargetTime = referenceTime + DurationDelay*1000;
      } else {
       // Ausschalten - aber der Kanal ist bereits ausgeschaltet.
       ChannelStates[chno].TFState = TimeFctStates::Idle; // falls Delay-On aktiv war, hier löschen
      }
     }
     break;
    case 3: // Blinken
     int OnOffObj = 0; // Steht für keine Aktion
     int OnOffConfig = (ReadChConfigByte(chno, APP_SW_FLASHOBJFL_O) & APP_SW_FLASHOBJFL_M) >> APP_SW_FLASHOBJFL_B;
     if (OnOffConfig == 3) // Immer Einschalten, egal ob 0 oder 1
     {
      OnOffObj = 3; // Blinken Aktivieren
     } else if (OnOffConfig == 1) // Blinken bei "1", Ausschalten bei "0"
     {
      if (trigger.SwOnOff)
       OnOffObj = 3; // Blinken Aktivieren
      else
       OnOffObj = 1; // Ausschalten
     } else if (OnOffConfig == 2) // Einschalten bei "1", Blinken bei "0"
     {
      if (trigger.SwOnOff)
       OnOffObj = 2; // Einschalten
      else
       OnOffObj = 3; // Blinken Aktivieren
     }
     trigger.Sw = false; // Ursprünglichen Trigger erst mal löschen, wird bei Bedarf unten neu erzeugt
     if (OnOffObj == 3)
     { // Blinkfunktion einschalten, geht nur wenn Idle (keine Retriggerung)
      if (ChannelStates[chno].TFState == TimeFctStates::Idle)
      {
       ChannelStates[chno].BlinkDnCnt = ReadChConfigByte(chno, APP_SW_FLASHNUMBER_O);
       unsigned Duration;
       // Zum Auftakt des Blinkens wird sofort umgeschaltet, der erste TFState ist also
       // abhängig vom aktuellen Schaltzustand
       if (trigger.OnOff)
       {
        ChannelStates[chno].IntSwitchStates |= STPREBLINK_M;
        Duration = (unsigned)ReadChConfigUInt16(chno, APP_SW_FLASHOFFMIN_O)*60 + ReadChConfigByte(chno, APP_SW_FLASHOFFSEC_O);
        ChannelStates[chno].TFState = TimeFctStates::BlinkOff;
        trigger.SwOnOff = false;
       } else {
        ChannelStates[chno].IntSwitchStates &= ~STPREBLINK_M;
        Duration = (unsigned)ReadChConfigUInt16(chno, APP_SW_FLASHONMIN_O)*60 + ReadChConfigByte(chno, APP_SW_FLASHONSEC_O);
        ChannelStates[chno].TFState = TimeFctStates::BlinkOn;
        trigger.SwOnOff = true;
       }
       ChannelStates[chno].TFTargetTime = referenceTime + Duration*1000;
       trigger.Sw = true;
       trigger.Evaluated = true;
      }
     } else if (OnOffObj == 1)
     { // Ausschalten
      trigger.Sw = true;
      trigger.SwOnOff = false;
      trigger.Evaluated = true;
      ChannelStates[chno].TFState = TimeFctStates::Idle;
     } else if (OnOffObj == 2)
     { // Einschalten
      trigger.Sw = true;
      trigger.SwOnOff = true;
      trigger.Evaluated = true;
      ChannelStates[chno].TFState = TimeFctStates::Idle;
     }
     break;
    }
   } else {
    ChannelStates[chno].TFState = TimeFctStates::Idle;
   }
  }
 }
}

void Appl::BlinkDeactivate(TStateAndTrigger &trigger, int chno)
{
 if ((ChannelStates[chno].TFState == TimeFctStates::BlinkOn) || (ChannelStates[chno].TFState == TimeFctStates::BlinkOff))
 { // Blinken ausschalten hat nur eine Funktion, wenn Blinken gerade aktiv ist
  switch ((ReadChConfigByte(chno, APP_SW_FLASHAFTER_O) & APP_SW_FLASHAFTER_M) >> APP_SW_FLASHAFTER_B)
  {
   case 0: // gespeicherter Zustand
    trigger.Sw = true;
    trigger.SwOnOff = ((ChannelStates[chno].IntSwitchStates & STPREBLINK_M) != 0);
    trigger.Evaluated = true;
    break;
   case 1: // Ausschalten
    trigger.Sw = true;
    trigger.SwOnOff = false;
    trigger.Evaluated = true;
    break;
   case 3: // Einschalten
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.Evaluated = true;
    break;
  }
 }
 ChannelStates[chno].TFState = TimeFctStates::Idle;
}

// Liefert "true" zurück, wenn gerade das Dauer-Ein deaktiviert wurde und die Treppenlichtfunktion
// neu gestartet werden soll.
// Ansonsten wird hier die Dauer-Ein Funktionalität realisiert.
bool Appl::PermanentOnFunction(TStateAndTrigger &trigger, int objno, int chno)
{
 if (ReadChConfigByte(chno, APP_ENAFUNCTIME_O) & APP_ENAFUNCTIME_M) // Zeitfunktion aktiviert
 {
  if (objno == OBJ_PERMANENTON)
  {
   trigger.Evaluated = true;
   unsigned value = ChObjectRead(chno, objno);
   if (value == 0)
   {
    if ((ChannelStates[chno].IntSwitchStates & STPERMANENTON_M) != 0)
    { // Deaktivierung eines PermanentOn Zustandes
     ChannelStates[chno].IntSwitchStates &= ~STPERMANENTON_M;
     if ((ChannelStates[chno].IntSwitchStates & STDISTIMEFCT_M) == 0) // Zeitfunktionen nicht per Objekt deaktiviert?
      if ((ReadChConfigByte(chno, APP_SW_TIMEFUNC_O) & APP_SW_TIMEFUNC_M) == 1) // Treppenlicht
       if (ReadChConfigByte(chno, APP_SW_TIMRESTAFPON_O) & APP_SW_TIMRESTAFPON_M) // Treppenlicht neu starten
       {
        return true; // Keine weiteren Wertänderungen, es wird eh nochmal eine Schleife gedreht
       }
     // Um den gespeicherten Zustand vor PermanentOn zum Ausgang zu bringen, wird daraus ein Switch-Trigger gemacht
     trigger.Sw = true;
     trigger.SwOnOff = trigger.OnOff;
    }
    // Wird das PermanentOn-Objekt wiederholt 0 gesetzt, ist das nur .Evaluated, sonst nix
   } else if (value == 1)
   {
    // Ein wiederholt 1 geschriebenes PermanentOn wirkt wie ein wiederholtes normales Einschalten:
    // Als Schalttrigger
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.OnOff = true;
    ChannelStates[chno].IntSwitchStates |= STPERMANENTON_M;
   }
  } else { // Keine Objektänderung, also Anwendung der PermanentOn Funktion
   if ((ChannelStates[chno].IntSwitchStates & STPERMANENTON_M) != 0)
   {
    trigger.Sw = false;   // Evtl Schaltauftrag wird gelöscht
    trigger.OnOff = true; // Zustand ist 1
   } else {
    // Ansonsten keinerlei Beeinflussung des Triggers
   }
  }
 }
 return false;
}

bool Appl::OneSafetyFunction(TStateAndTrigger &trigger, unsigned SafetyChanges, unsigned Mask, int ConfAddr, int chno, bool &EvaluateNextPrio)
{
 if (ChannelStates[chno].Safety & Mask) // Prio "x" ist aktiv
 {
  if ((SafetyChanges & Mask) || EvaluateNextPrio)
  { // Sicherheit Prio "x" gerade aktiviert oder Neuevaluierung gefragt
   switch (ReadChConfigByte(chno, ConfAddr))
   {
   case 0: // Bei Aktivierung Safety Kanal ausschalten
    trigger.Sw = true;
    trigger.SwOnOff = false;
    trigger.Evaluated = true;
    trigger.OnOff = false; // TODO muss das mitgeführt werden?
    break;
   case 1: // Bei Aktivierung Safety Kanal einschalten
    trigger.Sw = true;
    trigger.SwOnOff = true;
    trigger.Evaluated = true;
    trigger.OnOff = true; // TODO muss das mitgeführt werden?
    break;
   //case 2: // Ausgang unverändert lassen (in der Stellung blockieren)
   //case 255: Sicherheit inaktiv, dann wäre das Bit aber bereits aus .Safety herausgefiltert
   }
  } else { // Sicherheit Prio "x" bereits aktiv
   // Dann keine weitere Aktion notwendig
  }
  return true; // keine Auswertung der niedrigeren Prioritäten notwendig
 } else { // Prio "x" ist inaktiv
  if (SafetyChanges & Mask)
  { // Sicherheit Prio "x" gerade deaktiviert
   EvaluateNextPrio = true; // Dann muss eine Prio niedrigerer Stufe ihre Sicherheitsposition wiederherstellen
  }
}
 return false;
}

// SafetyChanges ist in Bit 0..2 gesetzt, wenn die Sicherheit Prio 1..3 gerade geändert worden ist.
// Diese Darstellung muss stimmen, denn die Abarbeitung unterscheidet sich, on ein Zustand konstant
// anliegt oder sich gerade geändert hat.
// Das Flag trigger.OnOff muss beim Aufruf der Funktion den Zustand enthalten, der ohne Sicherheit
// gelten würde. Nach Verlassen dieser Funktion ist das Flag nicht mehr gültig!
void Appl::ProcessChSafety(TStateAndTrigger &trigger, unsigned SafetyChanges, int objno, int chno)
{
 // Kann ankommende Trigger unterdrücken
 // Kann über Safety oder das Zwangsstellungsobjekt Trigger erzeugen
 bool ReEvaluateForcedOp = false;
 //SafetyChanges &= 7;
 if (ReadChConfigByte(chno, APP_ENAFUNCSAFETY_O) & APP_ENAFUNCSAFETY_M) // Kanalweise Safety Funktionalität aktiviert?
 {
  // Erst ForcedPos-Objekt auswerten und in eine Darstellung wie Safety/SafetyChanges umbauen
  // ForcedPos-Änderungen dabei in Bit 3 von SafetyChanges speichern
  // Bei variablem Zwangsstellungszielzustand (2bit Objekt) dies in .ForcedPos speichern
  if ((objno > 0) && ((objno & 0xff)) == OBJ_FORCEDOP)
  {
   trigger.Evaluated = true;
   unsigned value;
   if (objno & 0x100)
   { // Dann ist es keine Objektnummer, sondern in den Bits oberhalb von Bit 12 versteckt sich ein Objektwert
    value = (unsigned)objno >> 12;
   } else {
    value = ChObjectRead(chno, objno);
   }
   unsigned conf = ReadChConfigByte(chno, APP_SW_FORCEDPOS_O) & APP_SW_FORCEDPOS_M;
   if (conf > 2 )
   { // Steuerung über 2 Bit Objekt
    if (((value & 2) != 0) != ((ChannelStates[chno].Safety & 8) != 0)) // Aktivierungszustand geändert
     SafetyChanges |= 8;
    ChannelStates[chno].Safety &= 0xf7;
    if (value & 2) // Zwangsstellung ist aktiviert
    {
     ChannelStates[chno].Safety |= 8;
     ReEvaluateForcedOp = true;
    }
    ChannelStates[chno].ForcedPos &= 0xfe;
    if (value & 1) // Zielzustand ist "eingeschaltet"
     ChannelStates[chno].ForcedPos |= 1; // Zielzustand in .ForcedPos abspeichern
   } else { // 1 bit Objekt
    value &= 1;
    if ((value != 0) != ((ChannelStates[chno].Safety & 8) != 0)) // Aktivierungszustand geändert
     SafetyChanges |= 8;
    ChannelStates[chno].Safety &= 0xf7;
    if (value) // Zwangsstellung ist aktiviert
     ChannelStates[chno].Safety |= 8;
   }
  }
  SafetyChanges &= (ChannelStates[chno].Safety >> 4);
  if (SafetyChanges)
  {
   ChannelStates[chno].Safety &= 0xf8;
   ChannelStates[chno].Safety |= ActuatorSafety & (ChannelStates[chno].Safety >> 4);
   // Jetzt in .Safety:
   // Bit 7..4: Maske der freigeschalteten Sicherheitsoptionen für diesen Kanal
   // Bit 3..0: Aktueller Aktivierungsstand der Sicherheitsoptionen, soweit für diesen Kanal freigeschaltet
  }
  if ((ChannelStates[chno].Safety & 0xf) == 0)
  {
   if (SafetyChanges)
   { // Es wurde gerade die letzte Sicherheitsoption dieses Kanals deaktiviert
    // Nach Optionsauswahl: Ein/Aus/unverändert/Zustand ohne Sicherheit
    switch (ReadChConfigByte(chno, APP_SW_FORCEDEND_O))
    {
    case 0: // Bei Ende Safety Kanal ausschalten
     trigger.Sw = true;
     trigger.SwOnOff = false;
     trigger.Evaluated = true;
     trigger.OnOff = false; // TODO muss das mitgeführt werden?
     break;
    case 1: // Bei Ende Safety Kanal einschalten
     trigger.Sw = true;
     trigger.SwOnOff = true;
     trigger.Evaluated = true;
     trigger.OnOff = true; // TODO muss das mitgeführt werden?
     break;
    case 3: // Kontaktposition neu bestimmen, also aus dem in Trigger gelieferten Zustand einen Schaltauftrag machen
     trigger.Sw = true;
     trigger.SwOnOff = trigger.OnOff; // Genau für diesen Fall wird trigger.OnOff erzeugt!
     trigger.Evaluated = true;
    //case 2: // Ausgang unverändert lassen
    }
   }
  } else {
   // Irgendeine der Sicherheitsfunktionen ist aktiv, also darf kein anderer Trigger durchkommen
   trigger.Sw = false;
   // TODO Andere Flags mitführen?

   bool EvaluateNextPrio = false;
   // ============
   // Prio 1
   bool Done = OneSafetyFunction(trigger, SafetyChanges, 1, APP_SW_FORCEDPR1_O, chno, EvaluateNextPrio);
   // ============
   // Zwangsstellung / Forced Positioning
   if (not Done)
   {
    if (ChannelStates[chno].Safety & 8) // Forced Positioning ist aktiv
    {
     Done = true; // keine Auswertung der niedrigeren Prioritäten notwendig
     if ((SafetyChanges & 8) || EvaluateNextPrio || ReEvaluateForcedOp)
     { // Forced Positioning gerade aktiviert (oder muss neu gesetzt werden durch 2bit Objekt)

      if (ChannelStates[chno].ForcedPos & 2)
      {
       // In diesem Fall Ausgang unverändert lassen
      } else { // Ausgang in bestimmte Richtung schalten
       trigger.Sw = true;
       trigger.SwOnOff = ((ChannelStates[chno].ForcedPos & 1) != 0);
       trigger.Evaluated = true;
       trigger.OnOff = true; // TODO muss das mitgeführt werden?
      }
     } else { // Zwangsstellung bereits aktiv
      // Dann keine weitere Aktion notwendig
      Done = true;
     }
    } else { // Zwangsstellung ist inaktiv
     if (SafetyChanges & 1)
     { // Zwangsstellung gerade deaktiviert
      EvaluateNextPrio = true; // Dann muss eine Prio niedrigerer Stufe ihre Sicherheitsposition wiederherstellen
     }
    }
   }
   // ============
   // Prio 2
   if (not Done)
   {
    Done = OneSafetyFunction(trigger, SafetyChanges, 2, APP_SW_FORCEDPR2_O, chno, EvaluateNextPrio);
   }
   // ============
   // Prio 3
   if (not Done)
   {
    OneSafetyFunction(trigger, SafetyChanges, 4, APP_SW_FORCEDPR3_O, chno, EvaluateNextPrio);
   }
  }
 }
}

void Appl::PostProcessSingleSwitchObject(int chno, TStateAndTrigger &trigger, int objno, unsigned referenceTime)
{
 LogicFunction(trigger, objno, chno);
 trigger.OnOff = ((ChannelStates[chno].IntSwitchStates & STAFTERTIMFCT_M) != 0); // Delay benötigt den alten Zustand
 bool RepeatTimeFct;
 do {
  OneTimeFunctionsObjRelated(trigger, objno, chno, referenceTime);
  if (trigger.Sw) {
   if (trigger.SwOnOff) {
    ChannelStates[chno].IntSwitchStates |= STAFTERTIMFCT_M;
   } else {
    ChannelStates[chno].IntSwitchStates &= ~STAFTERTIMFCT_M;
   }
  }
  trigger.OnOff = ((ChannelStates[chno].IntSwitchStates & STAFTERTIMFCT_M) != 0);
  RepeatTimeFct = PermanentOnFunction(trigger, objno, chno);
  if (RepeatTimeFct) {
   // Offensichtlich wurde die Funktion "Treppenlicht neu starten bei Deaktivierung von Dauer-Ein" ausgelöst.
   // Das zieht eine erneute Ausführung der Zeitfunktionen nach sich.
   // Dafür wird in die Funktion ein "Einschalten" eingespielt.
   // (Ob Treppenlicht aktiv etc etc wird bereits in der PermanentOnFunction überprüft)
   objno = -1;
   trigger.Sw = true;
   trigger.SwOnOff = true;
  }
 } while (RepeatTimeFct);
 // Erzeugt Safety .Evaluated?
 ProcessChSafety(trigger, 0, objno, chno);
 // Auswertung von .Evaluated? Nur für das Statusobjekt
 UpdateStatusObjekt(chno, trigger); // wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein Vergleich mit dem alten Zustand stattfinden
 ChannelTrigger2RelaySwitch(chno, trigger); // Berücksichtigt evtl Invertierung
}

void Appl::OneCurrThresholdFct(int chno, float IMeas, int fctno)
{
#ifndef OMITCURRFCT
 float ScalVal;
 if (fctno == 0)
 { // Erste Schwellwertfunktion
  if (ReadChConfigByte(chno, APP_CURRTHSCAL1_O) & APP_CURRTHSCAL1_M)
   ScalVal = 0.01;
  else
   ScalVal = 0.1;
 } else { // Zweite Schwellwertfunktion
  ScalVal = 0.1;
 }
 float ThVal = ScalVal * ReadChConfigByte(chno, APP_CURRTH1_O+(APP_CURRTH2_O-APP_CURRTH1_O)*fctno);
 byte HystCnf = ReadChConfigByte(chno, APP_CURRHYST1_O+(APP_CURRTH2_O-APP_CURRTH1_O)*fctno) & APP_CURRHYST1_M;
 if (HystCnf == 0)
  return;
 float Hyst;
 if (HystCnf == 3)
  Hyst = 0.0015;
 else
  Hyst = 0.0125*HystCnf;
 bool ObjUpdate = false;
 int ObjVal = 0;
 if (IMeas > (ThVal+Hyst)) // Überschreitung oberer Hysteresepunkt
 {
  // Die Aktionen sollen nur ausgeführt werden, wenn die Schaltschwelle erstmalig überschritten worden ist.
  if (((ChannelStates[chno].CurrFctStates >> (fctno*CFTHRESHSTATE2_O)) & CFTHRESHSTATE1_M) != 1)
  {
   ChannelStates[chno].CurrFctStates &= ~(CFTHRESHSTATE1_M << (fctno*CFTHRESHSTATE2_O));
   ChannelStates[chno].CurrFctStates |= 1 << (fctno*CFTHRESHSTATE2_O);
   unsigned Action = ReadChConfigByte(chno, APP_CURRSNDCROSS1_O+(APP_CURRTH2_O-APP_CURRTH1_O)*fctno) & APP_CURRSNDCROSS1_M;
   switch(Action)
   {
   case 1:
   case 13:
    // Sende 0
    ObjUpdate = true;
    break;
   case 3:
   case 7:
    // Sende 1
    ObjVal = 1;
    ObjUpdate = true;
    break;
   default:
    break;
   }
  }
 } else if (IMeas < (ThVal-Hyst)) // Unterschreitung unterer Hysteresepunkt
 {
  if (((ChannelStates[chno].CurrFctStates >> (fctno*CFTHRESHSTATE2_O)) & CFTHRESHSTATE1_M) != 2)
  {
   ChannelStates[chno].CurrFctStates &= ~(CFTHRESHSTATE1_M << (fctno*CFTHRESHSTATE2_O));
   ChannelStates[chno].CurrFctStates |= 2 << (fctno*CFTHRESHSTATE2_O);
   unsigned Action = ReadChConfigByte(chno, APP_CURRSNDCROSS1_O+(APP_CURRTH2_O-APP_CURRTH1_O)*fctno) & APP_CURRSNDCROSS1_M;
   switch(Action)
   {
   case 4:
   case 7:
    // Sende 0
    ObjUpdate = true;
    break;
   case 12:
   case 13:
    //  Sende 1
    ObjVal = 1;
    ObjUpdate = true;
    break;
   default:
    break;
   }
  }
 }
 if (ObjUpdate)
  ChObjectWrite(chno, OBJ_STATECTH1+fctno, ObjVal);
#endif
}

// Aufruf wenn neue Strommesswerte zur Verfügung stehen
void Appl::CurrentFunctions(unsigned referenceTime)
{
#ifndef OMITCURRFCT
 bool IniAppRun = ((ChannelStates[0].CurrFctStates & CFINIDONE2_M) == 0) && AppObjSendEnabled();
 bool IniAppStartup = ((ChannelStates[0].CurrFctStates & CFINIDONE1_M) == 0);
 /* Verhalten bei Startup:
  * - Wenn Versenden des Stromwerts konfiguriert ist (zeit-/stromänderungsabhängig), wird beim Übergang zu AppRunning
  *   ein aktueller Wert versendet.
  * - Auswertung der Kontaktfehlerüberwachung beginnt beim Übergang zu AppStartup, als wenn dort ein Schaltvorgang stattgefunden hätte
  * - Auswertung der Thresholds beginnt beim Übergang zu AppStartup, als wenn dort ein Schaltvorgang stattgefunden hätte
  */
 for(int chno=0;chno<CHANNELCNT;chno++)
 {
  if (ReadChConfigByte(chno, APP_ENACURRMEAS_O) == 1) // Stromerkennung aktiv
  {
   bool RelState = (relay.GetRelState() & (1 << chno)) != 0;
   float IMeas = GetChannelCurrent(chno); // Strom in A
   //=====================
   // Stromwert versenden
   //=====================
   // Die Routine ist so ausgelegt, dass beim Wechsel zu AppRunning (also nach Ablauf der
   // Wartezeit vor Schalten und Objektversenden) Statustelegramme verschickt werden.
   // Eine konfigurierte Sendeperiode für Stromwerttelegramme startet dann zu diesem
   // Zeitpunkt.
   // TODO Eine Art von Telegrammratenbegrenzung für die Stromwerte einführen?
   // Ansonsten kann jeder Kanal bis zu 4 Telegramme je Sekunde erzeugen.
   bool SendStatus = false;
   unsigned short CnfStatTime = ReadChConfigUInt16(chno, APP_CURRTMRSNDVAL_O);
   if (CnfStatTime)
   { // Es ist Status senden nach einer gewissen Zeit konfiguriert
    SendStatus = IniAppRun; // Bei Startup versenden
    if (!(--ChannelStates[chno].CFStatusTime))
    { // Zeit abgelaufen, Status senden
     SendStatus = true;
    }
   }
   unsigned short IConf = ReadChConfigByte(chno, APP_CURRCHGSNDVAL_O);
   // Stromänderung, nach der gesendet wird, in 25mA Inkrementen. 0 bedeutet nicht senden.
   if (IConf)
   {
    if (!SendStatus) // Bei Startup versenden
     SendStatus = IniAppRun;
    float DeltaI = 0.025*IConf;
    if (abs(IMeas - ChannelStates[chno].CFLastSentValue) > DeltaI)
    { // Der Stromwert hat sich deutlich geändert
     SendStatus = true;
    }
   }
   float IObj = IMeas;
   //if (IObj < 0.005) // TODO Funktion aktivieren
   // IObj = 0;
   if (SendStatus && AppObjSendEnabled())
   { // Strommesswert versenden
    if ((ReadChConfigByte(chno, APP_TYPCURRMEAS_O) & APP_TYPCURRMEAS_M) == 5) // Datentyp Strommesswert
    { // 4 Byte Float nach IEEE, die Binärdaten des float können direkt versendet werden
     ChObjectWrite(chno, OBJ_CURRENT, *(int *)&IObj);
    } else {// 2 Byte Counter, Skalierung in mA
     ChObjectWrite(chno, OBJ_CURRENT, int(IObj*1000));
    }
   } else {
    if ((ReadChConfigByte(chno, APP_TYPCURRMEAS_O) & APP_TYPCURRMEAS_M) == 5) // Datentyp Strommesswert
    { // 4 Byte Float nach IEEE, die Binärdaten des float können direkt versendet werden
     ChObjectUpdate(chno, OBJ_CURRENT, *(int *)&IObj);
    } else {// 2 Byte Counter, Skalierung in mA
     ChObjectUpdate(chno, OBJ_CURRENT, int(IObj*1000));
    }
   }
   if (SendStatus)
   {
    ChannelStates[chno].CFStatusTime = CnfStatTime*RMSCURRENTVALUESPERSECOND;
    ChannelStates[chno].CFLastSentValue = IMeas;
   }

   //==================================
   // Grenzwertfunktionen Strommessung
   //==================================
   if ((RelState != ((ChannelStates[chno].CurrFctStates & CFCONTSTATE_M) !=0 )) || IniAppStartup)
   { // Schaltzustand hat sich geändert
    // Verzögerungszeiten der Auswertung neu setzen
    if (RelState)
    {
     ChannelStates[chno].CFContCloseBlanking = max(ReadChConfigByte(chno, APP_CURREVADELAY_O)*RMSCURRENTVALUESPERSECOND, RMSCURRENTVALUESPERSECOND)+1;
     //ChannelStates[chno].CFContFailBlanking = 0;
    } else {
     ChannelStates[chno].CFContCloseBlanking = RMSCURRENTVALUESPERSECOND+1;
     //ChannelStates[chno].CFContFailBlanking = 2*RMSCURRENTVALUESPERSECOND+2;
    }
   }
   if (ReadChConfigByte(chno, APP_CURRTHEN_O) & APP_CURRTHEN_M)
   { // Schwellwerte aktiv
    // Wann sollen die Stromgrenzwerte ausgewertet werden?
    byte EvalCnf = (ReadChConfigByte(chno, APP_CURREVATH_O) & APP_CURREVATH_M) >> APP_CURREVATH_B;
    if ((EvalCnf == 3) ||
      ((ChannelStates[chno].CFContCloseBlanking == 0) &&
        ((((EvalCnf == 1) && !RelState)) || ((EvalCnf == 2) && RelState))))
    {
     OneCurrThresholdFct(chno, IMeas, 0);
     if (ReadChConfigByte(chno, APP_CURRTHEN2_O) & APP_CURRTHEN2_M)
     { // Schwellwert 2 aktiv
      OneCurrThresholdFct(chno, IMeas, 1);
     }
    }
   }
   if (ChannelStates[chno].CFContCloseBlanking)
    ChannelStates[chno].CFContCloseBlanking--;
   //==========================
   // Kontaktfehlerüberwachung
   //==========================
   // Die Kontaktüberwachung überwacht auf Stromfluss bei geöffnetem Kontakt. Wenn mehr als 30mA
   // wird dies über ein Objekt gemeldet.
   // Die Funktionalität ist der Grenzwertfunktion ähnlich und doch ganz anders. Der Hauptunterschied
   // ist die Realisierung der Blanking-Zeit nach dem Ausschalten kombiniert mit einer Filterzeit
   // beim Aktivieren/Deaktivieren des Fehlerzustandes.
   // .CFContFailBlanking enthält hierzu im unteren Nibble einen Zähler für die Blankingzeit, im oberen Nibble
   // die Filterzeit. Die Filterzeiten sind unterschiedlich für Ein und Aus. Blanking und Filter sind so
   // abgestimmt, das genau nach Ende des Blankings ein neu gefiltertes Ergebnis zur Verfügung steht.
   bool ContFailState = ((ChannelStates[chno].CurrFctStates & CFCONTFAILSTATE_M) != 0);
   bool ContOpenCurr = (IMeas > 0.03);
   byte SendCfg = (ReadChConfigByte(chno, APP_CURRCONTFAILMON_O) & APP_CURRCONTFAILMON_M) >> APP_CURRCONTFAILMON_B;
   bool UpdFailState = false;
   byte CFContFBVal = ChannelStates[chno].CFContFailBlanking;
   if (!RelState)
   { // Auswertung nur bei Kontakt offen
    bool OldRelState = ((ChannelStates[chno].CurrFctStates & CFCONTSTATE_M) != 0);
    if (OldRelState || IniAppStartup)
    { // Also wurde gerade ausgeschaltet (oder es ist der erstmalige Durchlauf)
     CFContFBVal = RMSCURRENTVALUESPERSECOND*5/2-1; // Blanking-Zeit 2,5s minus einen Tick
    } else {
     // Es ist immer noch ausgeschaltet
     if ((CFContFBVal & 0xf) != 0)
     {
      CFContFBVal--;
      if ((CFContFBVal & 0xf) == RMSCURRENTVALUESPERSECOND)
      { // 1 Sekunde vor Ablaufen der Blankingzeit wird das Filter gestartet.
       CFContFBVal &= 0xf;
       CFContFBVal |= (RMSCURRENTVALUESPERSECOND+1) << 4; // +1 da unten bereits um eins dekrementiert
      }
      if ((CFContFBVal & 0xf) == 0)
      {
       // Die Blanking-Zeit ist jetzt gerade abgelaufen
       if (SendCfg == 3) // "Sende immer", das Fehlerflag wird immer auch nach Ausschalten des Kontaktes versendet
       {
        UpdFailState = true;
       }
      }
     }
     if (ContFailState != ContOpenCurr)
     { //
      if ((CFContFBVal & 0xf0) != 0)
      {
       CFContFBVal -= 0x10;
       if ((CFContFBVal & 0xf0) == 0)
       {
        // Die Filterzeit ist jetzt gerade abgelaufen
        ContFailState = ContOpenCurr;
        if ((SendCfg == 1) || (SendCfg == 3))// "Sende nach Änderung" oder "Sende immer"
         UpdFailState = true;
       }
      }
     } else { // Der jetzige Messwert entspricht dem gespeicherten Zustand
      CFContFBVal &= 0xf;
      if (ContFailState)
      { // Filterzeit zum Deaktivieren des Fehlerflags: 2,5s
       CFContFBVal |= (RMSCURRENTVALUESPERSECOND*5/2) << 4;
      } else { // Filterzeit zum Aktivieren des Fehlerflags: 0,5s
       CFContFBVal |= (RMSCURRENTVALUESPERSECOND/2) << 4;
      }
     }
    }
    if (UpdFailState)
    {
     ChObjectWrite(chno, OBJ_CONTACTMON, ContFailState ? 1 : 0);
    } else {
     ChObjectUpdate(chno, OBJ_CONTACTMON, ContFailState ? 1 : 0);
    }
    ChannelStates[chno].CurrFctStates &= ~CFCONTFAILSTATE_M;
    if (ContFailState)
    {
     ChannelStates[chno].CurrFctStates |= CFCONTFAILSTATE_M;
    }
// Alte Version
//    bool Chg = false;
//    if ((ChannelStates[chno].CFContFailBlanking & 0xf) <= 1)
//    { // Blanking-Zeit der Überwachung abgelaufen, ab jetzt wird überwacht
//     if (ContFailState)
//     { // Ist bereits aktiv
//      if (IMeas > 0.03)
//      {
//       ChannelStates[chno].CFContFailBlanking &= 0xf;
//      } else {
//       if (ChannelStates[chno].CFContFailBlanking & 0xf0) // Keine Filterzeit gesetzt
//       {
//        ChannelStates[chno].CFContFailBlanking -= 0x10;
//        if (!(ChannelStates[chno].CFContFailBlanking & 0xf0))
//        { // Wartezeit abgelaufen
//         ContFailState = false;
//         Chg = true;
//        }
//       } else {
//        ChannelStates[chno].CFContFailBlanking |= RMSCURRENTVALUESPERSECOND << 5; // Filterzeit für Rücksetzen des Fehlers
//       }
//      }
//     } else { // Ist nicht aktiv
//      if (IMeas > 0.03)
//      {
//       if (ChannelStates[chno].CFContFailBlanking & 0xf0) // Keine Filterzeit gesetzt
//       {
//        ChannelStates[chno].CFContFailBlanking -= 0x10;
//        if (!(ChannelStates[chno].CFContFailBlanking & 0xf0))
//        { // Wartezeit abgelaufen
//         NewFailState = true;
//         Chg = true;
//        }
//       } else {
//        ChannelStates[chno].CFContFailBlanking |= RMSCURRENTVALUESPERSECOND << 4; // Filterzeit für Setzen des Fehlers
//       }
//      } else {
//       ChannelStates[chno].CFContFailBlanking &= 0xf; // laufende Filterzeit löschen
//      }
//     }
//    }
//    //bool Chg = (ContFailState != ((ChannelStates[chno].CurrFctStates & CFCONTFAILSTATE_M) != 0));
//    byte SendCfg = (ReadChConfigByte(chno, APP_CURRCONTFAILMON_O) & APP_CURRCONTFAILMON_M) >> APP_CURRCONTFAILMON_B;
//    if (Chg) //(((SendCfg == 1) && Chg) || ((SendCfg == 3) && (UpdFailState || Chg)))
//    {
//     objectWrite(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+OBJ_CONTACTMON, ContFailState ? 1 : 0);
//    } else
//     if (Chg)
//     {
//      objectUpdate(OFSCHANNELOBJECTS+chno*SPACINGCHANNELOBJECTS+OBJ_CONTACTMON, ContFailState ? 1 : 0);
//     }
//    ChannelStates[chno].CurrFctStates &= ~CFCONTFAILSTATE_M;
//    if (ContFailState)
//    {
//     ChannelStates[chno].CurrFctStates |= CFCONTFAILSTATE_M;
//    }
   } else {
    // Wenn Kontakt geschlossen, eine evtl laufende Filterzeit immer löschen
    CFContFBVal = 0;
   }
   ChannelStates[chno].CFContFailBlanking = CFContFBVal;

   // Und ganz zum Schluss noch den Relaiszustand abspeichern
   ChannelStates[chno].CurrFctStates &= ~CFCONTSTATE_M;
   if (RelState)
    ChannelStates[chno].CurrFctStates |= CFCONTSTATE_M;
  }
 }
 if (AppObjSendEnabled())
  ChannelStates[0].CurrFctStates |= CFINIDONE2_M; // Init-Done
 ChannelStates[0].CurrFctStates |= CFINIDONE1_M;
#endif
}

void Appl::ProcAliveObject(unsigned referenceTime)
{
 unsigned AliveTime = ReadConfigUInt16(APP_SENDALIVE_O);
 if (AliveTime)
  if (AppObjSendEnabled())
  {
   if ((signed int)(referenceTime - AliveTargetTime) > 0)
   {
    objectWrite(OBJ_OPERATIONAL, 1);
    AliveTargetTime = referenceTime + AliveTime*1000;
   }
  }
}

// Aufruf wenn alte Zeit ungleich aktuelle Zeit
void Appl::TimeFunctionsTimeRelated(unsigned referenceTime)
{
 ProcAliveObject(referenceTime);
 TStateAndTrigger trigger = {false, false, false, false};
 for(int chno=0;chno<CHANNELCNT;chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   if (OneTimeFunctionsTimeRelated(trigger, chno, referenceTime))
   {
    // Eine Neuevaluierung hat definitiv stattgefunden, also Ablauf bis ganz unten.
    if (trigger.SwOnOff)
    {
     ChannelStates[chno].IntSwitchStates |= STAFTERTIMFCT_M;
    } else {
     ChannelStates[chno].IntSwitchStates &= ~STAFTERTIMFCT_M;
    }
   }
   trigger.OnOff = ((ChannelStates[chno].IntSwitchStates & STAFTERTIMFCT_M) != 0);
   PermanentOnFunction(trigger, -1, chno);
   ProcessChSafety(trigger, 0, -1, chno);
   // Auswertung von .Evaluated?
   UpdateStatusObjekt(chno, trigger); // wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein Vergleich mit dem alten Zustand stattfinden
   ChannelTrigger2RelaySwitch(chno, trigger); // Berücksichtigt evtl Invertierung
  }
 }
}

// Handbedienung -> Könnte vor oder nach Safety ausgeführt werden.
// Ändert den Relaiszustand (und auch das Rückmeldeobjekt) bis zur nächsten Evaluierung des Ausgangszustands.
// Der gespeicherte Zustand nach der Zeitfunktion wird nicht geändert.
// Handbedienung schaltet Ausgang um, bedient sich dabei direkt dem Relaiszustand, nicht dem Kanalzustand.
// Kanalzustand wird nicht geändert.
// Das Statusobjekt jedoch schon, dieses muss seine Info also vom Relaiszustand (+evtl Invertierung) erhalten!
void Appl::ManualControl(unsigned ButtonDownEvents)
{
 TStateAndTrigger trigger = {false, false, false, false};
 // Ist auch im Heizungsmodus aktiv. Ist damit die einzige Funktionalität, die dann aktiv ist.
 unsigned mask = 1;
 for (int chno=0;chno<CHANNELCNT;chno++)
 {
  if (ButtonDownEvents & mask)
  {
   // Der Umschaltversuch ist definitiv da, also Ablauf bis ganz unten.
   bool State = GetSwitchStatus(chno);
   trigger.Sw = true;
   trigger.SwOnOff = not State; // Es soll ja umgeschaltet werden
   trigger.OnOff = trigger.SwOnOff;
#ifdef EVALSAFETYAFTERMANU
   ProcessChSafety(trigger, 0, -1, chno);
#endif
   // Auswertung von .Evaluated?
   UpdateStatusObjekt(chno, trigger); // wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein Vergleich mit dem alten Zustand stattfinden
   ChannelTrigger2RelaySwitch(chno, trigger); // Berücksichtigt evtl Invertierung
  }
  mask <<= 1;
 }
}

// Wird nach Update der globalen Sicherheitsfunktionen aufgerufen, sowohl bjekt-, als auch zeitbasiert.
// Das Zwangsführungsobjekt wird ganz normal im ProcessChannelObj Kontext bearbeitet, also nicht hier
void Appl::ProcessSafetyChanges(unsigned SafetyChanges)
{
 TStateAndTrigger trigger;
 for(int chno=0;chno<CHANNELCNT;chno++)
 {
  if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
   trigger = {false, false, false, false};
   trigger.OnOff = ((ChannelStates[chno].IntSwitchStates & STAFTERTIMFCT_M) != 0);
   PermanentOnFunction(trigger, -1, chno);
   ProcessChSafety(trigger, SafetyChanges, -1, chno);
   // Auswertung von .Evaluated?
   UpdateStatusObjekt(chno, trigger); // wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein Vergleich mit dem alten Zustand stattfinden
   ChannelTrigger2RelaySwitch(chno, trigger); // Berücksichtigt evtl Invertierung
  }
 }
}

bool Appl::OneGlobalSafetyTimeRelated(unsigned SafetyConf, unsigned SafetyConfTime, unsigned SafetyNo, unsigned referenceTime)
{
 if ((SafetyConf <= 1) && SafetyConfTime)
 { // Safety Funktion freigeschaltet, Zeitabhängigkeit aktiv
  if ((signed int)(referenceTime - ActuatorSafetyTripTime[SafetyNo]) > 0) // Zeit abgelaufen
  { // Auslösen der Sicherheitsfunktion
   unsigned Mask = 1 << SafetyNo;
   if ((ActuatorSafety & Mask) == 0)
   { // War vorher nicht aktiv
    ActuatorSafety |= Mask;
    return true;
   }
  }
 }
 return false;
}

// Aufruf wenn alte Zeit ungleich aktuelle Zeit
void Appl::GlobalSafetyTimeRelated(unsigned referenceTime)
{
 unsigned SafetyChanges = 0;
 if (OneGlobalSafetyTimeRelated(ReadConfigByte(APP_SAFPRIO1FKT_O),
   ReadConfigUInt16(APP_SAFPRIO1TIM_O), 0, referenceTime))
  SafetyChanges |= 1;
 if (OneGlobalSafetyTimeRelated(ReadConfigByte(APP_SAFPRIO2FKT_O),
   ReadConfigUInt16(APP_SAFPRIO2TIM_O),1, referenceTime))
  SafetyChanges |= 2;
 if (OneGlobalSafetyTimeRelated(ReadConfigByte(APP_SAFPRIO3FKT_O),
   ReadConfigUInt16(APP_SAFPRIO3TIM_O),2, referenceTime))
  SafetyChanges |= 4;
 if (SafetyChanges)
 {
  ProcessSafetyChanges(SafetyChanges);
 }
}

// Funktion wird aufgerufen, wenn ein Objekt für diesen Sicherheits"kanal" empfangen wurde.
// Die Funktion reagiert dann auf das Objekt.
// objvalue: Empfangener Objektwert
// SafetyConf: Konfiguration der Safety-Funktion 1, 2 oder 3
// SafetyConfTime: Konfiguration der Überwachungszeit in Sekunden
// SafetyNo: Nummer der Sicherheitsfunktion-1: Also 0 für die Prio1, 1 für Prio2, 2 für Prio3
// Rückgabewert: Wahr wenn sich der Zustand dieser Sicherheit geändert hat
bool Appl::OneGlobalSafetyObjRelated(unsigned objvalue, unsigned SafetyConf, unsigned SafetyConfTime, unsigned SafetyNo, unsigned referenceTime)
{
 if (SafetyConf <= 1)
 { // Safety Funktion freigeschaltet
  unsigned Mask = 1 << SafetyNo;
  if ((objvalue & 1) == SafetyConf)
  { // Auslösen der Sicherheitsfunktion
   if ((ActuatorSafety & Mask) == 0)
   { // War vorher nicht aktiv
    ActuatorSafety |= Mask;
    return true;
   }
  } else {
   // Rücksetzen der Sicherheitsfunktion und evtl Rücksetzen der Zeitüberwachung
   if (SafetyConfTime)
   { // Es ist eine Überwachungszeit gewählt
    ActuatorSafetyTripTime[SafetyNo] = referenceTime + SafetyConfTime*1000;
   }
   if (ActuatorSafety & Mask)
   { // War vorher aktiv
    ActuatorSafety &= ~Mask;
    return true;
   }
  }
 }
 return false;
}

void Appl::GlobalSafetyObjRelated(int obj, unsigned referenceTime)
{
 unsigned SafetyChanges = 0;
 switch (obj)
 {
 case OBJ_SAFETYPRIO1:
  if (OneGlobalSafetyObjRelated(objectRead(obj), ReadConfigByte(APP_SAFPRIO1FKT_O),
    ReadConfigUInt16(APP_SAFPRIO1TIM_O), 0, referenceTime))
   SafetyChanges |= 1;
  break;
 case OBJ_SAFETYPRIO2:
  if (OneGlobalSafetyObjRelated(objectRead(obj), ReadConfigByte(APP_SAFPRIO2FKT_O),
    ReadConfigUInt16(APP_SAFPRIO2TIM_O), 1, referenceTime))
   SafetyChanges |= 2;
  break;
 case OBJ_SAFETYPRIO3:
  if (OneGlobalSafetyObjRelated(objectRead(obj), ReadConfigByte(APP_SAFPRIO3FKT_O),
    ReadConfigUInt16(APP_SAFPRIO3TIM_O), 2, referenceTime))
   SafetyChanges |= 4;
  break;
 }
 if (SafetyChanges)
 {
  ProcessSafetyChanges(SafetyChanges);
 }
}

void Appl::ProcessChannelObj(int chno, int objno, unsigned referenceTime)
{
 if ((ReadChConfigByte(chno, APP_OPMODE_O) & APP_OPMODE_M) == 1) // Schaltaktor
  {
  TStateAndTrigger trigger;
  trigger = ProcessSwitchObj(objno, chno);
  // Der Rest ist in eine Funktion ausgelagert, damit auch die Strommessung ein Trigger-Objekt generieren kann
  PostProcessSingleSwitchObject(chno, trigger, objno, referenceTime);
 }
}

void Appl::objectUpdated(int objno, unsigned referenceTime)
{
 if (objno >= OFSGENERALOBJECTS)
 {
  if (objno < OFSCHANNELOBJECTS)
  {
   // General objects
   GlobalSafetyObjRelated(objno, referenceTime);
  } else {
   // Channel specific objects
   objno -= OFSCHANNELOBJECTS;
   int chno = objno/SPACINGCHANNELOBJECTS;
   objno -= chno*SPACINGCHANNELOBJECTS;
   if (chno < CHANNELCNT)
   {
    ProcessChannelObj(chno, objno, referenceTime);
   }
  }
 } else {
  // Da die OFSGENERALOBJECTS bei 0 anfangen, währen das hier negative Objektnummern -> gar nichts machen
 }
}

