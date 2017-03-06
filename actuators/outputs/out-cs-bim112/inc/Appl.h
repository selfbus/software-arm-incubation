/*
 *  Appl.h - Application logic
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef APPL_H_
#define APPL_H_

/* Trigger - TStateAndTrigger
 * In den verschiedenen Funktionen des Ausgangskanals wird immer mit "Triggern" für die
 * Neubestimmung des Ausgangszustands gearbeitet. Trigger sind nichts anderes als eine
 * Darstellung von Schaltaufträgen. Zusätzlich ist in der Struktur TStateAndTrigger
 * auch noch ein Flag Evaluated enthalten, dass vereinfacht gesagt aussagt: Es wurde
 * bereits ein Objekt empfangen/eine Aktion festgestellt, die eine Neuberechnung des
 * Ausgangszustandes notwendig gemacht hat - aber nicht zwingend zu einem Schaltauftrag
 * geführt hat.
 * Diese Architektur ist für zwei Funktionen des Kanals notwendig:
 * - Es gibt eine Option "Statustelegramme immer senden", die ein Statustelegramm fordert,
 *   wenn der Zustand des Ausgangs neuberechnet wurde. Selbst wenn am Ende der Berechnung
 *   keine Änderung der Kontaktlage erfolgt.
 * - Die Handbedienung erfolgt ganz ähnlich: Sie schaltet den Ausgang um. Dies spiegelt sich
 *   auch im Statusobjekt wider. Jedoch stellt jede Neuberechnung des Ausgangszustandes den
 *   Buszustand wieder her. (Die Handbedienung kann wahlweise nach oder vor den Sicherheits-
 *   funktionen berechnet werden, d.h. im letzteren Fall kann eine Handbedienung durch
 *   Sicherheitsfkt gesperrt sein.)
 * Nur an wenigen Stellen der Abarbeitung wird tatsächlich auf einen abgespeicherten Schaltzustand
 * zurückgegriffen.
 */
typedef struct
{
 bool Sw;        // Es soll eine Schaltaktion stattfinden
 bool SwOnOff;   // Schaltaktion an oder aus.
 bool Evaluated; // Wahr wenn eine Operation ein Schaltbefehl erstellt hat. Dieser Schaltbefehl kann durch
                 // nachfolgende Auswertungen verändert oder auch gelöscht werden.
 bool OnOff;     // Wird kurzzeitig im Rahmen der Dauer-Ein Funktion zur Darstellung des abgespeicherten Zustands genutzt.
} TStateAndTrigger;

enum class TimeFctStates : byte {Idle, StairOn, StairWarn1, StairWarn2, StairBack2On, DelayOn, DelayOff, BlinkOn, BlinkOff};

