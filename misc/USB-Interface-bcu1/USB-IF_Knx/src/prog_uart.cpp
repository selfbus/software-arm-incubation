/*
 *  prog_uart.cpp - Soft-Uart of the User Programming interface
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <sblib/digital_pin.h>
#include "GenFifo.h"
#include "BufferMgr.h"
#include "knxusb_const.h"
#include "device_mgnt_const.h"
#include "prog_uart.h"
#include "error_handler.h"

#define RX_BYTE_MAX_COLLECT 16 //!< Number of bytes to receive (collect) before queuing them to transmit over hardware uart/serial

/**
 * Baudrate of the softUART for Prog-If
 * @details According to NXP AN10955 with 48MHz system clock an softUART could transmit with 57600 and receive with 19200.
 *          In reality the current implementation only works reliably with Tx/Rx 9600 baud.
 * @note  https://www.nxp.com/docs/en/application-note/AN10955.pdf
 */
#define BAUDRATE 9600

// Zeit eines Bits in us
#define BIT_TIME (1000000/BAUDRATE)

// Zeit eines halben Bits
#define HALFBIT_TIME (BIT_TIME/2)

// Empfänger Timeout, nachdem ein Paket als abgeschlossen betrachtet wird
#define REC_TIMEOUT (BIT_TIME*10)

// Wert des Vorteilers, um einen 1us Tick zu erhalten
#define TIMER_PRESCALER (SystemCoreClock / 1000000 - 1)

ProgUart proguart(timer32_0, TIMER32_0, PIO2_9, PIO0_11, CAP0, MAT0, MAT3, PIO1_10, PIO0_8);

// arx_CaptureCh braucht'nen Pin, den RX-Pin
// arx_MatchCh ist intern
// atx_MatchCh braucht'nen Pin, den TX-Pin
// Am Selfbus TS-ARM sind die Pins des Timer32_0 entsprechend herausgeführt (die des 16_0 z.B. nicht)

ProgUart::ProgUart(Timer& aTimer, int aTimerNum, int aRxPin, int aTxPin, TimerCapture arx_CaptureCh, TimerMatch arx_MatchCh, TimerMatch atx_MatchCh, int aIspEnPin, int aIspRstPin)
:timer(aTimer)
{
    timerNum = aTimerNum;
    rxPin = aRxPin;
    txPin = aTxPin;
    rx_captureCh = arx_CaptureCh;
    rx_matchCh = arx_MatchCh;
    tx_matchCh = atx_MatchCh;
    IspEnPin = aIspEnPin;
    IspRstPin = aIspRstPin;
    Enabled = false;
    initialize();
}

void ProgUart::initialize()
{
    pinMode(txPin, INPUT | PULL_UP);
    pinMode(rxPin, INPUT | HYSTERESIS | PULL_UP);
    rxbuffno = -1;
    txbuffno = -1;
    rxbitcnt = 0;
    txbitcnt = 0;
    rxlen = 0;
    txlen = 0;
    IspLines = 0;
    UpdIspLines();
    rxbyte = 0;
    txbyte = 0;
    txptr = nullptr;
    rxptr = nullptr;
}

void ProgUart::EnableUart(void)
{
    initialize();
    timer.setIRQPriority(0); // ensure highest IRQ-priority for the uart timer
    timer.begin();
    timer.captureMode(rx_captureCh, FALLING_EDGE | INTERRUPT);
    timer.start();
    timer.interrupts();
    timer.prescaler(TIMER_PRESCALER);

    timer.matchMode(tx_matchCh, SET);
    timer.match(tx_matchCh, 0xffff);

    // So wird der Sende-Ausgang auf 1 gesetzt, siehe auch EIB/bus.cpp
    timer.value(0xffff);
    while (timer.getMatchChannelLevel(tx_matchCh) == false)
        ;

    pinMode(txPin, OUTPUT_MATCH);
    pinMode(rxPin, INPUT_CAPTURE | HYSTERESIS | PULL_UP);
}

