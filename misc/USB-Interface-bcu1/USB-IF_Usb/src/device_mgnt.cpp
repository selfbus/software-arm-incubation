/*
 *  device_mgnt.cpp - Device management, mode managemant and mode switching
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <GenFifo.h>
#include "chip.h"
#include "string.h"
#include "busdevice_if.h"
#include "BufferMgr.h"
#include "cdc_dbg.h"
#include "knxusb_const.h"
#include "device_mgnt.h"
#include "device_mgnt_const.h"
#include "error_handler.h"

DeviceManagement devicemgnt;

DeviceManagement::DeviceManagement(void)
{
    txtimeout = 0;
    rxtimeout = 0;
    LastMode = TCdcDeviceMode::Halt;
    KnxActive = false;
}

bool DeviceManagement::KnxIsActive(void)
{
    return KnxActive;
}

void DeviceManagement::SysIf_Tasks(bool UsbActive)
{
    if (CdcDeviceMode == TCdcDeviceMode::ProgBusChip)
    {
        if (dev_rxfifo.Empty() != TFifoErr::Empty)
        {
            int buffno;
            dev_rxfifo.Pop(buffno);
            buffmgr.FreeBuffer(buffno);
        }
        LastMode = TCdcDeviceMode::ProgBusChip;
        rxtimeout = systemTime;
        txtimeout = systemTime;
        return;
    }

    if (dev_rxfifo.Empty() != TFifoErr::Empty)
    {
        int buffno;
        dev_rxfifo.Pop(buffno);
        uint8_t *ptr = buffmgr.buffptr(buffno);
        uint8_t DevPacketLength = ptr[0];
        if ((ptr[2+IDX_HRH_Id] == C_HRH_IdDev) && (DevPacketLength == C_Dev_Packet_Length))
        {
            rxtimeout = systemTime + C_RxTimeout; // Wird bei jedem Paket an dieses If gesetzt.
            KnxActive = true;
            switch (ptr[2+IDX_HRH_Id+1])
            {
                case C_Dev_Idle:
                    break;

                case C_Dev_Isp:
                    // Wird von der KNX-Seite verschickt, wenn gerade ein Datenpaket Programmierdaten
                    // vollständig zum Zieldevice verschickt worden ist.
                    cdcdbgif.reEnableReceive();
                    break;

                default:
                    // Etwas anderes sollte von KNX-Seite gar nicht kommen
                    failHardInDebug();
                    break;
            }
        }
        buffmgr.FreeBuffer(buffno);
    }

    if ((int)(systemTime - rxtimeout) > 0)
    {
        /* Timeout, anscheinen ist die KNX-Seite nicht funktionsfähig
         *   -> löscht ein Flag, das wird für das HID-Interface benötigt
         */
        KnxActive = false;
        // todo send a BAS_FeatureId::BusConnStat with BAS_ServiceId::FeatureInfo
        // to indicate that the bus is not active
    }

    TCdcDeviceMode mode;
    if (UsbActive)
        mode = CdcDeviceMode;
    else
        mode = TCdcDeviceMode::Halt;


    int deltaMs = (int)(systemTime - txtimeout);
    if ((deltaMs <= 0) && (LastMode == mode))
    {
        // everything is fine, no tx timeout or mode change
        return;
    }

    // Idle Paket Richtung USB-Seite verschicken
    int buffno = buffmgr.AllocBuffer();
    if (buffno < 0)
    {
        failHardInDebug();
        return;
    }

    uint8_t *buffptr = buffmgr.buffptr(buffno);
    *buffptr++ = C_Dev_Packet_Length;
    buffptr++;
    *buffptr++ = C_HRH_IdDev;
    *buffptr++ = C_Dev_Sys;
    LastMode = mode;
    switch (LastMode)
    {
        case TCdcDeviceMode::Halt:
        case TCdcDeviceMode::ProgBusChip:
            *buffptr++ = C_DevSys_Disable;
            KnxActive = false;
            break;

        case TCdcDeviceMode::HidOnly:
        case TCdcDeviceMode::UsbMon:
            *buffptr++ = C_DevSys_Normal;
            break;

        case TCdcDeviceMode::BusMon:
            *buffptr++ = C_DevSys_CdcMon;
            break;

        case TCdcDeviceMode::ProgUserChip:
            *buffptr++ = C_DevSys_UsrPrg;
            break;

        default:
            *buffptr++ = 0; //this should never happen. It´s the same 0 as TCdcDeviceMode::Halt
            failHardInDebug();
            break;
    }

    if (ser_txfifo.Push(buffno) != TFifoErr::Ok)
    {
        buffmgr.FreeBuffer(buffno);
    }
    txtimeout = systemTime + C_IdlePeriod;
}

    ///\todo handle communication timeouts and buffer-overflows, see below
    // Uart-Schnittstelle zur KNX-Seite
    /*
     * CDC & HID:
     * Wie vorgehen, wenn einer oder beide Endpoints keine Daten abnehmen?
     * Das ist der zu erwartende Fall, wenn das entsprechende Interface gar nicht
     * geöffnet ist.
     * Auf USB_IsConfigured(hUsb) testen, das sollte klar sein. Aber das sagt wohl nicht
     * sicher, ob das Device geöffnet ist. Oder verwirft der USB-Stack ansonsten einfach
     * die Daten, arbeitet aber ansonsten?
     * Das entsprechende Fifo läuft voll, dabei werden viele oder alle Buffer belegt.
     * Bis jetzt würde kein neuer Buffer angefragt oder kein neuer Buffer in den vollen
     * Fifo eingereiht werden. Das führt zu veralteten Daten im Fifo und u.U. dazu, dass
     * auch das aktive Interface nicht mehr funktioniert - weil kein freier Buffer mehr
     * verfügbar ist.
     * Ziel: Alte Buffer im Fifo werden bevorzugt gelöscht.
     *  -> Anstatt den neuen Buffer zu droppen, einfach den ältesten Eintrag im Fifo droppen
     * Ziel: Wenn später das Interface aktiviert wird, werden keine veralteten Daten
     *       verschickt. Für das User-Programm ist es leider nicht klar erkennbar, wann
     *       ein Interface aktiv ist bzw aktiviert wird.
     *       Timeout z.B. wenn eine Sekunde kein Datentransfer stattfand, aber der Fifo nicht leer ist?
     *       Dann Fifo leeren. Muss der aktuell anstehende Transfer abgebrochen werden?
     *       Die Knx-Seite muss das erfahren, damit z.B. der HID-Busmonitor Mode beendet wird.
     *       Was wenn zufälligerweise kein Transfer notwendig war, während das If disconnected war?
     *       -> Die HID-Seite hat mehr Möglichkeiten festzustellen, ob verbunden. Siehe Beispiele
     *       auch von Microchip!
     * Ziel: Es bleiben genug Buffer für das andere Interface frei.
     *  -> Deutlich mehr Buffer als Fifo-Tiefe. 16 Buffer sind nur 1056 Bytes, der Chip hat
     *     8kB Main-SRAM. Die Fifos sind 4 Stufen tief.
     */
    /*
     * Letzendlich bleibt unklar, ob der Host-Treiber bereits die Daten abholt, wenn das Device
     * konfiguriert ist. Das HID-Device sieht beispielsweise Start of Frame Events, obwohl das
     * Device nicht vom Programm geöffnet worden ist.
     * Also erst mal vereinfacht: Solange USB_IsConfigured() gehe ich davon aus, dass die Daten
     * vom Host auch abgeholt werden. Wenn nicht USB_IsConfigured(), dann Fifo und neue Aufträge
     * löschen. Wenn das später zu Problemen führt, wird das Ganze noch mal geändert.
     */

    /*
     * Die KNX-Seite muss die Änderung USB_IsConfigured mitbekommen
     * Ebenso steht die Übertragung von RTS/DTR noch aus
     */