#define STPREPRESET2_M  0x03 // 2 bit: 0: ausgeschaltet, 1: eingeschaltet, 3: kein gültiger Zustand gespeichert
#define STPREPRESET2_ON    1
#define STSWITCHOBJ_M   0x04 // letzter Zustand des Switch-Objekts
#define STSWITCHOBJ_O      2
#define STPREBLINK_M    0x08 // Zustand vor dem Blinken
#define STPREBLINK_O       3
#define STAFTERTIMFCT_M 0x10 // Zustand nach der Zeitfunktion
#define STAFTERTIMFCT_O    4
#define STPERMANENTON_M 0x20 // Permanent-On Funktion Ein/Aus
#define STPERMANENTON_O    5
#define STDISTIMEFCT_M  0x40 // Disable Time-Function Ein/Aus
#define STDISTIMEFCT_O     6
typedef struct
{
 byte IntSwitchStates; // 7bit, gespeicherte Schaltzustände verschiedener Funktionsblöcke. Enhalten sind:
 // PrePreset2State, 2bit
 // SwitchObject, 1bit
 // PreBlinkState, 1bit
 // ChannelStateAfterTimeFunction, 1bit
 // DisableTimeFunction, 1bit
 // PermanentOn, 1bit
 byte Preset; // unteres Nibble: Preset1, oberes Nibble: Preset2
 // 0: aus, 1: ein, 4: Zustand vor Preset2 wiederherstellen, 6: Konf wiederherstellen - Preset2 kennt nur 0 & 1
 byte BlinkDnCnt; // 1 Byte
 byte ScenesVal; // Werte für die 5 möglichen Szenen, d.h. 5 bit belegt
 //==> unsigned short Threshold1; // Da dieser Wert über den Bus geändert werden kann, wird er im Objekt selbst gespeichert
 //==> unsigned short OldTreshVal; // Der Objektwert Threshold wird direkt aus diesem gelesen, eine Ablage hier ist unnötig
 byte OldThreshState; // 2 bit belegt: 0: noch keine Thresholdüberschreitung festgestellt, 1: oberer Threshold, 2: unterer Threshold
 byte LogicObjVals; // 4 bit belegt
 // Bit 0 steht für den Objektwert Logik 1, Bit 1 für eine aktuelle Änderung Logik 1,
 // Bit 2 für den Objektwert Logik 2, Bit 3 für eine Änderung Logik 2
 byte Safety; // 8 bit belegt
 // Bit 7..4: Maske der freigeschalteten Sicherheitsoptionen für diesen Kanal
 // Bit 3..0: Aktueller Aktivierungsstand der Sicherheitsoptionen, soweit für diesen Kanal freigeschaltet
 byte ForcedPos; // 2 bit belegt
 // Bit 1: Bei 1 Schalthandlungen blockieren, ansonsten: Bit 0: Zielschaltzustand
 // Wenn Zielzustand variabel, wird dieses Bit auch zwischendurch aktualisiert
 unsigned int TFTargetTime; // Zielzeit der nächsten Aktion bei einer Zeitfunktion
 TimeFctStates TFState; // 1 Byte
 //==> unsigned short DurationStaircase; // Die Zeitdauer der Treppenlichtfunktion in Sekunden (wird im Kommunikationsobjekt selbst gespeichert)

 byte CurrFctStates; // 6 bit belegt, zusammengefasst mehrere gespeicherte Zustände für die Strommessfunktion
 // OldContactState, 1bit, gespeicherter Kontaktzustand, um Änderungen zu verfolgen
 // ThresholdStates 1/2, 2*2 Bit für die Zustände der Stromschwellwerte
 // OldContactFailState, 1bit, Zustand der Kontaktfehlerüberwachung
 byte CFContFailBlanking; // 2 Zähler für die Kontaktfehlerüberwachung:
 // oberes Nibble: Filter, unteres Nibble: Blanking-Zeit
 unsigned int CFStatusTime; // Zähler für die gewählte Frequenz "Stromwert senden"
 unsigned short CFContCloseBlanking; // Wartezeit nach Kontakt schließen bis zur Grenzwertüberwachung
 float CFLastSentValue; // letzter gesendeter Stromwert, um bei Änderungen entscheiden zu können, wann neu gesendet werden muss
} TChannelState;
// 25 Bytes (26 mit Alignment)

#define CFTHRESHSTATE1_M 0x03
#define CFTHRESHSTATE1_O 0
#define CFTHRESHSTATE2_M 0x0C
#define CFTHRESHSTATE2_O 2
#define CFCONTFAILSTATE_M 0x10
#define CFCONTFAILSTATE_O 4
#define CFCONTSTATE_M 0x20
#define CFCONTSTATE_O 5
#define CFINIDONE1_M 0x40 // Ini1: Beim Übergang zu "Startup"
#define CFINIDONE1_O 6
#define CFINIDONE2_M 0x80 // Ini2: Beim Übergang zu "Running"
#define CFINIDONE2_O 7

#define STOREAPPL_DOWNLOAD 0x80
#define STOREAPPL_BUSVFAIL 0x81
#define RECALLAPPL_STARTUP 0
#define RECALLAPPL_OTHER   1

/*
 * Liest die Wartezeit bei Systemstart für Schaltaktionen und Objektsenden.
 * Das Ergebnis ist in Sekunden.
 */
unsigned ReadStartDelayObjSendAndSwitching(void);

class Appl;

extern Appl appl;

class Appl
{
public:
 Appl(void);
 /*
  * Setzt einige Startzeitwerte für Safety und regelmäßige Telegramme
  */
 void StartupGlobSafetyStartTime(unsigned referenceTime);

 /*
  * ModifyAfterDownload
  * Funktion, die auf die ChannelStates eines Kanals angewendet wird, bevor diese in das Flash abgespeichert werden.
  * Einerseits gibt es Konfigurationsoptionen, die dann verschiedene Zustände beeinflussen.
  * Andererseits sollen bei einem umkonfigurierten Kanal die Initialzustände aller Optionen (bis auf Relaiszustand) wieder
  * hergestellt werden. Dies wird bereits in StoreApplData vorgenommen.
  * In der Praxis bedeutet dies, dass ein Kanal, der nicht geändert wurde, ähnlich wie bei einem Busspannungsausfall,
  * anschließend einfach "weiterläuft", mit geringsten Auswirkungen des Downloadvorgangs.
  */
 void ModifyChStateAfterDownload(int chno);

