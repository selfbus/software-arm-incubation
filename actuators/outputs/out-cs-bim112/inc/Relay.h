/*
 *  Relay.h - Switching queue, energy management, relay timing
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef RELAY_H_
#define RELAY_H_

#include <config.h>

//#define RELAYUSEISR // Muss gesetzt werden, wenn DoSwitching() innerhalb einer ISR aufgerufen wird.
#define RELAYUSEDISCRETETIMING // Wenn DoSwitching() nur in festen Zeitabständen mit Vielfachen von 1ms aufgerufen wird
// Bis jetzt ist nur diese Kombination getestet.

#define RELAYPOSTDELAY1 5
#define RELAYPOSTDELAY2 5

#define RELAYBUFLEN (CHANNELCNT+1)

#ifdef RELAYUSEISR
#define RelTmr timer32_0
#define RelTmrIRQn TIMER_32_0_IRQn
#endif

/*
 * Die Hauptaufgabe dieser Relais-Routinen ist die Verwaltung einer Besonderheit:
 * Der Schaltstrom kann nicht direkt dem Bus entnommen werden, dies wäre eine
 * zu große Belastung. Die maximale Stromaufnahme aus dem Bus ist nach dem Standard
 * auf 10mA pro Gerät beschränkt, statisch wie auch transient(!). Die Schaltenergie
 * wird daher kapazitiv im Gerät gespeichert. Treten nun viele Schaltvorgänge
 * nacheinander auf, kann es sein, dass nicht mehr genug Energie in den Kondensatoren
 * gespeichert ist. Dann müssen die Schaltaufträge verzögert werden, bis die
 * Kondensatoren sich wieder ausreichend aufgeladen haben.
 * Diese Routinen übernehmen diese Aufgabe, sie überwachen den Ladungszustand der
 * "Bulkkapazität" und führen Schaltvorgänge aus, wenn möglich. Andernfalls werden
 * sie gespeichert und später ausgeführt.
 * Die aufrufenden Routinen brauchen also diese Sonderfälle nicht berücksichtigen,
 * für sie sieht es aus, als wenn Schaltvorgänge immer sofort ausgeführt werden können.
 */
typedef struct
{
 unsigned short Mask;
 unsigned short Bits;
} TRelBuffer;

enum class RelOperatingStates : byte
{
 Disable,        // Transienter Zustand nach Start
 MeasMode,     // Durchführen des Vermessungspulses sobald genug Railspannung verfügbar
 Operating,    // Normalbetrieb (es wird erst nach "Operating" geschaltet, wenn die Mindestladung vorhanden ist!)
 BusVFail,     // Bearbeitung eines BrownOut
 EndState      // Stopzustand
};

enum class RelSubStates : byte
{
 Idle,   // Idle Zustand
 Pulse,  // Relais werden gerade bestromt
 Pulse2, // Zustand für evtl weitere Phase mit anderem Dutycycle
 Delay,  // Wartezustand nach dem Pulse bis zum Vermessungszeitpunkt (nur beim Systemstart benutzt)
 Delay2  // Wartezeit bis zum nächsten möglichen Relaisansteuerpuls
};

class Relay;

extern Relay relay;

class Relay
{
public:

 Relay(void);
 /*
  * Mit dieser Routine wird der Schaltauftrag für ein Ausgangsrelais erteilt.
  * Mit mehreren Aufrufen von Switch() werden Aufträge gesammelt, erst mit
  * dem Aufruf von DoEnqueue() werden sie als einAuftrag in die Warteschlange
  * eingereiht.
  * Der Hintergrund ist, dass es je nach Konfiguration möglich ist, gemeinsam
  * aufgetretene Schaltaufträge auch gemeinsam auszuführen. Wenn die gespeicherte
  * Schaltenergie nicht zum Schalten aller Relais in einem Auftrag ausreicht, wird
  * gewartet, bis mehr Energie zur Verfügung steht.
  * Eine Besonderheit ist der Parameter forced (wenn angegeben und true):
  * Normalerweise werden beim Einstellen von Schaltaufträgen solche Einträge
  * (bzw Teile davon) verworfen, wenn sich die Schaltstellung gegenüber der
  * aktuellen Relaisstellung nicht ändert. Ebenso können Einträge gefiltert
  * werden, wenn in der Warteschlange noch entsprechende Schaltaufträge warten
  * oder hinfällig werden, weil nun ein gegenteiliger Schaltauftrag vorliegt.
  * forced=true sorgt dafür, dass dieses Relais auf alle Fälle einen
  * Ansteuerimpuls bekommt, unabhängig von der gespeicherten Relaisstellung.
  * Dies kann sinnvoll sein, z.B. bei Systemstart.
  * Während ein Auftrag mit gesetztem forced-Bit in der Warteschlange ist,
  * kann er von einem nachfolgenden Auftrag verändert werden wie ein normaler
  * Auftrag - aber er wird dann mit verändertem Inhalt ausgeführt, er wird
  * nicht ausgelassen wie ansonsten.
  */
 void Switch(int no, bool state, bool forced=false);

/*
 * Wertet die über Switch() eingegangenen Schaltvorgänge aus, sortiert Doubletten aus.
 * Überprüft, ob damit Schaltaufträge in der Warteschlange überflüssig werden, ändert bei Bedarf.
 * Sortiert die verbleibenden Aufträge in die Warteschlange ein.
 */
 void DoEnqueue(void);

