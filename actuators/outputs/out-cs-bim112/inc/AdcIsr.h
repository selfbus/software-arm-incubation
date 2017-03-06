/*
 *  AdcIsr.h - Analog measurement and current calculation
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef ADCISR_H_
#define ADCISR_H_

#if ADCCHANNELCNT == 4
// 10 Wandlungen je Loop, 50kHz Samplefreq
#define IMEASNUMOFLOOPS 10
#define ADCSAMPLEFREQ 50000

#elif ADCCHANNELCNT == 8
// 20 Wandlungen je Loop, 100kHz Samplefreq
#define IMEASNUMOFLOOPS 20
#define ADCSAMPLEFREQ 100000

#elif ADCCHANNELCNT == 12
// 30 Wandlungen je Loop, 96kHz Samplefreq
#define IMEASNUMOFLOOPS 30
#define ADCSAMPLEFREQ 96000

#endif

#define IMEASMUXCHANNELS (ADCCHANNELCNT*2)
//#define ADCSAMPLEFREQ 100000 // Ergeben bei insg 20 Wandlungen je Loop (2 Wandlungen unnötig, aber eine Samplefreq von
                          // 90kHz lässt sich nicht exakt einstellen) 100 Stützstellen je Netzperiode bei 50Hz (83,33 bei 60Hz)
                         // 10.0µs je Wandlung
#define ADCCYCLEFREQ (ADCSAMPLEFREQ/IMEASNUMOFLOOPS) // 5kHz bei 100kHz SAMPLEFREQ und 20 Samples/Zyklus
#define RMSCURRENTVALUESPERSECOND 4 // Die Routine AdcIsrCurrFilt() berechnet mehrmals pro
  // Sekunde einen neuen Effektivwert. Es wird ein gleitender Wert aus den letzten 4 "Buffern" berechnet, jeder
  // Effektivwert entspricht einer Messperiode von einer ganzen Sekunde. Die momentane Auslegung entspricht
  // 4 Werten je Sekunde, 1 Sekunde Effektivwertberechnungszeit. RMSCURRENTVALUESPERSECOND sollte eine ganze Zahl ergeben!
#define BUFSIZE (ADCCYCLEFREQ/RMSCURRENTVALUESPERSECOND) //1250 // sollte Vielfaches von 5*(Stützstellen je Netzperiode bei 50Hz) sein, dann ist's auch bei 60Hz eine
                    // komplette Zahl von Netzperioden. Darf nicht so groß sein, dass der Akkumulator überlaufen könnte.
                    // 32-(2*10-1) bit = 13 bit = 8192 digits
                    // 5000 ist gleichbedeutend mit einer Sekunde
#define OFSCOMPOVRLIM 5    // Anzahl Messwertausreißer, die in einer Messperiode maximal auftreten dürfen, damit der Messbereich nicht als übersteuert betrachtet wird.
#define OFSCOMPHIGHLIM 516 // Der Offsetwert liegt im toleranzlosen Fall bei 511-512. Der Wert der dynamischen Kompensierung darf
#define OFSCOMPLOWLIM 508  // sich nur in festgelegten Grenzen befinden.

// Neben den Kanälen High-Range / Low-Range werden gemessen:
// Zwischenkreisspannung sowie die Busspannung (über den REF-Knoten)

// Takt für den AD-Wandler
#define ADC_CLOCK  2400000 // Max 4.5MHz, 11 Taktzyklen/Wandlung. Bei 2.4MHz 4.6µs je Wandlung, da bleiben 5.6µs für
                           // den Multiplexer und die Acquisition.
                           // Bei höherer SampleFreq darf die ADC_CLOCK ruhig näher an den Maxwert gehen.
                           // Beobachtete Zeit von SOC bis zum Eintritt ISR 6µs
//#define ADC_CLOCK  4363000 // Würde gut 2us sparen -> Auswirkungen auf Linearität & Rauschen?

// Bei der Auswertung der Strommesswerte werden die ADC-Werte auf Bereichsüberschreitung überprüft. So soll später eine
// Verfälschung der Offsetkorrektur verhindert werden. Da der ansteuernde OPAMP vermutlich nicht bis ganz an die Rails
// kommen kann, ist die exakte Grenze einstellbar.
// Experimentell ermittelt für den kleineren (empfindlicheren) Messbereich wurde ein nutzbarere Aussteuerbereich
// bis an die Extremwerte 0 und 1023 heran, d.h. der OPAMP ist aus Sicht des ADC tatsächlich Rail-to-Rail.
// Sicherheitshalber wird dennoch ein Sicherheitsabstand verwendet.
#define LIMITIMEASOVR (1023-5) // Limit IMeas Range Overflow (positive Richtung)
#define LIMITIMEASUND 5 // Limit IMEas Range Overflow (negative Richtung)



// INT ist beim Cortex M0 ein 32bit-Typ
typedef struct
{
 short int ActChIdx; // Interner Index für den aktuellen Messkanal. Die ISR ermittelt aus der Tabelle AdcChCfg die notwendige Konfiguration
 short int DnCnt;    // Ein Downcounter für die restlichen Durchläufe der aktuellen Messperiode
 short int UCnt;     // Für den Fall, dass "BUFSIZE" nicht durch 4 teilbar ist, wird ein eigener Counter für die Spannungsmessungen benötigt
 int CrCopy;         // Eine Kopie des LPC_ADC->CR Registers, jedoch ohne die Bits, die durch AdcChCfg ergänzt werden. Beschleunigt den Zugriff.
 int AccuSum[ADCCHANNELCNT*2];          // Akkumulator mit der Summe der Messwerte über eine Messperiode zur Offsetkorrektur
 unsigned int AccuSqr[ADCCHANNELCNT*2]; // Akkumulator mit der Summe der Quadrate zur Effektivwertberechnung
 short int Offs[ADCCHANNELCNT*2];       // Aktueller Offsetkorreturwert, der in der ISR benutzt wird
 short int OvrRng[ADCCHANNELCNT*2];     // Zähler mit der Anzahl der Messbereichsüberlaufe (in positiver wie negativer Richtung) über eine Messperiode
 int RegSum[ADCCHANNELCNT*2];           // Hier legt die ISR die Messwertsumme nach einer Messperiode ab
 unsigned int RegSqr[ADCCHANNELCNT*2];  // Hier legt die ISR die Summe der Quadrate nach einer Messperiode ab
 short int RegOvr[ADCCHANNELCNT*2];     // Hier legt die ISR den Zählwert der Messbereichsüberschreitungen nach einer Messperiode ab
 short int OffsIntegral[ADCCHANNELCNT*2];  // Ein Integral der vergangenen RegSum, wird für die Offsetkorrektur verwendet
 short unsigned GainCorr[ADCCHANNELCNT*2]; // Gain-Korrekturwerte in 1.15 FixedPoint (vorzeichenlos)
 float CurrSqrVals[ADCCHANNELCNT][4];   // Für ein gleitendes Filter über die vergangenen 4 Messwerte
 float CurrentVal[ADCCHANNELCNT];       // Und schließlich das Ergebnis der Strommessung
 short unsigned int UAccu[2]; // Ein kleiner Akkumulator für die Spannungsmesswerte
 union {
  short unsigned int UValues[2];
  struct
  {
   short unsigned int AdcUrail; // Die gemittelten Spannungswerte
   short unsigned int AdcUbus;
  };
 };
 bool NewData;        // Flag, mit dem die ISR einen neuen Datensatz "nach unten" meldet
#ifdef SPICSEMULATION
 bool SpiActive;
#endif
} TIsrAdData;
// 20 + 72*ADCCHANNELCNT

extern volatile TIsrAdData IsrData;

void analogSetup(void);

void adctimerSetup(void);

/*
 * Konfiguriert die PWM für die Relaisansteuerung, setzt den Pegel
 * am Ausgangspin jedoch noch auf "Treiber deaktiviert"
 */
void pwmSetup(void);

/*
 * Mit dieser Funktion wird während eines Ansteuerpulses für die Relais
 * die PWM am entsprechenden Pin aktiviert und später wieder deaktiviert.
 */
void pwmEnable(bool ena);

void IsrSetup(void);

float GetChannelCurrent(int ChIdx);

int GetRailVoltage(void);

int GetBusVoltage(void);

bool AdcIsrNewDataAvail(void);

void AdcIsrProcOffset(void);

void AdcIsrCurrFilt(void);

#endif /* ADCISR_H_ */
