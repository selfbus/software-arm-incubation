/*
 *  Relay.cpp - Switching queue, energy management, relay timing
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
#include <Relay.h>
#include <AdcIsr.h>
#include <Appl.h>

Relay relay;

// System time in milliseconds (from timer.cpp)
extern volatile unsigned int systemTime;

#ifdef RELAYUSEISR
extern "C" void TIMER32_0_IRQHandler (void)
{
 relay.Do();
 RelTmr.resetFlags();
}
#endif

bool BusVoltageCheck(void)
{
 bool Fail = false;
 // Die Überwachung auf Busspannungsausfall ist zweigeteilt:
 // - Unterschreitung einer absoluten Mindestspannung auf dem Bus
 // - Absinken der Busspannung unter ein Level, wo ein Halten der Mindestreserve für Schaltaktionen bei
 //   Busspannungsausfall nicht mehr sichergestellt ist.
 if (GetBusVoltage() < (int)MINUBUSVOLTAGEFALLING)
  Fail = true;
 // Im Disable-Zustand oder während MeasMode liefert BusVoltageFailRailLevel() keine sinnvollen Ergebnisse,
 // da die Energiemenge noch unbekannt, die für einen Schaltvorgang notwendig ist.
 if (relay.IsOperating())
 {
  if (relay.BusVoltageFailRailLevel())
  {
   Fail = true;
  }
 }
 if (Fail)
 {
  relay.BusVoltageFailed();
 }
 return not Fail;
}

Relay::Relay(void)
{
 ChRealSwStatus = 0;
 ChTargetSwStatus = 0;
 BufWrPtr = 0;
 BufRdPtr = 0;
 NewMask = 0;
 NewBits = 0;
 OpState = RelOperatingStates::Disable;
 SubState = RelSubStates::Idle;
 SingleSwitchEnergy = 0;
 OpChgReq = 0;
}

void Relay::Switch(int no, bool state, bool forced)
{
 unsigned int zw;
 zw = 1 << no;
 NewMask |= zw;
 if (forced)
  NewForced |= zw;
 if (state)
 {
  NewBits |= zw;
 } else {
  NewBits &= ~zw;
 }
}

bool Relay::FirstBufEntry(unsigned &Index)
{
 if (BufWrPtr != BufRdPtr)
 {
  Index = BufRdPtr;
  return true;
 } else {
  return false;
 }
}

ALWAYS_INLINE bool Relay::BuffersEmpty(void)
{
 return (BufWrPtr == BufRdPtr);
}

ALWAYS_INLINE bool Relay::BuffersNonEmpty(void)
{
 return (BufWrPtr != BufRdPtr);
}

void Relay::NextBufIndex(unsigned &Index)
{
 Index++;
 if (Index >= RELAYBUFLEN)
 {
  Index=0;
 }
}

bool Relay::NextBufEntry(unsigned &Index)
{
 NextBufIndex(Index);
 if (Index == BufWrPtr)
 {
  return false;
 } else {
  return true;
 }
}

/* Löscht den Eintrag, auf den "Index" zeigt, aus der Warteschlange heraus
 * Index zeigt danach auf den Folgeeintrag.
 * Liefert true, wenn Index gültig ist, false wenn nach dem gelöschten
 * Eintrag keiner mehr folgt.
 */
bool Relay::DelBufEntry(unsigned &Index)
{
 unsigned ActIndex = Index;
 unsigned NextIndex = Index;
 bool retval = false;
 if (NextBufEntry(NextIndex))
 {
  retval = true;
  do
  {
   Buffer[ActIndex] = Buffer[NextIndex];
   ActIndex = NextIndex;
  } while (NextBufEntry(NextIndex));
 }
 BufWrPtr = ActIndex;
 return retval;
}

unsigned Relay::GetTrgState(void)
{
 return ChTargetSwStatus;
}

unsigned Relay::GetRelState(void)
{
 return ChRealSwStatus;
}