void ProgUart::DisableUart(void)
{
    timer.noInterrupts();
    initialize();
}

void ProgUart::Enable(void)
{
    Enabled = true;
    EnableUart();
    UpdIspLines();
}

void ProgUart::Disable(void)
{
    Enabled = false;
    DisableUart();
    UpdIspLines();
}

void ProgUart::SetIspLines(uint8_t data)
{
    IspLines = data;
    UpdIspLines();
}

void ProgUart::UpdIspLines(void)
{
    if (Enabled)
    {
        // Zustände setzen
        digitalWrite(IspEnPin, (IspLines & 2) != 0);
        digitalWrite(IspRstPin, (IspLines & 1) != 0);
        pinMode(IspEnPin, OUTPUT);
        pinMode(IspRstPin, OUTPUT);
    } else {
        // einen Disabled-Zustand setzen. High-Z? Auf alle Fälle sollte der User-Chip laufen
        pinMode(IspEnPin, INPUT | PULL_UP);
        pinMode(IspRstPin, INPUT | PULL_UP);
        IspLines = 3;
    }
}

extern "C" void TIMER32_0_IRQHandler(void)
{
    proguart.timerInterruptHandler();
}

/*
 * RxD Simpelvariante:
 * Die fallende Flanke des Startbit wird gecaptured. Die Mitte des Startbits und die folgenden
 * Bits werden über Timerinterrupt per ISR eingesammelt.
 *
 * RxD Idle: Über Capture wird die fallende Flanke detektiert. Anschließend soll die
 * Mitte des Startbits abgepasst werden. Dafür wird eine halbe Bitzeit später ein Timer-Match
 * programmiert, der lediglich die ISR aufruft.
 *
 * RxD empfängt: Sollte das Startbit gesampelt werden? Dann nur auf 0 testen. Ansonsten Bit
 * reinshiften. Wenn noch nicht alle Bits drin, dann nächste Matchzeit + 1 Bitzeit setzen.
 * Wenn Stopbit detektiert worden, dann Byte in Buffer schreiben, Schreibadresse inkrementieren.
 * Zu viele Bytes empfangen -> Buffer abschließen und in Fifo einreihen.
 *
 * RxD Timeout: Wird benutzt, um eine abgeschlossene RxD-Sequenz zu erkennen. (Wenn die Maximalgröße
 * nicht erreicht oder halt überschritten wird.) Dazu kann der gleiche Match-Kanal verwendet werden.
 * Er wird benutzt, wenn gerade kein Empfang stattfindet. Die Timeout-Zeit ist 1 bis 2 Byte. Schlägt dieser
 * Timeout an, wird ebenfall der Buffer abgeschlossen und in den Fifo eingereiht.
 *
 * Senderichtung:
 * Wenn der Transmitter Idle ist und ein Buffer zum versenden vorhanden, dann wird irgendwie die ISR
 * getriggert.
 * In der ISR:
 * TxD Idle: Wenn im Buffer noch ungesendete Daten vorhanden sind, Startbit anlegen und nächsten Pegelwechsel
 * über ein Match-Register programmieren. (Oder selbst das Startbit über Match anlegen lassen?) Der nächste
 * Match ist nicht zwangsläufig eine Bitzeit entfernt, bei aufeinanderfolgenden gleichen Bits kann das auch
 * später sein.
 * TxD Busy: Immer wieder den nächsten Pegelwechsel programmieren. Als letztes eine Match-Zeit nach dem Ende
 * des Stopbit. Wenn weitere Daten vorhanden sind, dann nächstes Startbit vorbereiten. Wenn der Buffer nun
 * leer ist, muss eine Bestätigung zur USB-Seite zurückgesendet werden. Dafür den alten Buffer verwenden und
 * kurz modifizieren.
 */
