# TP-UART 2 Emulator für das Selfbus SB-Interface (LPC1115)

Macht aus einem Selfbus SB-Interface (LPC1115) einen KNX-TP1-Transceiver, der auf
seiner UART das **Host-Protokoll eines TP-UART 2** spricht. Damit lässt er
sich von beliebiger TP-UART-fähiger Software ansteuern:

* **knxd** – Treiber `tpuart`

* ETS über knxd, Calimero, u. a.

Die KNX-Seite (Bit-Timing, Kollisionsauflösung, ACK, Wiederholungen) übernimmt die
[Selfbus-Bibliothek](https://github.com/selfbus/software-arm-lib) (`sblib`).

---

## Hardware

| SB-Interface (LPC1115)    | Host |
| ------------------------- | ---- |
| `PIN_TX` (Default PIO1_7) | RX   |
| `PIN_RX` (Default PIO1_6) | TX   |
| GND                       | GND  |

Pegel: 3,3 V TTL. Pins sind in [src/config.h](src/config.h) änderbar.

Host-seitig: **19200 Baud, 8 Datenbits, gerade Parität, 1 Stoppbit (8E1)**.

---

## Build

Das Projekt ist eine reguläre sblib-Anwendung und wird wie die übrigen
Selfbus-Apps mit MCUXpresso gebaut:

1. `software-arm-incubation` klonen (inkl. Submodul `software-arm-lib`).
2. `misc/TPUART2-Emu` klonen.
3. Build-Symbole: identisch zu ft12, zusätzlich nichts erforderlich.`BUSMONITOR` darf **nicht** gesetzt sein.

---

## Unterstützte Services

### Host → Transceiver

| Code                   | Service                         | Verhalten                                |
| ---------------------- | ------------------------------- | ---------------------------------------- |
| `0x01`                 | `U_Reset.req`                   | Reset, antwortet mit `0x03`              |
| `0x02`                 | `U_State.req`                   | antwortet mit `U_State.ind`              |
| `0x03` / `0x04`        | `U_SetBusy` / `U_QuitBusy`      | ACK auf dem Bus aus/ein                  |
| `0x05`                 | `U_ActivateBusmon`              | Busmonitor, ACK aus (Ende nur per Reset) |
| `0x08`–`0x0C`          | `U_L_DataOffset`                | Offset für Frames > 64 Byte              |
| `0x0D`                 | `U_SystemState`                 | antwortet `0x4B` + Statusbyte            |
| `0x0E` / `0x0F`        | `U_StopMode` / `U_ExitStopMode` | `bus.pause()` / `bus.resume()`           |
| `0x10`–`0x17`          | `U_AckInformation`              | wird konsumiert, siehe *ACK-Verhalten*   |
| `0x24` + 1 Byte        | `U_MxRstCnt`                    | Wiederholungszähler, wird konsumiert     |
| `0x28` + 2 Byte        | `U_SetAddress` (knxd)           | setzt die eigene IA                      |
| `0xF1` + 2 Byte        | `U_SetAddress` (OpenKNX)        | setzt die eigene IA                      |
| `0x40`–`0x7F` + 1 Byte | `U_L_DataEnd`                   | letztes Frame-Oktett, löst Senden aus    |
| `0x80`–`0xBF` + 1 Byte | `U_L_DataStart/Cont`            | Frame-Oktett n                           |

`U_ProductId`, `U_Configure`, `U_IntRegRd/Wr`, `U_SetRepetition` und unbekannte
Codes werden inklusive ihrer Datenbytes verworfen, ohne den Parser zu
desynchronisieren.

`U_State.req` und `U_SetAddress` sind **Pflicht**:
Der OpenKNX-Stack sendet beide sekündlich (`requestState()`/`requestConfig()`) und erklärt die Verbindung nach 5 s ohne Antwort für tot – ab dann verwirft er jedes
`L_Data.req` stillschweigend.
Knxd nutzt `U_State.req` als 10-s-Keepalive.

### Transceiver → Host

| Code            | Service                        |
| --------------- | ------------------------------ |
| `0x03`          | `U_Reset.ind`                  |
| `0x07` \| Flags | `U_State.ind`                  |
| `0x2B`          | `U_StopMode.ind`               |
| `0x4B` + 1 Byte | `U_SystemStat.ind`             |
| `0x8B` / `0x0B` | `L_Data.con` positiv / negativ |
| Rohframe        | `L_Data.ind` inkl. Prüfsumme   |

---

## Bewusste Abweichungen von einem echten TP-UART

Diese Punkte ergeben sich aus der Architektur von `sblib` und sind für den
Betrieb relevant.

### 1. ACK-Verhalten: autonom, nicht host-gesteuert

Ein echter TP-UART streamt die ersten Frame-Oktette sofort zum Host, der
innerhalb des Frames mit `U_AckInformation` antwortet. `sblib` meldet ein
Telegramm erst, wenn es **vollständig** empfangen ist – zwischen
`Bus::handleTelegram()` und dem ACK-Slot liegen nur ca. 13 Bitzeiten (≈ 1,4 ms),
zu wenig für einen UART-Roundtrip.

Deshalb quittiert die Bibliothek selbst.
Konkret: mit deaktiviertem Transport Layer quittiert `sblib` **jedes** empfangene Telegramm (Gruppen- *und* Individualadressen). `U_AckInformation` vom Host wird gelesen und verworfen.

Praktische Folge: Ein Telegramm an eine physikalische Adresse, die es auf der
Linie gar nicht gibt, wird trotzdem quittiert. ETS erkennt dadurch „Gerät nicht
vorhanden" nicht mehr zuverlässig, wenn nur dieses Interface an der Linie hängt.
Das ist dasselbe Verhalten wie beim Selfbus-FT1.2-Interface.

### 2. Kein Echo gesendeter Telegramme

Ein echter TP-UART spiegelt gesendete Frames als `L_Data.ind` zurück. `sblib`
gibt gesendete Telegramme nicht heraus. Sowohl knxd (`recvecho`) als auch der
OpenKNX-Stack (`_isEcho`) verwerfen das Echo ohnehin.

### 3. `L_Data.con` immer positiv

`sblib` meldet das Sendeergebnis nicht nach außen
(`TLayer4::finishedSendingTelegram()` ist nicht virtuell). Nach Abschluss der
Übertragung wird `0x8B` gesendet. `sblib` wiederholt intern bereits bei NACK und
BUSY (je 3×). Nur bei Zeitüberschreitung (`TPUART_TX_CONFIRM_TIMEOUT_MS`, 400 ms)
kommt `0x0B` plus gesetztes `TRANSMIT_ERROR` im nächsten `U_State.ind`.

### 4. Nur Standard-Frames

Die `Bus`-Zustandsmaschine von `sblib` implementiert aktuell keine Extended Frames. Frames mit gelöschtem Bit 7 im Kontrollbyte werden mit negativem `L_Data.con` und
`PROTOCOL_ERROR` abgewiesen.

### 5. Quellandresse

`Bus::prepareTelegram()` überschreibt die Absenderadresse mit
`bcu->ownAddress()`. Der Emulator setzt deshalb vor jedem Senden
`setOwnAddress()` auf die Quelladresse des Host-Frames. So bleibt das Telegramm
byte-identisch. Dies ist eine Voraussetzung für Tunneling mit mehreren Individualadressen.

### 6. Gedrosselte Ausgabe

`sblib` liefert ein Telegramm am Stück, ein echter TP-UART im Bustakt. Die
Ausgabe wird deshalb auf **1 Oktett pro Millisekunde** gedrosselt
(`TPUART_TX_PACING_MS`). Das kostet keinen Durchsatz – TP1 ist mit 9600 bit/s
langsamer – und hält die Emulation dicht am Verhalten echter Hardware.

Relevant ist das vor allem für die **alte** OpenKNX-Implementierung
(`OpenKNX/knx`, `src/knx/tpuart_data_link_layer.cpp`). Sie verwirft den
Frame-Start, wenn beim ersten Byte schon mehr als `OVERRUN_COUNT` (7) Oktette
im RX-Puffer liegen. Die aktuelle Implementierung von `thelsing/knx` kennt
diese Prüfung nicht mehr; sie arbeitet mit `TpFrame` und einer 2-ms-Lücke zur
Resynchronisation.

Auf `0` setzen für ungedrosselte Ausgabe, wenn der Host Bursts verträgt
(knxd und der aktuelle OpenKNX-Stack tun das; beim alten Stack zusätzlich
`-DOVERRUN_COUNT=64` setzen).

---

## Host-Konfiguration

### knxd

```ini
[B.tpuart]
device = /dev/ttyKNX1
driver = tpuart
```

Oder auf der Kommandozeile:

```
knxd -e 1.0.240 -E 1.0.241:8 -D -T -R -S -b tpuart:/dev/ttyKNX1
```

ttyKNX1 ist das per Regel gemappte serielle Interface (siehe knxd Doku).

knxd schickt alle 10 s ein `U_State.req` als Keepalive – das wird beantwortet.

### OpenKNX-Stack

In der `platformio.ini`:

* `-DNCN5120` **weglassen** (sonst erwartet der Stack `U_Configure`,
  Marker-Mode, Baudratenumschaltung auf 38400 und die Analogregister)
* `-DKNX_BAUDRATE` weglassen, der Default 19200 ist korrekt
* UART auf `19200, SERIAL_8E1`