#if 0
void Relay::DoEnqueue_old(void)
{
 // Die Abarbeitungsroutinen sind so formuliert, dass in einem Durchlauf erteilte Schaltaufträge
 // gemeinsam durchgeführt werden. Es wird bei mangelnder Energie im Pufferkondensator dann auch
 // gewartet, bis genug Ladung für mehrere Relais gleichzeitig vorhanden ist.
 // Dieses Verhalten kann mittels #define aktiviert/deaktiviert werden. Ist es deaktiviert,
 // werden die Aufträge bei Bedarf auseinandergedröselt.
 if (NewMask)
 {
  unsigned short Schnittmenge;
  bool StillNotDone = false;
  unsigned Index;
  // Erst mal schnell den Zielzustand setzen ohne Berücksichtigung der Schaltverzögerungen
  ChTargetSwStatus &= ~NewMask;
  ChTargetSwStatus |= NewBits;

#ifdef RELAYUSEISR
  NVIC_DisableIRQ(RelTmrIRQn);
#endif
  // Der neue Schaltauftrag wird in die Warteschlange gestellt
  // Dazu muss erst überprüft werden, ob noch ein alter Auftrag zum selben Kanal in der
  // Queue wartet (und mangels ausreichender Energie in den Pufferkondensatoren noch nicht
  // ausgeführt werden konnte). Heben sich die Schaltaufträge auf, werden sie aussortiert.
  if (FirstBufEntry(Index))
  {
   do {
    StillNotDone = false;
    Schnittmenge = Buffer[Index].Mask & NewMask;
    if (Schnittmenge)
    {
     // Es kann davon ausgegangen werden: Der gefundene ältere Schaltauftrag würde den Zustand ändern,
     // ist also ungleich dem realen Zustand.
     // Beim neuen Auftrag ist dies noch unbekannt. Er kann ein- oder ausschaltend sein, unabhängig vom
     // alten Zustand.
     // Kanalweise:
     // Neuer Auftrag == alter Auftrag ? => neuer Auftrag unnötig, löschen
     // Neuer Auftrag != alter Auftrag ? => hebt den alten Auftrag auf, beide löschen(!)
     // Blöderweise kann ein Auftrag mehrere Kanäle enthalten und das ganze Kanal für Kanal unterschiedlich sein
     Buffer[Index].Mask &= ~((Buffer[Index].Bits ^ NewBits) & Schnittmenge);
     // In jedem Fall: Die Schnittmenge kann aus dem neuen Auftrag rausgelöscht werden
     NewMask &= ~Schnittmenge;
     // Der alte Auftrag oder der neue Auftrag (oder beide) könnten jetzt leer sein
     if (Buffer[Index].Mask == 0)
     {
      // alten Auftrag aus dem Fifo herauslöschen
      if (!DelBufEntry(Index))
      {
       break; // Ups, der Index ist nicht mehr gültig, da es keinen Eintrag danach mehr gibt
      }
      StillNotDone = true;
      //Es darf kein NextBufEntry aufgerufen werden, der Index ist schon weitergesetzt!
     }
     if (NewMask == 0)
     {
      break; // Die neue Maske ist leer, da braucht nicht weiter überprüft werden
     }
    }
    if (!StillNotDone)
    {
     StillNotDone = NextBufEntry(Index);
    }
   } while (StillNotDone);
  }
#ifdef RELAYUSEISR
  NVIC_EnableIRQ(RelTmrIRQn);
#endif
  // Für einen neuen Auftrag, der es durch die Bereinigung oben geschafft hat, gilt:
  // Es ist kein anderer, älterer Auftrag zu diesem Kanal in der Warteschlange vorhanden.
  // Noch nicht sichergestellt ist, ob der Schaltauftrag den Schaltzustand überhaupt verändern
  // würde. Aber das ist einfach: Einfach mit ActState vergleichen
  // Es darf kein Schaltauftrag eingetragen werden, der den aktuellen Zustand nicht verändert.
  NewMask &= (ChRealSwStatus ^ NewBits);
 }
 // Ist nach der ganzen Überprüfung noch etwas übrig von Schaltauftrag?
 if (NewMask)
 {
  // Den Auftrag jetzt in die Queue eintragen
  // Als Folge des Aussortierens oben kann der Buffer nie überlaufen, es bleiben
  // immer höchstens so viele Aufträge übrig, wie Kanäle vorhanden sind.
  Buffer[BufWrPtr].Mask = NewMask;
  Buffer[BufWrPtr].Bits = NewBits & NewMask;
  NextBufIndex(BufWrPtr);
 }
 NewMask = 0;
 NewBits = 0;
}
#endif

void Relay::DoEnqueue()
{
 DoEnqueueInt(&NewMask, &NewBits, &NewForced);
}