void ProgUart::timerInterruptHandler()
{
    // rx_captureCh Gebraucht zur Erkennung der fallenden Flanke Startbit
    // rx_matchCh   Erzeugt die Ints bei den Bits und bei einem Rx-Timeout
    // tx_matchCh   Erzeugt die Flankenwechsel am Sendeausgang
    if (timer.flag(tx_matchCh)) // Nach einem Pegelwechsel
    {
        uint32_t time = timer.match(tx_matchCh);
        timer.resetFlag(tx_matchCh);
        if (txbitcnt != 1)
        {
            bool level = false;
            if (txbitcnt != 11)
            {
                level = (txbyte & 0x01) != 0;
                txbyte = (txbyte >> 1) + 128;
            }
            time += BIT_TIME;
            txbitcnt--;
            while ((txbitcnt != 1) && (level == ((txbyte & 0x01) != 0)))
            {
                txbyte = (txbyte >> 1) + 128;
                time += BIT_TIME;
                txbitcnt--;
            }
            timer.match(tx_matchCh, time);
            if (txbitcnt != 1)
            {
                if (level)
                    timer.matchMode(tx_matchCh, CLEAR | INTERRUPT);
                else
                    timer.matchMode(tx_matchCh, SET | INTERRUPT);
            }
            else
            {
                timer.matchMode(tx_matchCh, INTERRUPT);
            }
        }
        else
        {
            // Ende des Stopbits ist erreicht
            txbitcnt = 0;
            timer.matchMode(tx_matchCh, DISABLE);
        }
    }

    if ((txbuffno >= 0) && (txbitcnt == 0))
    {
        if (txlen != 0)
        {
            txbyte = *txptr++;
            txlen--;
            txbitcnt = 11;
            // Programmiere fallende Flanke Startbit
            uint32_t time = timer.value() + BIT_TIME;
            timer.match(tx_matchCh, time);
            timer.matchMode(tx_matchCh, INTERRUPT | CLEAR);
        }
        else
        {
            // Buffer wurde versendet
            timer.captureMode(tx_matchCh, DISABLE);
            // Buffer kann für Bestätigungsantwort verwendet werden
            txptr = buffmgr.buffptr(txbuffno);
            *txptr++ = C_Dev_Packet_Length;
            txptr++;
            *txptr++ = C_HRH_IdDev;
            *txptr++ = C_Dev_Isp;
            *txptr++ = 0;
            if (ser_txfifo.Push(txbuffno) != TFifoErr::Ok)
            {
                failHardInDebug();
                buffmgr.FreeBuffer(txbuffno);
            }
            txbuffno = -1;
        }
    }

    bool rx_bytedone = false;
    bool rx_timeout = false;
    if (timer.flag(rx_captureCh))  // Ereignis an RxD
    {
        timer.captureMode(rx_captureCh, DISABLE);
        if (rxbitcnt == 0) // Läuft schon ein Empfang?
        {   // Nein, noch nicht
            uint32_t time = timer.capture(rx_captureCh);
            timer.match(rx_matchCh, HALFBIT_TIME + time);
            //timer.restart();
            timer.matchMode(rx_matchCh, INTERRUPT);
            rxbitcnt = 10;
            if (rxbyte != 0)
            {
                failHardInDebug(); // this should never happen
            }
            rxbyte = 0;
        }
        else
        {
            failHardInDebug(); // this should never happen
        }
        timer.resetFlag(rx_captureCh);
    }

    if (timer.flag(rx_matchCh)) // Mitte einer Bitzeit am seriellen Empfänger ODER Timeout
    {
        bool rx_rearm = false;
        if (rxbitcnt == 0)
        {
            // Timeout
            rx_timeout = true;
            timer.matchMode(rx_matchCh, DISABLE);
            // Kein Rearm, also sollte der Timeout nicht wiederholt auftreten
        }
        else if (rxbitcnt == 1)
        {
            rxbitcnt = 0;
            // Stopbit
            if (digitalRead(rxPin))
            { // Stopbit empfangen
                rx_bytedone = true;
            }
            else
            {
                failHardInDebug();
            }
            timer.matchMode(rx_matchCh, DISABLE);
            rx_rearm = true;
        }
        else if (rxbitcnt == 10)
        {
            // Startbit
            if (digitalRead(rxPin))
            { // Doch kein Startbit
                rxbitcnt = 0;
                timer.matchMode(rx_matchCh, DISABLE);
                rx_rearm = true;
            }
        }
        else
        {
            // Datenbit
            rxbyte = (rxbyte >> 1) | (digitalRead(rxPin)?128:0);
        }

        if (rxbitcnt > 1)
        {
            uint32_t time = timer.match(rx_matchCh);
            timer.match(rx_matchCh, BIT_TIME+time);
            rxbitcnt--;
        }
        timer.resetFlag(rx_matchCh);

        if (rx_rearm)
        {
            // Timeout-Timer setzen
            uint32_t time = timer.match(rx_matchCh);
            timer.match(rx_matchCh, REC_TIMEOUT + time);
            timer.matchMode(rx_matchCh, INTERRUPT);
            timer.captureMode(rx_captureCh, FALLING_EDGE | INTERRUPT);
        }
    }

    if (rx_bytedone)
    {
        if (rxbuffno < 0)
        {
            rxbuffno = buffmgr.AllocBuffer();
            if (rxbuffno >= 0)
            {
                rxlen = 0;
                rxptr = buffmgr.buffptr(rxbuffno);
                *rxptr++ = 3; // Länge, Wird bei Abschluss des Pakets noch mal aktualisiert
                *rxptr++ = 0; // Checksumme wird im Uart-Transceiver Richtung USB gerechnet
                *rxptr++ = 2; // Das Paket als CDC-Paket kennzeichnen
            }
        }
        if (rxbuffno >= 0)
        {
            *rxptr++ = rxbyte;
            rxlen++;
        }
        rxbyte = 0;
    }

    if ((rxlen >= RX_BYTE_MAX_COLLECT) || (rx_timeout && (rxlen > 0))) // Um die Verzögerung klein zu halten, wird die Paketlänge auf RX_BYTE_MAX_COLLECT begrenzt
    {
        rxptr = buffmgr.buffptr(rxbuffno);
        *rxptr = rxlen+3; // Die Länge setzen
        if (ser_txfifo.Push(rxbuffno) != TFifoErr::Ok)
        {
            failHardInDebug();
            buffmgr.FreeBuffer(rxbuffno);
        }
        rxbuffno = -1;
        rxlen = 0;
    }
}

