/*
 *  AdcIsr.cpp - Analog measurement and current calculation
 *
 *  For any further information see: inc/config.h
 *
 *  Copyright (C) 2017 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

/*
 * Important: This file should always be compiled with optimizations, otherwhise the ISR will consume too much time!
 *            Tested with -O3
 *
 * Wichtig: Diese Datei muss immer mit Optimierungen übersetzt werden, andernfalls wird die ISR zu viel Zeit benörigen!
 *          Getestet mit -O3
 */

#include <sblib/platform.h>
#include <config.h>
#include <AdcIsr.h>
#include <math.h>

#if (BUFSIZE*8) > 32767
#error BUFSIZE*8 too great for data type of IsrData.OffsIntegral!
#endif

volatile TIsrAdData IsrData;

typedef struct
{
 short int SelMsk; // Die Bitmaske zur Konfiguration des ADC Kanals
 short int LastCh; // Die Kanalnummer des letzten verwendeten ADC Kanals
 short int MuxMsk; // Die Bitmaske, mit der der externe Multiplexer angesteuert wird.
 bool MuxEna0;
 bool MuxEna1; // Wird nur bei mehr als 8 Kanälen benötigt
} TOneAdcChCfg;

#define ADCMSKLORNG (1 << CHANALOWRANGE)  // 1 0
#define ADCMSKHIRNG (1 << CHANAHIGHRANGE) // 2 1
#define ADCMSKURAIL (1 << CHANARAILVOLT)  // 4 2
#define ADCMSKUBUS (1 << CHANABUSVOLT)    // 0x80 7

#if IMEASNUMOFLOOPS == 10
TOneAdcChCfg AdcChCfg[10] =
{{ADCMSKHIRNG, CHANABUSVOLT, 0, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 0, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 1, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 1, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 2, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 2, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 3, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 3, true, false},
 {ADCMSKURAIL, CHANALOWRANGE, 0, false, false}, // AD2, Umeas rail
 {ADCMSKUBUS, CHANARAILVOLT, 0, false, false}   // AD7, Umeas EIB
};
/*TOneAdcChCfg AdcChCfg[10] =
{{2, 7, 0, true, false},
 {1, 1, 0, true, false},
 {2, 0, 1, true, false},
 {1, 1, 1, true, false},
 {2, 0, 2, true, false},
 {1, 1, 2, true, false},
 {2, 0, 3, true, false},
 {1, 1, 3, true, false},
 {4, 0, 0, false, false},    // AD2, Umeas rail
 {0x80, 2, 0, false, false}  // AD7, Umeas EIB
};*/
#elif IMEASNUMOFLOOPS == 20
 TOneAdcChCfg AdcChCfg[20] =
{{ADCMSKHIRNG, CHANABUSVOLT, 0, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 0, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 1, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 1, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 2, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 2, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 3, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 3, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 4, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 4, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 5, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 5, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 6, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 6, true, false},
 {ADCMSKHIRNG, CHANALOWRANGE, 7, true, false},
 {ADCMSKLORNG, CHANAHIGHRANGE, 7, true, false},
 {ADCMSKURAIL, CHANALOWRANGE, 0, false, false}, // AD2, Umeas rail
 {ADCMSKUBUS, CHANARAILVOLT, 0, false, false},  // AD7, Umeas EIB
 {ADCMSKURAIL, CHANABUSVOLT, 0, false, false},  // Zwei eigentlich unnötige Wandlungen, jedoch werden
 {ADCMSKUBUS, CHANARAILVOLT, 0, false, false}   // insg 20 Wandlungen je Schleife benötigt.
};
/* TOneAdcChCfg AdcChCfg[20] =
{{2, 7, 0, true, false},
 {1, 1, 0, true, false},
 {2, 0, 1, true, false},
 {1, 1, 1, true, false},
 {2, 0, 2, true, false},
 {1, 1, 2, true, false},
 {2, 0, 3, true, false},
 {1, 1, 3, true, false},
 {2, 0, 4, true, false},
 {1, 1, 4, true, false},
 {2, 0, 5, true, false},
 {1, 1, 5, true, false},
 {2, 0, 6, true, false},
 {1, 1, 6, true, false},
 {2, 0, 7, true, false},
 {1, 1, 7, true, false},
 {4, 0, 0, false, false},    // AD2, Umeas rail
 {0x80, 2, 0, false, false}, // AD7, Umeas EIB
 {4, 7, 0, false, false},    // Zwei eigentlich unnötige Wandlungen, jedoch werden
 {0x80, 2, 0, false, false}  // insg 20 Wandlungen je Schleife benötigt.
};*/
#elif IMEASNUMOFLOOPS == 30
 TOneAdcChCfg AdcChCfg[30] =
 {{ADCMSKHIRNG, CHANABUSVOLT, 0, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 0, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 1, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 1, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 2, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 2, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 3, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 3, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 4, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 4, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 5, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 5, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 6, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 6, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 7, true, false},
  {ADCMSKLORNG, CHANAHIGHRANGE, 7, true, false},
  {ADCMSKHIRNG, CHANALOWRANGE, 0, false, true},
  {ADCMSKLORNG, CHANAHIGHRANGE, 0, false, true},
  {ADCMSKHIRNG, CHANALOWRANGE, 1, false, true},
  {ADCMSKLORNG, CHANAHIGHRANGE, 1, false, true},
  {ADCMSKHIRNG, CHANALOWRANGE, 2, false, true},
  {ADCMSKLORNG, CHANAHIGHRANGE, 2, false, true},
  {ADCMSKHIRNG, CHANALOWRANGE, 3, false, true},
  {ADCMSKLORNG, CHANAHIGHRANGE, 3, false, true}, // 24 Messungen der 12 Kanäle
  {ADCMSKURAIL, CHANALOWRANGE, 0, false, false}, // AD2, Umeas rail
  {ADCMSKUBUS, CHANARAILVOLT, 0, false, false},  // AD7, Umeas EIB
  {ADCMSKURAIL, CHANABUSVOLT, 0, false, false},  // Vier eigentlich unnötige Wandlungen, jedoch werden
  {ADCMSKUBUS, CHANARAILVOLT, 0, false, false},  // insg 30 Wandlungen je Schleife benötigt.
  {ADCMSKURAIL, CHANABUSVOLT, 0, false, false},
  {ADCMSKUBUS, CHANARAILVOLT, 0, false, false}
 };