 /*
  * ModifyBeforeReset
  * Funktion, die auf die ChannelStates eines Kanals angewendet wird, bevor diese in das Flash abgespeichert werden.
  * Macht im Moment nix.
  */
 void ModifyChStateBeforeReset(int chno);

 /*
  * ModifyAfterBusVoltageRecovery
  * Funktion, die auf die ChannelStates eines Kanals angewendet wird, nachdem diese bei Systemstart aus dem Flash geladen worden sind.
  * Je nach gewählter Konfiguration werden verschiedene Zustände beeinflusst.
  */
 void ModifyChStateAfterBusVoltageRecovery(int chno);

 /*
  * Setzt die Zustände eines Kanals auf Default-Werte.
  */
 void SetIniChannelState(int chno);

 void StoreChannelState(int chno, byte* ptr, unsigned referenceTime);

 void RecallChannelState(int chno, byte* ptr, unsigned referenceTime);

 void StoreApplData(int type);

 void RecallAppData(int type);

 /*
  * Aktualisiert den Zustand aller KanalZustandsobjekte (Statusobjekt). Wenn die
  * Applikation im Zustand AppRunning ist, werden die Objekte auch verschickt.
  */
 void UpdateAllStatusObjects(void);

 /*
  * Zustand des Kanals, noch vor einer evtl. gewählten Ausgangsinvertierung
  * Holt sein Info aus dem Relaiszustand und rechnet rück.
  */
 bool GetSwitchStatus(int chno);

 /*
  * Diese Funktion muss bei Empfang eines Objektes aufgerufen werden.
  * referenceTime ist die aktuelle Zeit bei Aufruf, sie dient dazu, dass
  * intern alle Funktionen von einem konsistenten Zeitpunkt ausgehen.
  */
 void objectUpdated(int objno, unsigned referenceTime);

 /*
  * Die Bearbeitung der Zeitfunktion gliedert sich in zwei verschiedene Funktionen:
  * - Die Reaktion auf Schaltobjekte etc
  * - Die Reaktion abhängig von abgelaufenen Zeiten
  * Dies ist die Bearbeitung für eine Zeitfunktion in Abhängigkeit der abgelaufenen Zeit.
  * Aufgerufen wird sie immer, wenn sich die systemTime ändert. Die Routine muss nicht
  * jede Millisekunde aufgerufen werden. Ein bischen Jitter schadet nicht...
  */
 void TimeFunctionsTimeRelated(unsigned referenceTime);

 /*
  * Die Funktionalität der globalen Sicherheit, abhängig von der aktuellen Zeit
  */
 void GlobalSafetyTimeRelated(unsigned referenceTime);

 /*
  * Auch die Bearbeitung der Strommessfunktionen gliedert sich in zwei verschiedene Funktionen:
  * - Die Reaktion auf empfangene Objekte
  * - Die Reaktion auf den gemessenen Strom
  * Dies ist die Bearbeitung der Strommessfunktionen in Abhängigkeit des gemessenen Stroms
  */
 void CurrentFunctions(unsigned referenceTime);

 /*
  * Hier wird mal die Handbedienungsfunktionalität draus.
  */
 void ManualControl(unsigned ButtonDownEvents);

 /*
  * Ermittelt die Daten für "Kanal schalten bei Busspannungsausfall" und startet das Relaisschalten
  */
 void ConfigRelayStart(void);

 /*
  * Setzt die Zwangsfunktion nach Busspannungswiederkehr
  * Der konfigurierte Wert nach Busspannungswiederkehr wird gesetzt und evtl Schaltaufträge an die
  * Relay-Unit abgesetzt.
  */
 void StartupSafetyAndForcedPos(void);

 /*
  * Muss aufgerufen werden, wenn die Applikation in den Running-Zustand versetzt wird
  * (Also z.B. nach Busspannungswiederkehr). Schaltet dann entsprechend konfigurierte
  * Kanäle in den Startzustand.
  */
 void InitialChannelSwitch(unsigned referenceTime);

 /*
  * Liest das Telegrammratenlimit
  */
 int ReadTelRateLimit(void);

protected:
 TChannelState ChannelStates[CHANNELCNT];
 short unsigned ActuatorSafety; // Speicher für den aktuellen Auslösezustand von Safety 1..3 in Bit 0..2
 short unsigned RestartSkipBvrMask;
 // Maske der Kanäle, bei denen bei einem Übergang NoApp->App nicht die Sicherheit neu gestartet wird.
 // Dies sind Kanäle, die durch den Download nicht verändert worden sind.
 unsigned int ActuatorSafetyTripTime[3];
 unsigned int AliveTargetTime;

 /*
  * Realisiert eine Stromschwellwertfunktion
  */
 void OneCurrThresholdFct(int chno, float IMeas, int fctno);