void Relay::DoEnqueueInt(unsigned short *pmask, unsigned short *pbits, unsigned short *pforced )
{
 unsigned int mask = *pmask;
 unsigned int bits = *pbits;
 unsigned int forced = *pforced;
 *pmask = 0;
 *pbits = 0;
 *pforced = 0;
 // Die Abarbeitungsroutinen sind so formuliert, dass in einem Durchlauf erteilte Schaltaufträge
 // gemeinsam durchgeführt werden. Es wird bei mangelnder Energie im Pufferkondensator dann auch
 // gewartet, bis genug Ladung für mehrere Relais gleichzeitig vorhanden ist.
 // Dieses Verhalten kann mittels #define aktiviert/deaktiviert werden. Ist es deaktiviert,
 // werden die Aufträge bei Bedarf auseinandergedröselt.
 if (mask)
 {
  unsigned short Schnittmenge;
  bool StillNotDone = false;
  unsigned Index;
  // Erst mal schnell den Zielzustand setzen ohne Berücksichtigung der Schaltverzögerungen
  ChTargetSwStatus &= ~mask;
  ChTargetSwStatus |= bits;
  ChForcedSwMsk |= forced;

#ifdef RELAYUSEISR
  NVIC_DisableIRQ(RelTmrIRQn);
#endif
  // Der neue Schaltauftrag wird in die Warteschlange gestellt
  // Dazu muss erst überprüft werden, ob noch ein alter Auftrag zum selben Kanal in der
  // Queue wartet (und mangels ausreichender Energie in den Pufferkondensatoren noch nicht
  // ausgeführt werden konnte). Heben sich die Schaltaufträge auf, werden sie aussortiert.
  // Eine Ausnahme bilden Kanäle mit gesetztem "Forced"-Bit, sie sollen einen Schaltpuls
  // bekommen, unabhängig von der aktuellen Schaltposition. Daher wird in so einem Fall
  // zwar der alte, überholte Schaltauftrag aus der Warteschlange gelöscht, der neue aber
  // in jedem Fall eingestellt.
  if (FirstBufEntry(Index))
  {
   do {
    StillNotDone = false;
    Schnittmenge = Buffer[Index].Mask & mask;
    if (Schnittmenge)
    {
     // Es wurde ein älterer Schaltauftrag gefunden, der einen oder mehrere gleiche Kanäle anspricht.
     // Damit ist die Schnittmenge im alten Auftrag auf alle Fälle überflüssig und wird dort gelöscht.
     // Kanalweise:
     // Beim alten Auftrag gilt: Er ändert den aktuellen Schaltzustand oder es ist zusätzlich das Forced-Bit gesetzt.
     // Beim neuen Auftrag gilt. Er kann ein- oder ausschaltend sein, unabhängig vom alten Zustand.
     // Neuer Auftrag == alter Auftrag ? => neuer Auftrag unnötig, löschen. Der alten Auftrag bekommt dann den Forced-Status
     // Neuer Auftrag != alter Auftrag ? => hebt den alten Auftrag auf, alten löschen. Der neue bleibt nur bei gesetzem
     //                                     Forced-Bit bestehen. Er "erbt" dann den Forced-Status.
     // Blöderweise kann ein Auftrag mehrere Kanäle enthalten und das ganze Kanal für Kanal unterschiedlich sein
     Buffer[Index].Mask &= ~((Buffer[Index].Bits ^ bits) & Schnittmenge);
     // Die Schnittmenge wird noch um die "Forced"-Schaltaufträge bereinigt für die gilt: neu=alt
     // Oder umgekehrt gesagt: Wenn (neu != alt) & Forced, dann das Bit in Mask erhalten.
     // Dazu dieses Bit aus der Schnittmenge entfernen.
     Schnittmenge &= ~((Buffer[Index].Bits ^ bits) & ChForcedSwMsk);
     // und dann aus dem neuen Auftrag herausgelöscht.
     mask &= ~Schnittmenge;
     // Der alte Auftrag oder der neue Auftrag (oder beide) könnten jetzt leer sein
     if (Buffer[Index].Mask == 0)
     {
      // alten Auftrag aus dem Fifo herauslöschen
      if (!DelBufEntry(Index))
      {
       break; // Ups, der Index ist nicht mehr gültig, da es keinen Eintrag danach mehr gibt
      }
      StillNotDone = true;
      //Es darf kein NextBufEntry aufgerufen werden, der Index ist schon weitergesetzt!
     }
     if (mask == 0)
     {
      break; // Die neue Maske ist leer, da braucht nicht weiter überprüft werden
     }
    }
    if (!StillNotDone)
    {
     StillNotDone = NextBufEntry(Index);
    }
   } while (StillNotDone);
  }
#ifdef RELAYUSEISR
  NVIC_EnableIRQ(RelTmrIRQn);
#endif
  // Für einen neuen Auftrag, der es durch die Bereinigung oben geschafft hat, gilt:
  // Es ist kein anderer, älterer Auftrag zu diesem Kanal in der Warteschlange vorhanden.
  // Noch nicht sichergestellt ist, ob der Schaltauftrag den Schaltzustand überhaupt verändern
  // würde. Aber das ist einfach: Einfach mit ActState vergleichen
  // Der Schaltauftrag wird nur eingetragen, wenn er den Zustand ändert oder den Forced-Status hat.
  mask &= ((ChRealSwStatus ^ bits) | ChForcedSwMsk);
 }
 // Ist nach der ganzen Überprüfung noch etwas übrig von Schaltauftrag?
 if (mask)
 {
  // Den Auftrag jetzt in die Queue eintragen
  // Als Folge des Aussortierens oben kann der Buffer nie überlaufen, es bleiben
  // immer höchstens so viele Aufträge übrig, wie Kanäle vorhanden sind.
  Buffer[BufWrPtr].Mask = mask;
  Buffer[BufWrPtr].Bits = bits & mask;
  NextBufIndex(BufWrPtr);
 }
}