/* TOneAdcChCfg AdcChCfg[30] =
 {{2, 7, 0, true, false},
  {1, 1, 0, true, false},
  {2, 0, 1, true, false},
  {1, 1, 1, true, false},
  {2, 0, 2, true, false},
  {1, 1, 2, true, false},
  {2, 0, 3, true, false},
  {1, 1, 3, true, false},
  {2, 0, 4, true, false},
  {1, 1, 4, true, false},
  {2, 0, 5, true, false},
  {1, 1, 5, true, false},
  {2, 0, 6, true, false},
  {1, 1, 6, true, false},
  {2, 0, 7, true, false},
  {1, 1, 7, true, false},
  {2, 0, 0, false, true},
  {1, 1, 0, false, true},
  {2, 0, 1, false, true},
  {1, 1, 1, false, true},
  {2, 0, 2, false, true},
  {1, 1, 2, false, true},
  {2, 0, 3, false, true},
  {1, 1, 3, false, true},     // 24 Messungen der 12 Kanäle
  {4, 0, 0, false, false},    // AD2, Umeas rail
  {0x80, 2, 0, false, false}, // AD7, Umeas EIB
  {4, 7, 0, false, false},    // Vier eigentlich unnötige Wandlungen, jedoch werden
  {0x80, 2, 0, false, false}, // insg 30 Wandlungen je Schleife benötigt.
  {4, 7, 0, false, false},
  {0x80, 2, 0, false, false}
 };*/
#endif

#define RELPWM_URAILSHIFT   4
#define RELPWM_URAILOFFS   18
#define RELPWM_PWMTABLELEN 46