 /*
  * Die eigentliche Schaltroutine. Wenn die gespeicherte Energie in den Elkos ausreicht,
  * werden den Schaltauftrag aus der Warteschlange geholt und unter Berücksichtigung der
  * aktuellen Zeit die Ansteuerdaten für die Relais-Treiber erzeugt.
  * Je nach Konfiguration werden zusammen erteilte Schaltaufträge gemeinsam durchgeführt
  * oder unabhängig betrachtet.
  * Der Aufrufer ist dann für die Übertragung dieser Daten zu den Treibern zuständig.
  */
 bool DoSwitching(unsigned time, unsigned &RelDriverData);

 /*
  * Liefert den gewünschten Schaltzustand der Relaisausgänge. D.h. als wenn die
  * gespeicherten Schaltaufträge bereits durchgeführt worden wären.
  */
 unsigned GetTrgState(void);

 /*
  * Liefert die aktuellen Schaltzustände der Relais.
  */
 unsigned GetRelState(void);

 /*
  * Startet die Relais-Schalt-Abarbeitung
  * Der Klasse müssen jetzt schon die Kanäle mit einer BusVoltageFailure-Zwangsstellung bekannt gemacht werden.
  */
 void Start(unsigned short BusVFailureMask, unsigned short BusVFailureData);

 /*
  * Stoppt die Relais-Schalt-Abarbeitung
  * Eine aktive Spulenansteuerung wird zuende geführt, jedoch keine neuen gestartet.
  * Auch wenn die Warteschlange noch nicht leer ist.
  */
 void Stop(void);

 /*
  * Stoppt die normale Relais-Abarbeitung und führt als nächstes die
  * BusVoltageFailure-Zwangsstellungen aus.
  * Eine aktive Spulenansteuerung wird zuende geführt, jedoch keine neuen gestartet.
  * Auch wenn die Warteschlange noch nicht leer ist.
  */
 void BusVoltageFailed(void);

 /*
  * Rückgabewert True, wenn die Relaismimik im Funktionszustand "Operating" ist, d.h.
  * sie wurde über "Start" gestartet, die Vermessung des Energieverbrauchs eines Schaltvorgangs
  * wurde durchgeführt, die Ladung in der Speicherkondensatoren ist hoch genug, um auch die
  * evtl. Schaltvorgänge im Falle eines Busspannungsausfalls bedienen zu können.
  */
 bool IsOperating(void);

 /*
  * Schreibt die aktuellen Relaisdaten in den Speicherbereich "data". Die Länge der
  * geschriebenen Daten wird als Rückgabewert zurückgegeben.
  * Die Daten enthalten die aktuellen Relaiszustände und noch ausstehende Schaltaufträge,
  * die aufgrund zu niedrigem Ladungszustand der Speicherkondensatoren verzögert sind.
  * Beim nächsten Systemstart werden sie ausgeführt, um einen konsistenten Systemzustand
  * herzustellen.
  */
 unsigned int GetData(void *data);

 /*
  * Liest Relaisdaten aus einem Speicherbereich "data". Die Länge der gelesenen Daten
  * wird als Rückgabewert zurückgegeben.
  * Die Daten enthalten die aktuellen Relaiszustände und noch ausstehende Schaltaufträge,
  * die vor dem letzten Reset/Brownout nicht mehr ausgeführt werden konnten.
  * Mit dieser Routine werden sie von den Relais-Routinen übernommen und nach "RelayStart"
  * ausgeführt.
  */
 unsigned int SetData(void *data);

 /*
  * Rückgabewert True, wenn die Busspannung nicht mehr ausreichend ist, um die Speicherrail auf
  * Mindestwert (in Bezug auf BusVoltageFailSwitching) zu laden. Dies ist dann bereits ein
  * BrownOut-Kriterium.
  */
 bool BusVoltageFailRailLevel(void);