void Relay::Start(unsigned short BusVFailureMask, unsigned short BusVFailureData)
{
 BusVFailMask = BusVFailureMask;
 BusVFailData = BusVFailureData;
 OpChgReq |= RELREQSTART;
}

void Relay::Stop(void)
{
 OpChgReq |= RELREQSTOP;
}

void Relay::BusVoltageFailed(void)
{
 // In den Zuständen VOR Operating ist nicht sichergestellt, dass genug Energie für
 // die Busspannungsausfall-Schaltaktion vorhanden ist.
 // Und in den Zuständen danach wurde der Schaltbetrieb schon eingestellt.
 if (OpState == RelOperatingStates::Operating)
 {
  OpChgReq |= RELREQBUSVFAIL;
  if (BusVFailMask) // Gibt es Kanäle, die eine bestimmte Stellung bei Busspannungsausfall einnehmen sollen?
  {
   bool StillNotDone = false;
   unsigned Index;
 #ifdef RELAYUSEISR
   NVIC_DisableIRQ(RelTmrIRQn);
 #endif
   // Schaltaufträge in der Warteschlange, die ebenfalls die Kanäle betreffen, die
   // in eine bestimmte Endstellung geschaltet werden sollen, werden gelöscht.
   // Da ein gespeicherter Schaltauftrag mehrere Kanäle betreffen kann, muss die
   // ganze Liste durchgegangen und gefiltert werden.
   if (FirstBufEntry(Index))
   {
    do {
     StillNotDone = false;
     if (Buffer[Index].Mask & BusVFailMask)
     {
      // Dieser Schaltauftrag enthält mindestens einen Kanal, der vom Brownout-Schaltauftrag betroffen ist.
      // Diese Bits rauslöschen
      Buffer[Index].Mask &= ~BusVFailMask;
      // Der alte Auftrag könnte jetzt leer sein
      if (Buffer[Index].Mask == 0)
      {
       // alten Auftrag aus dem Fifo herauslöschen
       if (!DelBufEntry(Index))
       {
        break; // Ups, der Index ist nicht mehr gültig, da es keinen Eintrag danach mehr gibt
       }
       StillNotDone = true;
       //Es darf kein NextBufEntry aufgerufen werden, der Index ist schon weitergesetzt!
      }
     }
     if (!StillNotDone)
     {
      StillNotDone = NextBufEntry(Index);
     }
    } while (StillNotDone);
   }
 #ifdef RELAYUSEISR
  NVIC_EnableIRQ(RelTmrIRQn);
 #endif
  }
 } else if (OpState == RelOperatingStates::MeasMode)
 {
  OpChgReq |= RELREQSTOP;
 }
 ChForcedSwMsk = 0;
 // Die gesamte "Forced" Geschichte zum Ausführen redundanter Relaispulse wird mit BusVFail hinfällig.
 // Die Forced-Flags werden nicht im Flash gesichert. Nach einem Neustart werden diese Aufträge als
 // normale Aufträge abgearbeitet. Dass sie dann durch gegenteilige Aufträge gelöscht werden können ist
 // kein Problem, da nach Systemstart recht bald Wiederholpulse ausgeführt werden.
}

//inline void Relay::MakeOneChRelData(unsigned &DriverData, unsigned ch, unsigned short Mask, unsigned short Bits)
//{
// if (Mask & (1 << ch))
// {
//  Mask &= ~(1 << ch);
//  if (Bits & (1 << ch))
//  {
//   ChRealSwStatus |= (1 << ch);
//   DriverData |= (RELAYPATTERNON << (2*ch));
//  } else {
//   ChRealSwStatus &= ~(1 << ch);
//   DriverData |= (RELAYPATTERNOFF << (2*ch));
//  }
// }
//}