// Tabelle mit den Dutycycle-Werten in Abhängigkeit von der Spannung Ubulk, hiermit werden
// die Relais entsprechend der anliegenden Spannung gepulst.
// Das ganze könnte auch berechnet werden, das würde jedoch deutlich mehr Rechenzeit erfordern.
// ----> Korrektur für den Spannungsbfall der Freilaufdiode ist enthalten.
unsigned short RelPwmTable[RELPWM_PWMTABLELEN] =
{
  unsigned(RELPWMPRD*0.967),
  unsigned(RELPWMPRD*0.919),
  unsigned(RELPWMPRD*0.875),
  unsigned(RELPWMPRD*0.835),
  unsigned(RELPWMPRD*0.799),
  unsigned(RELPWMPRD*0.766),
  unsigned(RELPWMPRD*0.735),
  unsigned(RELPWMPRD*0.707),
  unsigned(RELPWMPRD*0.681),
  unsigned(RELPWMPRD*0.656),
  unsigned(RELPWMPRD*0.634),
  unsigned(RELPWMPRD*0.613),
  unsigned(RELPWMPRD*0.593),
  unsigned(RELPWMPRD*0.574),
  unsigned(RELPWMPRD*0.557),
  unsigned(RELPWMPRD*0.541),
  unsigned(RELPWMPRD*0.525),
  unsigned(RELPWMPRD*0.511),
  unsigned(RELPWMPRD*0.497),
  unsigned(RELPWMPRD*0.484),
  unsigned(RELPWMPRD*0.471),
  unsigned(RELPWMPRD*0.460),
  unsigned(RELPWMPRD*0.448),
  unsigned(RELPWMPRD*0.438),
  unsigned(RELPWMPRD*0.427),
  unsigned(RELPWMPRD*0.418),
  unsigned(RELPWMPRD*0.408),
  unsigned(RELPWMPRD*0.400),
  unsigned(RELPWMPRD*0.391),
  unsigned(RELPWMPRD*0.383),
  unsigned(RELPWMPRD*0.375),
  unsigned(RELPWMPRD*0.368),
  unsigned(RELPWMPRD*0.360),
  unsigned(RELPWMPRD*0.354),
  unsigned(RELPWMPRD*0.347),
  unsigned(RELPWMPRD*0.340),
  unsigned(RELPWMPRD*0.334),
  unsigned(RELPWMPRD*0.328),
  unsigned(RELPWMPRD*0.323),
  unsigned(RELPWMPRD*0.317),
  unsigned(RELPWMPRD*0.312),
  unsigned(RELPWMPRD*0.306),
  unsigned(RELPWMPRD*0.301),
  unsigned(RELPWMPRD*0.297),
  unsigned(RELPWMPRD*0.292),
  unsigned(RELPWMPRD*0.287)
};


ALWAYS_INLINE void SetChMultiplexer(int idx)
{
#ifdef PIOMUXENA0
 digitalWrite(PIOMUXENA0, AdcChCfg[idx].MuxEna0);
#endif
#if ADCCHANNELCNT > 8
   digitalWrite(PIOMUXENA1, AdcChCfg[idx].MuxEna1);
#endif
#ifdef PIOMUX2
 gpioPorts[PIOMUXPORT]->MASKED_ACCESS[7] = AdcChCfg[idx].MuxMsk;
#else
 gpioPorts[PIOMUXPORT]->MASKED_ACCESS[3] = AdcChCfg[idx].MuxMsk;
#endif
 LPC_ADC->CR = IsrData.CrCopy + AdcChCfg[idx].SelMsk; // Start über steigende Flanke eines Timers, unten SelMask,
 // alles andere Standard
}

ALWAYS_INLINE void SetChMultiplexerStartOnly(int idx)
{
 LPC_ADC->CR = IsrData.CrCopy + AdcChCfg[idx].SelMsk; // Start über steigende Flanke eines Timers, unten SelMask,
 // alles andere Standard
}