 /*
  * Wie der Name sagt: Eine Debugging-Funktion ohne definierten Inhalt.
  */
 void debugging(void);

protected:
 /*
  * Berechnet die Zahl der möglichen Schaltvorgänge mit dem aktuellen Ladezustand.
  * Von dieser Zahl muss meist noch die Reserve für die BusVoltageFailure-Schaltvorgänge
  * abgezogen werden.
  */
 int CalcAvailRelEnergy(void);

 /*
  * Erzeugt für eine Relais die Schaltmuster in DriverData.
  */
 //inline void MakeOneChRelData(unsigned &DriverData, unsigned ch, unsigned short Mask, unsigned short Bits);

 TRelBuffer Buffer[RELAYBUFLEN];
 unsigned short NewMask;
 unsigned short NewBits;
 unsigned short NewForced;
 unsigned short ChRealSwStatus; // Aktueller Schaltzustande der Relais.
 unsigned short ChTargetSwStatus; // Schaltzustände, wenn alle in der Warteschlange stehenden Aufträge abgearbeitet sind.
 unsigned short ChForcedSwMsk; // Enthält die Kanäle, die unabhängig vom Zustand einen Schaltpuls bekommen sollen
 unsigned short BusVFailMask;
 unsigned short BusVFailData;
 unsigned BufWrPtr;
 unsigned BufRdPtr;
#define RELREQSTART    1
#define RELREQSTOP     2
#define RELREQBUSVFAIL 4
 unsigned short OpChgReq;
 RelOperatingStates OpState;
 RelSubStates SubState;
 unsigned int DriverData;
 unsigned int NextPointInTime;
 unsigned int EnergyCalcRefVoltage; // Speichert beim Messmodus die UBulk-Spannung vor der Messung
 unsigned int SingleSwitchEnergy;
 int PulseRepTmr[CHANNELCNT]; // Zähler/Timer für die Pulswiederholung
 unsigned int IdleDetTime; // Timer für den Idle-Detector
 unsigned int IdleDetRefVoltage;
 bool IdleDetected;

 void DoEnqueueInt(unsigned short *pmask, unsigned short *pbits, unsigned short *pforced );

 /*
  * Idle Detektor, liefert True wenn die Speicherkondensatoren voll sind und eine gewisse Zeit keine
  * Schaltaktionen getätigt wurden.
  */
 bool IdleDetect(unsigned time);

 /*
  * Liefert true, wenn die Warteschlange nicht leer ist.
  */
 bool BuffersNonEmpty(void);

 /*
  * Liefert true, wenn die Warteschlange leer ist.
  */
 bool BuffersEmpty(void);

 /*
  * Liefert den ältesten Eintrag der Warteschlange
  */
 bool FirstBufEntry(unsigned &Index);

 /*
  * Ändert den Index auf den nächsten Eintrag, berücksichtigt den möglichen Überlauf des Index.
  */
 void NextBufIndex(unsigned &Index);

 /*
  * Der übergebene Index wird auf den Folgeneintrag weitergesetzt. Liefert false, wenn es keinen
  * Folgeeintrag gibt, ansonsten true. Die Routine setzt voraus, das Index beim Aufruf auf einen
  * gültigen Eintrag der Warteschlange zeigt.
  */
 bool NextBufEntry(unsigned &Index);

 /* Löscht den Eintrag, auf den "Index" zeigt, aus der Warteschlange heraus
  * Index zeigt danach auf den Folgeeintrag.
  * Liefert true, wenn Index gültig ist, false wenn nach dem gelöschten
  * Eintrag keiner mehr folgt.
  */
 bool DelBufEntry(unsigned &Index);
};

/*
 * Liefert True, wenn die Busspannung hoch genug ist, um die Funktion des Aktors weiterhin
 * zu gewährleisten.
 * Die hierfür notwendige Mindestspannung ist von zwei Faktoren abhängig:
 * - Eine Mindestspannung, die bei Ausfall der Busspannung noch genügend Zeit lässt
 *   (durch Energie im Elko am 3,3V Schaltregler), um den aktuellen Systemzustand
 *   zu speichern.
 * - Eine Mindestspannung, ab der das Halten der Mindestenergie in den großen
 *   Speicherelkos nicht mehr gewährleistet ist. Die Mindestenergie ist abhängig
 *   von der Anzahl der Schaltkanäle, für die eine vorgegebene Schaltstellung bei
 *   Busspannungsausfall konfiguriert wurde.
 */
bool BusVoltageCheck(void);

/*
 * Eine Debugging-Funktion ohne definierten Inhalt.
 */
void RelTestEnqueue(void);

#endif /* RELAY_H_ */