 /*
  * Aktualisiert den Zustand eines StatusObjekts anhand des Triggers.
  * Dabei wird die Konfiguration für Status-Objekte der Applikation berücksichtigt
  * (Immer schicken, nur bei Änderung, nie).
  * Nur wenn die Applikation im Zustand AppRunning ist, wird ein Objekt auch verschickt.
  * Wird VOR der Beauftragung der Relay-Unit aufgerufen, dann kann noch einfach ein
  * Vergleich mit dem alten Zustand stattfinden.
  */
 void UpdateStatusObjekt(int chno, TStateAndTrigger &trigger);

 /*
  * Leitet aus dem Trigger den Schaltauftrag für die Relay-Klasse ab.
  * Dabei wird eine evtl. Invertierung des Ausgangs berücksichtigt.
  */
 void ChannelTrigger2RelaySwitch(int chno, TStateAndTrigger &trigger);

 /*
  * Diese Routine führt die komplette Bearbeitung nach dem SwitchObject durch, d.h.
  * Logik- & Zeitfunktionen, Permanent On (Teil der Zeitfkt) und Sicherheit.
  * Letztendlich werden Schaltaufträge an die Relais-Unit übergeben.
  */
 void PostProcessSingleSwitchObject(int chno, TStateAndTrigger &trigger, int objno, unsigned referenceTime);

 /*
  * Verarbeitet die Schaltobjekte, wird indirekt über objectUpdated() aufgerufen.
  * Zu den Schaltobjekten gehören:
  * - Switch
  * - Preset
  * - Scene
  * - Threshold
  * - Schaltaufträge erzeugt von der Strommessung werden sehr ähnlich wie Schaltobjekte weiterverarbeitet,
  *   sind aber keine "Objekte"
  * In dieser Routine werden auch die Änderungen durch Objekte an den Schaltfunktionen bearbeitet
  * (Szenen speichern, Presets ändern etc.), obwohl diese Objekte keine Schaltaufträge generieren können.
  */
 TStateAndTrigger ProcessSwitchObj(int objno, int chno);

 /*
  * Bearbeitet eine Logikfunktion innerhalb eines Kanals
  * Diese Routine kann über trigger übergebene Schaltaufträge ändern oder auch löschen, je
  * nach dem wie die Logikfunktion konfiguriert wurde.
  * Zusätzlich bearbeitet diese Routine die empfangenen Logikobjekte, obwohl diese
  * direkt keine Schaltaufträge generieren können.
  */
 void OneLogicFunction(byte LogicNo, TStateAndTrigger &trigger, int chno);

 /*
  * Bearbeitet die beiden Logikfunktionen für einen Kanal.
  * Diese Routine kann über trigger übergebene Schaltaufträge ändern oder auch löschen, je
  * nach dem wie die Logikfunktionen konfiguriert wurden.
  * Zusätzlich bearbeitet diese Routine die empfangenen Logikobjekte, obwohl diese
  * direkt keine Schaltaufträge generieren können.
  */
 void LogicFunction(TStateAndTrigger &trigger, int objno, int chno);

  /*
  * Eine interne Funktion der Abarbeitung der Zeitfunktionalität
  */
 bool OneTimeFunctionsTimeRelated(TStateAndTrigger &trigger, int chno, unsigned referenceTime);

 /*
  * Die Bearbeitung der Zeitfunktion gliedert sich in zwei verschiedene Funktionen:
  * - Die Reaktion auf Schaltobjekte etc
  * - Die Reaktion abhängig von abgelaufenen Zeiten
  * Dies ist die Bearbeitung für eine Zeitfunktion in Abhängigkeit ankommender Objekte.
  */
 void OneTimeFunctionsObjRelated(TStateAndTrigger &trigger, int objno, int chno, unsigned referenceTime);

 /*
  * Interne Funktion, um die Blinkfunktion zu deaktivieren
  */
 void BlinkDeactivate(TStateAndTrigger &trigger, int chno);

 /*
  * Interne Funktion, um das "Alive"/"In Operation" Telgramm zu erzeugen
  */
 void ProcAliveObject(unsigned referenceTime);

 byte IniValueThresholdState(int chno);
 void ProcessThresholds(int chno, TStateAndTrigger &Trigger);

 /*
  * Die Dauer-Ein Funktion unterdrückt, wenn aktiviert, ankommende Schaltaufträge. Wird
  * sie selber aktiviert oder deaktiviert, erzeugt sie die entsprechenden Schaltaufträge.
  * Bei Deaktivierung kann es sogar notwendig sein, die Treppenlichtfunktion neu zu starten.
  * In diesem Fall liefert diese Funktion dann "true" zurück, ansonsten false.
  * Die aufrufende Funktion ProcessChannelObj reagiert dann entsprechend.
  */
 bool PermanentOnFunction(TStateAndTrigger &trigger, int objno, int chno);