/*
 * Rückgabewert True, wenn die Relaismimik im Funktionszustand "Operating" ist, d.h.
 * sie wurde über "Start" gestartet, die Vermessung des Energieverbrauchs eines Schaltvorgangs
 * wurde durchgeführt, die Ladung in der Speicherkondensatoren ist hoch genug, um auch die
 * evtl. Schaltvorgänge im Falle eines Busspannungsausfalls bedienen zu können.
 */
bool Relay::IsOperating(void)
{
 return (OpState == RelOperatingStates::Operating);
}

/*
 * Rückgabewert True, wenn die Busspannung nicht mehr ausreichend ist, um die Speicherrail auf
 * Mindestwert (in Bezug auf BusVoltageFailSwitching) zu laden. Dies ist dann bereits ein
 * BrownOut-Kriterium.
 */
bool Relay::BusVoltageFailRailLevel(void)
{
 int zw = max(GetBusVoltage() - ADCRAILVOLTAGELOSS, 0);
 return ((unsigned)(zw*zw) < (SingleSwitchEnergy*__builtin_popcount(BusVFailMask)+ADC12VOLTSQR));
}

int Relay::CalcAvailRelEnergy(void)
{
 unsigned zw = GetRailVoltage();
 zw = zw*zw - ADC12VOLTSQR; // Gesamtenergie minus verbleibende Energie bei 12V
 int RelEnergyAvail = 0;
 while (zw >= SingleSwitchEnergy)
 {
  zw -= SingleSwitchEnergy;
  RelEnergyAvail++;
  if (RelEnergyAvail >= (2*CHANNELCNT))
  {
   break; // Keine weiteren Durchläufe erforderlich, wenn mehr Energie als für zwei mal alle Kanäle zur Verfügung steht
  }
 }
 return RelEnergyAvail;
}

/*
 * Parameter time: Aktuelle Zeit, kann systemTime sein
 *                 Es bietet sich an eine Zeit im ms Raster zu benutzen, dies muss jedoch nicht sein.
 *                 So könnte die Zeit genauso das Timerraster widerspiegeln, in dem die Routine aufgerufen
 *                 wird. Dann müssen die Konstanten für die Wartezeiten natürlich ebenfalls nach dieser Einheit
 *                 ausgerichtet sein.
 * Es gibt eine Wartezeit direkt nach einer Bestromung der Relaisspule. Sie dient dazu, besser den
 * Ladezustand der Bulkkondensatoren ermitteln zu können. Daraus wird die Anzahl der anschließend möglichen
 * Schaltvorgänge ermittelt.
 * Rückgabewert true wenn neue SPI-Daten generiert wurden, also eine Übertragung notwendig wäre.
 */