void analogSetup(void)
{
 pinMode(PIOANALOWRANGE, INPUT_ANALOG);  // AD0 I range low
 pinMode(PIOANAHIGHRANGE, INPUT_ANALOG); // AD1 I range high
 pinMode(PIOANARAILVOLT, INPUT_ANALOG);  // AD2 Umeas rail
 pinMode(PIOANABUSVOLT, INPUT_ANALOG);   // AD7 Umeas EIB
#ifdef PIOMUXENA0
 pinMode(PIOMUXENA0, OUTPUT);
#endif
#ifdef PIOMUXENA1
 pinMode(PIOMUXENA1, OUTPUT);
#endif
 //pinMode(PIOMUX0, OUTPUT_MATCH); Debug only
 pinMode(PIOMUX0, OUTPUT);
 pinMode(PIOMUX1, OUTPUT);
#ifdef PIOMUX2
 pinMode(PIOMUX2, OUTPUT);
#endif
#ifdef PIODBGISRFLAG
 pinMode(PIODBGISRFLAG, OUTPUT);
#endif

#ifdef PIOMUX2
 gpioPorts[PIOMUXPORT]->MASKED_ACCESS[7] = AdcChCfg[0].MuxMsk; // Über Masked_Access werden nur die untersten 3 Bits des Ports beschrieben
#else
 gpioPorts[PIOMUXPORT]->MASKED_ACCESS[3] = AdcChCfg[0].MuxMsk; // Über Masked_Access werden nur die untersten 2 Bits des Ports beschrieben
#endif
#ifdef PIOMUXENA0
 digitalWrite(PIOMUXENA0, AdcChCfg[0].MuxEna0);
#endif
#if ADCCHANNELCNT > 8
 digitalWrite(PIOMUXENA1, AdcChCfg[0].MuxEna1);
#endif
 // Folgendes Setup stammt aus analog_pin.cpp
 // Disable power down bit to the ADC block.
 LPC_SYSCON->PDRUNCFG &= ~(1<<4);
 // Enable AHB clock to the ADC.
 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);
 // Vorteiler des ADC (SystemCoreClock momentan 48MHz, der Teiler SYSAHBCLKDIV wirkt sich auf Core und Peripherie aus
 IsrData.CrCopy = ((SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / ADC_CLOCK - 1) << 8;
 LPC_ADC->CR = IsrData.CrCopy;
 IsrData.CrCopy |= 0x06000000; // Wandlungsstart über CT16B0_MAT0
 LPC_ADC->GDR;                 // Dummy Lesezugriff auf das Global data register
 SetChMultiplexer(0); // Setzt auch die Multiplexer, wählt den Adc-Kanal und "Start auf Tmr-Match"
#ifdef SPICSEMULATION
 IsrData.SpiActive = false;
#endif
 IsrData.NewData = false;
}

extern "C" void ADC_IRQHandler (void)
{
#ifdef PIODBGISRFLAG
 digitalWrite(PIODBGISRFLAG, true);
#endif
 int NextIndex = IsrData.ActChIdx+1;
 if (NextIndex >= IMEASNUMOFLOOPS)
  NextIndex=0;
 SetChMultiplexer(NextIndex);
 // ADC-Wert auslesen
 int AdcVal = (LPC_ADC->DR[AdcChCfg[NextIndex].LastCh] & 0xffff) >> 6;
 //int AdcVal = (LPC_ADC->GDR & 0xffff) >> 6; // Durch einen Silicon-Bug ist das GDR-Register praktisch unbrauchbar
 if (IsrData.ActChIdx < IMEASMUXCHANNELS)
 { // Messungen des Stroms
  if (AdcVal > LIMITIMEASOVR)
   IsrData.OvrRng[IsrData.ActChIdx]++;
  if (AdcVal < LIMITIMEASUND)
   IsrData.OvrRng[IsrData.ActChIdx]++;
  AdcVal -= IsrData.Offs[IsrData.ActChIdx];
  // Aufsummieren für Mittelwertberechnung (Offset)
  IsrData.AccuSum[IsrData.ActChIdx] += AdcVal;
  // Aufsummieren für Effektivwertberechnung
  IsrData.AccuSqr[IsrData.ActChIdx] += AdcVal*AdcVal;
  if (IsrData.DnCnt == 0)
  {
   IsrData.RegSum[IsrData.ActChIdx] = IsrData.AccuSum[IsrData.ActChIdx];
   IsrData.AccuSum[IsrData.ActChIdx] = 0;
   IsrData.RegSqr[IsrData.ActChIdx] = IsrData.AccuSqr[IsrData.ActChIdx];
   IsrData.AccuSqr[IsrData.ActChIdx] = 0;
   IsrData.RegOvr[IsrData.ActChIdx] = IsrData.OvrRng[IsrData.ActChIdx];
   IsrData.OvrRng[IsrData.ActChIdx] = 0;
  }
 } else {
  // - Messung der Busspannung (gefiltert durch ein dickes LC-Filter, effektiv die Versorgungsspannung.
  // - Messung der "Railspannung", der mit dem großen Energiespeicher für die Schaltvorgänge
  int Idx = IsrData.ActChIdx-IMEASMUXCHANNELS;
  if (Idx < 2)
  {
   unsigned Voltage;
   IsrData.UAccu[Idx] += AdcVal;
   if ((++IsrData.UCnt & 6) == 0) // Bei 4 gesammelten Werten ergibt sich alle 0,8ms ein neuer Wert.
   {
    Voltage = IsrData.UAccu[Idx] >> 2;
    IsrData.UValues[Idx] = Voltage;
    IsrData.UAccu[Idx] = 0;
    if (Idx == 0)
    {
     // Die gemessene Spannung wird in einen Array-Index umgerechnet
     Voltage >>= RELPWM_URAILSHIFT;
     if (Voltage < RELPWM_URAILOFFS)
     {
      Voltage = RELPWMPRD;
     } else {
      Voltage -= RELPWM_URAILOFFS;
      if (Voltage > RELPWM_PWMTABLELEN)
      {
       Voltage = RELPWM_PWMTABLELEN;
      }
      Voltage = RelPwmTable[Voltage];
     }
     // Das Enable Signal zum Shiftregister ist low-aktiv, das passt mit der PWM Logik gut zusammen
     RelPwmTmr.match(RELPWMDCCH, Voltage); // Schreiben des neuen Dutycycle-Werts
     //RelPwmTmr.match(RELPWMDCCH, RELPWMPRD);
    }
   }
   if (Idx == 1)
   {
#ifdef SPICSEMULATION
    // RelSpi ist "fremd" hier, aber irgendwie muss das SPI-CS bedient werden (der Pin ist nicht der Hardware-CS Pin)
    if (IsrData.SpiActive && !(SPISSP->SR & 0x10))
    {
     digitalWrite(PIOSPICS, true);
    }
#endif
    if (IsrData.DnCnt == 0) // Erst nachdem oben alle Kopieraktionen beendet sind, wird hier das Flag gesetzt
    {
     IsrData.NewData = true;
    }
   }
  }
 }
 if (NextIndex == 0)
 {
  if (IsrData.DnCnt-- == 0)
  {
   IsrData.DnCnt = (BUFSIZE-1);
  }
 }
 IsrData.ActChIdx = NextIndex;
#ifdef PIODBGISRFLAG
 digitalWrite(PIODBGISRFLAG, false);
#endif
}