bool ProgUart::TxBusy(void)
{
    return (txbuffno >= 0);
}

bool ProgUart::rxBusy(void)
{
    return (rxbuffno >= 0);
}

TProgUartErr ProgUart::TransmitBuffer(int buffno)
{
    if (TxBusy())
    {
        return TProgUartErr::Busy;
    }

    uint8_t *ptr = buffmgr.buffptr(buffno);
    uint8_t len = *ptr;
    if ((len < 2) || (len > 67))
    {
        return TProgUartErr::Error;
    }

    timer.noInterrupts();
    txptr = buffmgr.buffptr(buffno);
    txlen = *txptr-3;
    txptr += 3; // Längenangabe, Checksumme und CDC-Id überspringen
    txbuffno = buffno;
    timer.interrupts();
    NVIC_SetPendingIRQ((IRQn_Type) (TIMER_16_0_IRQn + timerNum));

    return TProgUartErr::Ok;
}

void ProgUart::SerIf_Tasks(void)
{
    if (TxBusy())
    {
        return;
    }

    if (rxBusy())
    {
        return;
    }

    if (cdc_txfifo.Empty() == TFifoErr::Empty)
    {
        return;
    }

    int buffno;
    cdc_txfifo.Pop(buffno);
    if (!Enabled)
    {
        buffmgr.FreeBuffer(buffno);
        return;
    }

    TProgUartErr err = TransmitBuffer(buffno);
    if (err != TProgUartErr::Ok)
    {
        // Momentan als einzige Fehlerbehandlung: Paket verwerfen
        failHardInDebug();
        buffmgr.FreeBuffer(buffno);
        buffno = -1;
    }
}