bool Relay::DoSwitching(unsigned time, unsigned &RelDriverData)
{
 bool retval = false;
 IdleDetect(time);
 // Erst die Verwaltung der SubStates (aktuell laufender Puls, Wartezeit nach Puls...)
 // =================================
 if (SubState == RelSubStates::Pulse) // Es wird gerade eine/mehrere Relaisspulen bestromt
 {
#ifdef RELAYUSEDISCRETETIMING
  if ((signed)(time-NextPointInTime) >= 0)
  //if ((time-PulseStartTime) >= RELAYPULSEDURATION)
#else
  if ((signed)(time-NextPointInTime) > 0)
  //if ((time-PulseStartTime) > RELAYPULSEDURATION)
#endif
  {
   DriverData = 0;
   retval = true;
   SubState = RelSubStates::Delay;
   NextPointInTime = time + RELAYPOSTDELAY1;
  }
 }

 if (SubState == RelSubStates::Delay) // Wartezeit nach einem Ansteuerpuls für eine korrekte Vermessung
 {
#ifdef RELAYUSEDISCRETETIMING
  if ((signed)(time-NextPointInTime) >= 0)
  //if ((time-PulseStartTime) >= RELAYPOSTDELAY)
#else
  if ((signed)(time-NextPointInTime) > 0)
  //if ((time-PulseStartTime) > RELAYPOSTDELAY)
#endif
  {
   pwmEnable(false);

   SubState = RelSubStates::Delay2;
   NextPointInTime = time + RELAYPOSTDELAY2;
   if (OpState == RelOperatingStates::MeasMode)
   {
    unsigned zw1 = EnergyCalcRefVoltage;
    zw1 = zw1*zw1;
    // aktuelle Bulkspannung festhalten
    unsigned zw2 = GetRailVoltage();
    zw2 = zw2*zw2;
    if (zw1 > zw2)
    {
     // Benötigte Energie ausrechnen und abspeichern
     SingleSwitchEnergy = zw1 - zw2; // Nach Messungen keine Sicherheitsmarge erforderlich
    } else {
     // Problem... Einfach eine seeehr große Zahl annehmen.
     SingleSwitchEnergy = 100000;
    }
   }
  }
 }

 if (SubState == RelSubStates::Delay2) // Die noch folgende Wartezeit bis zum nächsten Puls
 {
#ifdef RELAYUSEDISCRETETIMING
  if ((signed)(time-NextPointInTime) >= 0)
  //if ((time-PulseStartTime) >= RELAYPOSTDELAY)
#else
  if ((signed)(time-NextPointInTime) > 0)
  //if ((time-PulseStartTime) > RELAYPOSTDELAY)
#endif
  {
   SubState = RelSubStates::Idle;
  }
 }

 // Folgend die Verwaltung der Operating States der Relay-Unit
 // ==========================================================
 if ((OpState == RelOperatingStates::MeasMode) && (SingleSwitchEnergy))
 { // Messung wurde bereits durchgeführt, warten bis erreichen der Mindestreserve für's BusVoltageFailureSwitching
  if (OpChgReq & (RELREQSTOP | RELREQBUSVFAIL))
  { // Wenn währenddessen der Start schon wieder abgeblasen wurde...
   OpState = RelOperatingStates::Disable;
   OpChgReq = 0;
  } else {
   if (CalcAvailRelEnergy() >= __builtin_popcount(BusVFailMask))
   {
    for (unsigned ch=0; ch<CHANNELCNT; ch++)
     PulseRepTmr[ch] = time; // Die PulseRepTmr werden auf "fällig" gesetzt
    OpState = RelOperatingStates::Operating;
   }
  }
 }

 int RelEnergyAvail=0;
 bool StartASwitch = false;

 if (OpState == RelOperatingStates::Disable)
 {
  if (OpChgReq & (RELREQSTOP | RELREQBUSVFAIL))
  {
   OpChgReq = 0; // Dann auch einen evtl StartRequest löschen
  }
  if (OpChgReq & RELREQSTART)
  {
   // Bulkspannung speichern
   EnergyCalcRefVoltage = GetRailVoltage();
   // Wenn Bulkspannung > fester Wert
   if (EnergyCalcRefVoltage > MINURAILINITVOLTAGE)
   {
    // Es wird das erste Relais bestromt mit der alten Schaltrichtung,
    // d.h. es wird nicht umgeschaltet. Dies dient nur zur Messung,
    // um wieviel die Bulkspannung dabei einbricht.
    if (ChRealSwStatus & 1)
    {
     DriverData = RELAYPATTERNON;
    } else {
     DriverData = RELAYPATTERNOFF;
    }
    PulseRepTmr[0] = time + RELAYREPPULSEDELAYLONG;
    OpState = RelOperatingStates::MeasMode;
    NextPointInTime = time + RELAYPULSEDURATION;
    SubState = RelSubStates::Pulse;
    retval = true;
    OpChgReq &= 0;
   }
  }
 }

 if ((OpState == RelOperatingStates::BusVFail) && (SubState == RelSubStates::Idle))
 {
  OpState = RelOperatingStates::Disable;
 }

 if ((OpState == RelOperatingStates::Operating) && (SubState == RelSubStates::Idle))
 {
  if ((OpChgReq & RELREQBUSVFAIL))
  {
   if (BusVFailMask)
   { // Es gibt Kanäle, die für ein BusVoltageFailureSwitching konfiguriert sind
    DriverData = 0;
    // Die Routine ist zwar ähnlich wie die "normale" Schaltroutine, doch leider zu unerschiedlich um sie ohne weiteres zu vereinigen.
    for (unsigned ch=0;ch<CHANNELCNT;ch++)
    {
     if (BusVFailMask & (1 << ch))
     {
      //BusVFailMask &= ~(1 << ch); unnötig
      if (BusVFailData & (1 << ch))
      {
       ChTargetSwStatus |= (1 << ch);
       ChRealSwStatus |= (1 << ch);
       DriverData |= (RELAYPATTERNON << (2*ch));
      } else {
       ChTargetSwStatus &= ~(1 << ch);
       ChRealSwStatus &= ~(1 << ch);
       DriverData |= (RELAYPATTERNOFF << (2*ch));
      }
     }
    }
    NextPointInTime = time + RELAYPULSEDURATION;
    SubState = RelSubStates::Pulse;
    OpState = RelOperatingStates::BusVFail;
    retval = true;
   } else { // kein BusVoltageFailureSwitching, dann geht es hier ganz einfach
    OpState = RelOperatingStates::Disable;
   }
   OpChgReq = 0;
  } else {
   if ((OpChgReq & RELREQSTOP))
   { // Es wurde ein Stop verlangt, dann gibt es kein BusVoltageFailureSwitching!
    OpState = RelOperatingStates::Disable;
    OpChgReq = 0;
   } else {
    if (BuffersNonEmpty())
    {
     // Für wie viele Relais reicht die gespeicherte Energie?
     RelEnergyAvail = CalcAvailRelEnergy() - __builtin_popcount(BusVFailMask);
     if (RelEnergyAvail > 0)
      StartASwitch = true;
    } else
     if (IdleDetect(time))
      if ((CalcAvailRelEnergy() - __builtin_popcount(BusVFailMask)) > 0)
      {
       // Nix los, Elkos voll.
       // Mal nachgucken, ob eine Pulswiederholung ansteht.
       // Wenn ein Kanal geschaltet worden ist, wird später noch mal ein Puls nachgelegt.
       int oldest_ch = -1;
       int oldest_age = -1;
       int age;
       for (int ch=0; ch < CHANNELCNT; ch++)
       {
        age = (signed)(time-PulseRepTmr[ch]);
        if (age >= 0)
        {
         if (age > oldest_age)
         {
          oldest_ch = ch;
          oldest_age = age;
         }
        }
       }
       if (oldest_ch >= 0)
       {
        PulseRepTmr[oldest_ch] = time + RELAYREPPULSEDELAYLONG;
        int mask = 1 << oldest_ch;
        if (ChRealSwStatus & mask)
        {
         DriverData = RELAYPATTERNON << (2*oldest_ch);
        } else {
         DriverData = RELAYPATTERNOFF << (2*oldest_ch);
        }
        NextPointInTime = time + RELAYPULSEDURATION;
        SubState = RelSubStates::Pulse;
        retval = true;
       }
      }
   }
  }
 }

 // Nachfolgend wird das Ansteuermuster für die Relaistreiber generiert.
 // Die Routine wird aus einem Auftrag so viele Schalthandlungen raushohlen, wie Energie zur Verfügung
 // steht (wenn gewünscht) oder umgekehrt den Schaltauftrag zusammenhalten und erst ausführen,
 // wenn genug Energie zur Verfügung steht (wenn RELAYKEEPTASKSTOGETHER definiert).

 // Die Routine kann auch mehrere Aufträge zusammenfassen, wenn die Energie reicht (unter Beachtung
 // von RELAYKEEPTASKSTOGETHER). Es ist durch DoEnqueue() sichergestellt, dass ein Kanal nie mehr als
 // einmal in der Warteschlage vorkommt, deshalb ist das unproblematisch.
 if (StartASwitch)
 {
  int RelEnergyNeeded;
  bool AnotherLoop = false;
  DriverData = 0;
  do
  {
#ifdef RELAYKEEPTASKSTOGETHER
   RelEnergyNeeded = __builtin_popcount(Buffer[BufRdPtr].Mask); // Zähle gesetzte Bits in .Mask
#else
   RelEnergyNeeded = 1;
#endif
   if (RelEnergyAvail < RelEnergyNeeded)
   {
    break;
   }
   for (unsigned ch=0;ch<CHANNELCNT;ch++)
   {
    if (Buffer[BufRdPtr].Mask & (1 << ch))
    {
     Buffer[BufRdPtr].Mask &= ~(1 << ch);
     ChForcedSwMsk &= ~(1 << ch);
     PulseRepTmr[ch] = time + RELAYREPPULSEDELAY;
     if (Buffer[BufRdPtr].Bits & (1 << ch))
     {
      ChRealSwStatus |= (1 << ch);
      DriverData |= (RELAYPATTERNON << (2*ch));
     } else {
      ChRealSwStatus &= ~(1 << ch);
      DriverData |= (RELAYPATTERNOFF << (2*ch));
     }
     if ((--RelEnergyAvail == 0) || (Buffer[BufRdPtr].Mask == 0))
     {
      break;
     }
    }
   }
   if (Buffer[BufRdPtr].Mask == 0)
   {
    AnotherLoop = NextBufEntry(BufRdPtr);
   }
  } while (AnotherLoop);
  if (DriverData != 0)
  {
   NextPointInTime = time + RELAYPULSEDURATION;
   SubState = RelSubStates::Pulse;
   retval = true;
  }
 }
 if (retval && (DriverData != 0))
  pwmEnable(true);
 RelDriverData = DriverData;
 return retval;
}