void adctimerSetup(void)
{
 // Der Timer ist die eigentliche Zeitbasis der Sampling-Routinen, er startet periodisch den ADC.
 // Die Interrupt-Routine auf ADC-Conversion Done macht dann alles weitere.
 // Es werden die Timer-Funktionen aus Timer.h/.cpp genutzt
 AdcCtrlTmr.begin();
 AdcCtrlTmr.noInterrupts();
 AdcCtrlTmr.prescaler(0);
 AdcCtrlTmr.matchMode(0, TOGGLE | RESET); // Toggle und Reset mit Ch 0 (Der Timer hat 4 "Channels" je Timer)
 // ==> Dadurch, dass nur "getoggled" werden kann, muss die halbe Matchperiode konfiguriert werden!
 AdcCtrlTmr.match(0, (SystemCoreClock / LPC_SYSCON->SYSAHBCLKDIV) / ADCSAMPLEFREQ / 2 - 1);
 AdcCtrlTmr.start();
}

int GetRailVoltage(void)
{
 return IsrData.AdcUrail;
}

int GetBusVoltage(void)
{
 return IsrData.AdcUbus;
}

// Die Relais werden mit einer PWM angesteuert, die abhängig von der Spannung Ubulk ist
// Bei 12V sind es 100%, bei höheren Spannungen weniger, so dass der Durchschnitt wieder
// 12V sind. (Die Tabelle enthält eine Korrektur des Diodenabfalls im Freilauffall.)
void pwmSetup(void)
{
 pwmEnable(false);
 RelPwmTmr.begin();
 RelPwmTmr.noInterrupts();
 RelPwmTmr.prescaler(0);
 RelPwmTmr.matchMode(RELPWMPRDCH, RESET); // Weist einem Zähler die Funktion der Periodendauer zu
 RelPwmTmr.match(RELPWMPRDCH, RELPWMPRD-1);
 RelPwmTmr.match(RELPWMDCCH, RELPWMDC50);
 RelPwmTmr.pwmEnable(RELPWMDCCH); // Startet einen Zähler als PWM
 RelPwmTmr.start();
}

/* Die PWM ist immer nur dann aktiv, wenn Relais geschaltet werden.
 * Dafür wird sie von der Relais-Unit vor einem Schaltpuls eingeschaltet
 * und danach wieder deaktiviert.
*/
void pwmEnable(bool ena)
{
 if (ena)
 {
  pinMode(PIORELPWM, OUTPUT_MATCH);
 } else {
  digitalWrite(PIORELPWM, true); // Treiber inaktiv bei high Pegel
  pinMode(PIORELPWM, OUTPUT);
 }
}