 /*
  * Zum Funktionsblock "Sicherheit" gehören sowohl kanalübergreifende Funktionen, wie auch
  * kanalspezifische (Zwangsstellung). Die Sicherheitsfunktionen werden auf der Basis von
  * Änderungen verarbeitet. In SafetyChanges steht Bit 0 für Änderungen an Safety1, Bit 1 Safety2,
  * Bit 2 Safety3. Die Routine bestimmt zusätzlich noch evtl Änderungen der Zwangsstellung
  * und berechnet daraus die notwendigen Änderungen des Ausgangszustands.
  */
 void ProcessChSafety(TStateAndTrigger &trigger, unsigned SafetyChanges, int objno, int chno);

 /*
  * Diese Funktion wird von ProcessChSafety aufgerufen und berechnet eine Sicherheitsfunktion.
  * Ein wichtiger Rückgabewert ist, ob die nächste Prioritätsstufe neu berechnet werden muss.
  * Dies ist immer dann der Fall, wenn eine höherpriore Sicherheitsfunktion deaktiviert wurde.
  */
 bool OneSafetyFunction(TStateAndTrigger &trigger, unsigned SafetyChanges, unsigned Mask, int ConfAddr, int chno, bool &EvaluateNextPrio);

 /*
  * Interne Funktion, die von GlobalSafetyTimeRelated oder -ObjRelated aufgerufen wird.
  * Das Zwangsführungsobjekt wird ganz normal im ProcessChannelObj Kontext bearbeitet, also nicht hier
  */
 void ProcessSafetyChanges(unsigned SafetyChanges);

 /*
  * Eine Funktion der globalen Sicherheit.
  */
 bool OneGlobalSafetyTimeRelated(unsigned SafetyConf, unsigned SafetyConfTime, unsigned SafetyNo, unsigned referenceTime);

 // Funktion wird aufgerufen, wenn ein Objekt für diesen Sicherheits"kanal" empfangen wurde.
 // Die Funktion reagiert dann auf das Objekt.
 // objvalue: Empfangener Objektwert
 // SafetyConf: Konfiguration der Safety-Funktion 1, 2 oder 3
 // SafetyConfTime: Konfiguration der Überwachungszeit in Sekunden
 // SafetyNo: Nummer der Sicherheitsfunktion-1: Also 0 für die Prio1, 1 für Prio2, 2 für Prio3
 // Rückgabewert: Wahr wenn sich der Zustand dieser Sicherheit geändert hat
 bool OneGlobalSafetyObjRelated(unsigned objvalue, unsigned SafetyConf, unsigned SafetyConfTime, unsigned SafetyNo, unsigned referenceTime);

 /*
  * Die Funktionalität der globalen Sicherheit, abhängig von empfangenen Objekten
  */
 void GlobalSafetyObjRelated(int obj, unsigned referenceTime);

 /*
  * Interne Funktion, wird zur Verarbeitung von Kanalobjekten aufgerufen
  */
 void ProcessChannelObj(int chno, int objno, unsigned referenceTime);
};

// Besonderheit: Die Adresse darf beliebig sein, daher auch ungerade
inline unsigned short PtrRdUint16(void *ptr)
{
 return *(byte*)ptr | *((byte*)ptr+1) << 8;
}

// Besonderheit: Die Adresse darf beliebig sein, daher auch ungerade
inline unsigned short PtrRdUint32(void *ptr)
{
 return *(byte*)ptr | *((byte*)ptr+1) << 8 | *((byte*)ptr+2) << 16 | *((byte*)ptr+3) << 24;
}

// Besonderheit: Die Adresse darf beliebig sein, daher auch ungerade
inline void PtrWrUint16(void *ptr, unsigned short val)
{
 byte *bptr;
 bptr = (byte*)ptr;
 *bptr++ = val & 0xff;
 val >>= 8;
 *bptr++ = val & 0xff;
}

// Besonderheit: Die Adresse darf beliebig sein, daher auch ungerade
inline void PtrWrUint32(void *ptr, unsigned val)
{
 byte *bptr;
 bptr = (byte*)ptr;
 *bptr++ = val & 0xff;
 val >>= 8;
 *bptr++ = val & 0xff;
 val >>= 8;
 *bptr++ = val & 0xff;
 val >>= 8;
 *bptr++ = val & 0xff;
}


#endif /* APPL_H_ */