unsigned int Relay::GetData(void *data)
{
 byte* ptr=(byte *)data;
 unsigned RdPtr = BufRdPtr;
 PtrWrUint16(ptr, ChRealSwStatus);
 ptr+=2;
 PtrWrUint16(ptr, ChTargetSwStatus);
 ptr+=2;
 for (unsigned i=1;i<RELAYBUFLEN;i++) // Ein Eintrag bleibt immer frei, daher müssen auch nur RELAYBUFLEN-1 Einträge abgespeichert werden.
 {
  if (RdPtr == BufWrPtr)
  {
   PtrWrUint16(ptr, 0);
   ptr+=2;
   PtrWrUint16(ptr, 0);
   ptr+=2;
  } else {
   PtrWrUint16(ptr, Buffer[RdPtr].Bits);
   ptr+=2;
   PtrWrUint16(ptr, Buffer[RdPtr].Mask);
   ptr+=2;
   NextBufIndex(RdPtr);
  }
 }
 return (RELAYBUFLEN-1)*4+2*2; // 20 Bytes bei 4 Kanälen, 28 bei 6, 36 bei 8 Kanälen
}

inline unsigned short RelayStorageRd16(byte* &ptr, bool &Fail, unsigned short unusedmask)
{
 unsigned short data = PtrRdUint16(ptr);
 if (data & unusedmask)
  Fail = true;
 ptr+=2;
 return data;
}