void IsrSetup(void)
{
 IsrData.DnCnt = BUFSIZE;
 IsrData.ActChIdx = 0;
 for (int idx=0; idx < (ADCCHANNELCNT*2); idx++)
 {
  IsrData.Offs[idx] = 511;
  IsrData.OffsIntegral[idx] = 0;
  if (idx & 1)
  {
   // Low Range
   IsrData.GainCorr[idx] = (short unsigned)(1.0275*32768); // +2,75% Korrektur für den Low-Range
  } else {
   // High Range
   IsrData.GainCorr[idx] = (short unsigned)(1.0125*32768); // +1,25% Korrektur für den High-Range
  }
 }
 IsrData.UAccu[0] = 0;
 IsrData.UAccu[1] = 0;
 NVIC_SetPriority(ADC_IRQn, 1); // Das sollte für erhöhte Interruptpriorität sorgen
 NVIC_EnableIRQ(ADC_IRQn);
}

bool AdcIsrNewDataAvail(void)
{
 if (IsrData.NewData)
 {
  IsrData.NewData = false;
  return true;
 }
 return false;
}

ALWAYS_INLINE float Square(float a)
{ // Mir ist nicht 100% klar, ob pow(a,2) nur optimiert a*a berechnet, oder doch eine universelle Exponentialfunktion. Daher hier:
 return a*a;
}

void AdcIsrCurrFilt(void)
{
 float ChCurr;
 for (unsigned ChIdx=0; ChIdx < ADCCHANNELCNT; ChIdx++)
 {
  ChCurr = 0;
  for (unsigned i=0; i<3; i++)
  {
   ChCurr += IsrData.CurrSqrVals[ChIdx][i];
   IsrData.CurrSqrVals[ChIdx][i+1] = IsrData.CurrSqrVals[ChIdx][i];
  }
  if ((IsrData.RegOvr[(ChIdx << 1)+1]) < OFSCOMPOVRLIM)
  { // Der Low-Range ist nicht übersteuert -> Low-Range als Datenquelle benutzen
   IsrData.CurrSqrVals[ChIdx][0] =
     (float)IsrData.RegSqr[(ChIdx << 1)+1] *
     Square((float)IsrData.GainCorr[(ChIdx << 1)+1]) *
     Square(MAXCURRLOWRANGE/512/32768);
  } else { // High-Range benutzen
   IsrData.CurrSqrVals[ChIdx][0] =
     (float)IsrData.RegSqr[ChIdx << 1] *
     Square((float)IsrData.GainCorr[ChIdx << 1]) *
     Square(MAXCURRHIGHRANGE/512/32768);
  }
  ChCurr += IsrData.CurrSqrVals[ChIdx][0];
  IsrData.CurrentVal[ChIdx] = sqrt(ChCurr * (0.25 / (float)BUFSIZE)); // Strom in A
 }
}

float GetChannelCurrent(int ChIdx)
{
 return IsrData.CurrentVal[ChIdx];
}

void AdcIsrProcOffset(void)
{
 for (unsigned ChIdx=0; ChIdx < IMEASMUXCHANNELS; ChIdx++)
 {
  if ((IsrData.RegOvr[ChIdx]) < OFSCOMPOVRLIM)
  {
   //IsrData.OffsIntegral[ChIdx] += IsrData.RegSum[ChIdx];
   IsrData.OffsIntegral[ChIdx] = min(max(IsrData.OffsIntegral[ChIdx] + IsrData.RegSum[ChIdx], -BUFSIZE*8), BUFSIZE*8);
   if (IsrData.OffsIntegral[ChIdx] > (BUFSIZE*4))
   {
    IsrData.OffsIntegral[ChIdx] -= (BUFSIZE*4);
    IsrData.Offs[ChIdx] = min(IsrData.Offs[ChIdx]+1, OFSCOMPHIGHLIM);
   }
   if (IsrData.OffsIntegral[ChIdx] < (-BUFSIZE*4))
   {
    IsrData.OffsIntegral[ChIdx] += (BUFSIZE*4);
    IsrData.Offs[ChIdx] = max(IsrData.Offs[ChIdx]-1, OFSCOMPLOWLIM);
   }
  }
 }
}