unsigned int Relay::SetData(void *data)
{
 byte* ptr=(byte *)data;
 unsigned short rdata;
 unsigned unusedmask = ~((1 << CHANNELCNT)-1);
 bool fail = false;
 BufWrPtr = 0;
 BufRdPtr = 0;
 ChRealSwStatus = RelayStorageRd16(ptr, fail, unusedmask);
 ChTargetSwStatus = RelayStorageRd16(ptr, fail, unusedmask);
 for (unsigned i=1;i<RELAYBUFLEN;i++) // Ein Eintrag bleibt immer frei, daher müssen auch nur RELAYBUFLEN-1 Einträge gelesen werden.
 {
  Buffer[BufWrPtr].Bits = RelayStorageRd16(ptr, fail, unusedmask);
  rdata = RelayStorageRd16(ptr, fail, unusedmask);
  Buffer[BufWrPtr].Mask = rdata;
  if (rdata == 0)
   break;
  NextBufIndex(BufWrPtr);
 }
 if (fail)
 {
  BufWrPtr = 0;
  BufRdPtr = 0;
  ChRealSwStatus = 0;
  ChTargetSwStatus = 0;
 }
 return (RELAYBUFLEN-1)*4+2*2; // 20 Bytes bei 4 Kanälen, 28 bei 6, 36 bei 8 Kanälen
}

bool Relay::IdleDetect(unsigned time)
{
 int RailVoltage = GetRailVoltage();
 if ((OpState == RelOperatingStates::Operating) && (SubState == RelSubStates::Idle) && BuffersEmpty())
 {
  if (IdleDetected)
  {
   return true;
  } else {
   if ((signed)(RailVoltage - IdleDetRefVoltage) >= IDLEDETECTVOLT)
   {
    // Die Spannungsdifferenz ist zu groß, ist wohl nicht Idle
   } else if ((signed)(time-IdleDetTime) >= 0)
   {
    IdleDetected = true;
    return true;
   } else
    return false;
  }
 }
 IdleDetected = false;
 IdleDetRefVoltage = RailVoltage;
 IdleDetTime = time + IDLEDETECTTIME;
 return false;
}

void Relay::debugging(void)
{
 BufWrPtr = 3;
 BufRdPtr = 3;
}

//NVIC_EnableIRQ(TIMER_32_0_IRQn);
//NVIC_DisableIRQ(TIMER_32_0_IRQn);

void RelTestEnqueue(void)
{
 relay.debugging();
 relay.Switch(0, 1);
 relay.Switch(1, 1);
 relay.DoEnqueue();
 relay.Switch(1, 1); // Doppelauftrag, sollte gelöscht werden
 relay.Switch(2, 1);
 relay.DoEnqueue();
 relay.Switch(3, 1);
 relay.DoEnqueue();
 relay.Switch(4, 1);
 relay.DoEnqueue();
 relay.Switch(2, 0); // Sollte diesen und den vorvorigen Auftrag löschen
 relay.DoEnqueue();
}

