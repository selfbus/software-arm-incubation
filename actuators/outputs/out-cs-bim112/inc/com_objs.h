#define OFSGENERALOBJECTS 0

#define OBJ_OPERATIONAL 0  // 1 bit   - Tx - Trigger - In Operation, sendet "1"
#define OBJ_SAFETYPRIO1 1  // 1 bit   - Rx - Trigger - Safety Priority 1 Objekt, erwarteter Wert abh. von Konfig
#define OBJ_SAFETYPRIO2 2  // 1 bit   - Rx - Trigger - Safety Priority 2 Objekt, erwarteter Wert abh. von Konfig
#define OBJ_SAFETYPRIO3 3  // 1 bit   - Rx - Trigger - Safety Priority 3 Objekt, erwarteter Wert abh. von Konfig

#define OFSCHANNELOBJECTS 10
#define SPACINGCHANNELOBJECTS 20
// Objektnummer = OFSCHANNELOBJECTS + (Kanal-1)*SPACINGCHANNELOBJECTS + Objektoffset
#define OBJ_SWITCH      0  // 1 bit   - Rx - Trigger - Das eigentliche "Switch"-Objekt, "1" Einschalten, "0" Ausschalten, invertierung mittels NO/NC-Wahl möglich
#define OBJ_PERMANENTON 1  // 1 bit   - Rx - Zustand - Dauerhaft-Ein
#define OBJ_DISTIMEFCT  2  // 1 bit   - Rx - Zustand - Zeitfunktionen deaktivieren, bei "1" wird jegliche Zeitfunktion deaktiviert und übergangen. Es wird keine Umschaltung ausgelöst. Umgekehrt bei Rückkehr zu "0"
#define OBJ_TIMDURATION 3  // 2 Bytes -RxRd- Zustand - Dauer der Treppenlichtfunktion in Sekunden. Kann beschrieben und auch rückgelesen werden.
#define OBJ_WARNSTAIRL  4  // 1 bit   - Tx - Trigger - Vorwarnung Ende der Treppenlichtzeit, sendet "1" bei Beginn Vorwarnzeit, danach "0" (ABB Doku hier uneindeutig)
#define OBJ_CALLPRESET  5  // 1 bit   - Rx - Trigger - Rufe Preset 1/2 auf, Bitwert "0" steht für Preset 1, "1" für Preset 2
#define OBJ_SETPRESET   6  // 1 bit   - Rx - Trigger - Bei "0" speichert aktuellen Schaltzustand als Preset 1, "1" als Preset 2
#define OBJ_SCENE       7  // 1 Byte  - Rx - Trigger - Szene aufrufen oder speichern: Bit 7: "0" aufrufen, "1" speichern. Bit 6: don't care. Bit 5..0: Szenenummer-1
#define OBJ_LOGIC1      8  // 1 bit   - Rx - Zustand/Trigger - Logikeingang 1
#define OBJ_LOGIC2      9  // 1 bit   - Rx - Zustand/Trigger - Logikeingang 2
#define OBJ_FORCEDOP   10  // 1 o. 2 bit  - Rx - Zustand - Zwangsposition aktivieren/deaktivieren
#define OBJ_THRESHOLD  11  // 1 o. 2 Byte - Rx - Zustand - Der überwachte Wert der Schwellwertfunktionalität
#define OBJ_CHGTHRESH1 12  // 1 o. 2 Byte - Rx - Zustand - Schwellwert 1 der Schwellwertfunktionalität ändern
#define OBJ_CONTACTMON 15  // 1 bit   -TxRd- Zustand - Kontaktüberwachung, "1" bei Strommesswert > 30mA bei geöffnetem Kontakt
#define OBJ_CURRENT    16  // 2 o. 4 Byte -TxRd- Zustand - Strommesswert in gewählter Datendarstellung, kann auch gelesen werden
#define OBJ_STATECTH1  17  // 1 bit   -TxRd- Zustand - Ergebnis des Vergleichs gemessener Strom - Stromschwelle 1, kann auch gelesen werden
#define OBJ_STATECTH2  18  // 1 bit   -TxRd- Zustand - Ergebnis des Vergleichs gemessener Strom - Stromschwelle 2, kann auch gelesen werden
#define OBJ_STATESW    19  // 1 bit   - Tx - Zustand - Rückmeldung Schaltzustand

/*
ab Zeile 4174
OBJEKTE
 0 In Operation,      1 Bit, ReadFlag, CommFlag, TransmitFlag
 1 Safety Priority 1, 1 Bit, WriteFlag, CommFlag, UpdateFlag
 2 Safety Priority 2, 1 Bit, WriteFlag, CommFlag, UpdateFlag
 3 Safety Priority 3, 1 Bit, WriteFlag, CommFlag, UpdateFlag
 4 -unused-
 5 -unused-
 6 -unused-
 7 -unused-
 8 -unused-
 9 -unused-
-----------
Heizaktor
10 Output A, Switch, 1 Bit, WriteFlag, CommFlag (Heizventil schalten)
oder 10 Output A, Control value (PWM), 1 Byte, WriteFlag, CommFlag (Stetigansteuerung �ber PWM)
11 Output A, Trigger valve purge, 1 Bit, WriteFlag, CommFlag (Ventilsp�lung)
12 Output A, Status valve purge, 1 Bit, CommFlag, TransmitFlag
13 Output A, RTR fault, 2 Bytes, CommFlag, TransmitFlag (wenn keine Werte vom Raumtempregler mehr kommen)
14 Output A, Forced operation, 1 Bit, WriteFlag, CommFlag
15 Output A, Status heating, 1 Bit, CommFlag, TransmitFlag (wenn Ventil schalten gew�hlt, dann hier R�ckmeldewert)
oder 15 Output A, Status heating, 1 Byte, CommFlag, TransmitFlag (wenn PWM gew�hlt, dann hier R�ckmeldewert)
16 Output A, Set preset 1/2, 1 Bit, WriteFlag, CommFlag
17 Output A, 8 bit scene recall/store, 1 Byte, WriteFlag, CommFlag
18 Output A, Logical connection 1, 1 Bit, WriteFlag, CommFlag
19 Output A, Logical connection 2, 1 Bit, WriteFlag, CommFlag
20 Output A, Forced Positioning, 2 Bit, WriteFlag, CommFlag
21 Output A, Threshold input, 2 Byte, WriteFlag, CommFlag
22 Output A, Change Threshold value 1, 2 Bytes, WriteFlag, CommFlag
23 -unused-
24 -unused-
25 Output A, Contact monitoring, 1 Bit, CommFlag, TransmitFlag (Sendet 1, wenn >30mA bei ge�ffnetem Kontakt gemessen, 2Sek Verz�gerung)
26 Output A, Current Value/mA, 2 Byte, ReadFlag, CommFlag, TransmitFlag
oder 26 Output A, Current Value/mA, 4 Byte, ReadFlag, CommFlag, TransmitFlag
27 Output A, Status Current-Threshold 1, 1 Bit, CommFlag, TransmitFlag
28 Output A, Status Current-Threshold 2, 1 Bit, CommFlag, TransmitFlag
29 Output A, Status Switch, 1 Bit, CommFlag, TransmitFlag (normales R�ckmeldeobjekt)

...

Schaltaktor
70 Output D, Switch, 1 bit, WriteFlag, CommFlag
71 Permanent ON, 1 Bit, WriteFlag, CommFlag
72 Disable time function, 1 Bit, WriteFlag, CommFlag
73 Duration of staircase lighting, 2 Bytes, ReadFlag, WriteFlag, CommFlag
74 Warning stair lighting, 1 Bit, CommFlag, TransmitFlag
75 Call preset 1/2, 1 Byte, WriteFlag, CommFlag
76 Set preset 1/2, 1 Bit, WriteFlag, CommFlag
77 8 bit scene recall/store, 1 Byte, WriteFlag, CommFlag
78 Logical connection 1, 1 Bit, WriteFlag, CommFlag
79 Logical connection 2, 1 Bit, WriteFlag, CommFlag
80 Forced Positioning, 2 Bit, WriteFlag, CommFlag
81 Threshold input, 2 Byte, WriteFlag, CommFlag (Schalten aufgrund eines externen Schwellwertes)
82 Change Threshold value 1, 2 Bytes, WriteFlag, CommFlag
83 -unused-
84 -unused-
85 Contact monitoring, 1 Bit, CommFlag, TransmitFlag (Sendet 1, wenn >30mA bei ge�ffnetem Kontakt gemessen, 2Sek Verz�gerung)
86 Current Value, 4 Byte, ReadFlag, CommFlag, TransmitFlag
87 Status Current-Threshold 1, 1 Bit, CommFlag, TransmitFlag
88 Status Current-Threshold 2, 1 Bit, CommFlag, TransmitFlag
89 Status Switch, 1 Bit, CommFlag, TransmitFlag (normales R�ckmeldeobjekt)

*/
#define APP_STARTADDR 0x4800
#define APP_CHOFFS 64
// _O: Offset der Zieladresse, _B: Bitoffset, _M Maske
#define APP_OPMODE_O           0 // Funktionsmodus
#define APP_OPMODE_B           0 //  1: Schaltaktor 2: Heizaktor
#define APP_OPMODE_M        0xff
#define APP_INIAFTERDNL_O      1 // Init nach download
#define APP_INIAFTERDNL_B      7 //  0: Nein, 1: Ja
#define APP_INIAFTERDNL_M   0x80
#define APP_SW_NONC_O          1 // Schalttyp NO/NC
#define APP_SW_NONC_B          4 //  0: Normally open 1: Normally closed
#define APP_SW_NONC_M       0x10
#define APP_SNDSTSAT_O         1 // Sende Status wenn
#define APP_SNDSTSAT_B         1 //  0: nie, 1: nach Änderung, 3: immer
#define APP_SNDSTSAT_M      0x0e //  Maske aus 3 Bit
#define APP_STSINV_O           1
#define APP_STSINV_B           0 // Invertiere Status
#define APP_STSINV_M        0x01 //  0: Nein, 1: Ja
//Aus XML
//#define APP_INIAFTERDNL_O      1 // Init nach download
//#define APP_INIAFTERDNL_B      0 //  0: Nein, 1: Ja
//#define APP_SW_NONC_O          1 // Schalttyp NO/NC
//#define APP_SW_NONC_B          3 //  0: Normally open 1: Normally closed
//#define APP_SW_NONC_M       0x08
//#define APP_SNDSTSAT_O         1 // Sende Status wenn
//#define APP_SNDSTSAT_B         4 //  0: nie, 1: nach Änderung, 3: immer
//#define APP_SNDSTSAT_M      0x70 //  Maske aus 3 Bit
//#define APP_STSINV_O           1
//#define APP_STSINV_B           7 // Invertiere Status
//#define APP_STSINV_M        0x80 //  0: Nein, 1: Ja
#define APP_REACTPFAIL_O       2 // Reaktion auf Busspannungsausfall
#define APP_REACTPFAIL_B       0 //  0: Kontakt öffnen, 1: Kontakt schließen, 2: Kontakt unverändert
#define APP_REACTPFAIL_M    0xff
#define APP_SW_ABVR_O          3 // Schaltobjekt nach Busspannungswiederkehr (entnommen aus realen Konfdaten)
#define APP_SW_ABVR_B          0 //  0: Schreibe 0, 1: Schreibe 1, 2: kein Schreibzugriff
#define APP_SW_ABVR_M       0x03
#define APP_ENACURRMEAS_O      4 // Stromerkennung aktivieren
#define APP_ENACURRMEAS_B      0 //  0: Nein, 1: Ja

#define APP_CURRCONTFAILMON_O 17 // Kontaktfehlerüberwachung durch Strommessung (Messwert > 30mA bei geöffnetem Kontakt)
#define APP_CURRCONTFAILMON_B  1 //  0: Nein, 1: Nur nach Änderung senden, 3: Immer
#define APP_CURRCONTFAILMON_M 0x0e // Betrifft nur das Auslösen des eigenständigen Sendens, nicht den Objektwert
#define APP_CURRCONTFAILINV_O 17 // Status der Kontaktfehlerüberwachung invertieren
#define APP_CURRCONTFAILINV_B  0 //  0: Nein, 1: Ja
#define APP_CURRCONTFAILINV_M 0x01

#define APP_TYPCURRMEAS_O      6 // Datentyp des Strommesswerts
#define APP_TYPCURRMEAS_B      0 //  3: 2 Byte Zähler (DPT7.012), 5: 4 Byte Float (DPT14.019)
#define APP_TYPCURRMEAS_M   0xff
#define APP_CURRTMRSNDVAL_O   14 // Stromwert nach Zykluszeit senden
#define APP_CURRTMRSNDVAL_B    0 //  Zeit in Sekunden, 0..65535, 16bit Wert, Wert 0 bedeutet nicht senden
#define APP_CURRTMRSNDVAL_M 0xffff
#define APP_CURRCHGSNDVAL_O   13 // Stromwert bei Änderung senden
#define APP_CURRCHGSNDVAL_B    0 //  0: nein, 1: 25mA, 2: 50mA, 4: 100mA, 8: 200mA, 20: 500mA, 40: 1A, 80: 2A, 200: 5A
#define APP_CURRCHGSNDVAL_M 0xff

#define APP_CURRTHEN_O        63 // Stromschaltschwellen aktivieren
#define APP_CURRTHEN_B         0 //  0: Nein, 1: Ja
#define APP_CURRTHEN_M      0x01
#define APP_CURREVATH_O       17 // Auswertung der Stromschaltschwellen
#define APP_CURREVATH_B        4 //  3: immer, 2: nur bei geschlossenem Kontakt, 1: nur bei offenem Kontakt
#define APP_CURREVATH_M     0x30
#define APP_CURREVADELAY_O    16 // Auswerteverzögerung nach Schließen des Kontaktes
#define APP_CURREVADELAY_B     0 //  Zeit in Sekunden, 0..255,
#define APP_CURREVADELAY_M  0xff
#define APP_CURRTHSCAL1_O     17 // Skalierung der Stromschaltschwelle 1
#define APP_CURRTHSCAL1_B      6 //  0: 100mA, 1: 10mA
#define APP_CURRTHSCAL1_M   0x40
#define APP_CURRTH1_O          7 // Stromwert Schaltschwelle 1
#define APP_CURRTH1_B          0 //  Wert 1..250, Skalierung 10mA oder 100mA, abh. von weiterem Parameter
#define APP_CURRTH1_M       0xff
#define APP_CURRHYST1_O        8 // Stromwert Hysterese Schaltschwelle 1
#define APP_CURRHYST1_B        0 //  3: 3mA, 1: 25mA, 2: 50mA, 4: 100mA, 8: 200mA, 20: 500mA, 40: 1A, 80: 2A, 200: 5A
#define APP_CURRHYST1_M     0xff
#define APP_CURRSNDCROSS1_O    9 // Bei Schaltschwellenquerung senden (Schaltschwelle 1)
#define APP_CURRSNDCROSS1_B    0 //  1: Sende 0 bei Übersch., 3: Sende 1 bei Übersch., 4: Sende 0 bei Unter, 12: Sende 1 bei Unter,
#define APP_CURRSNDCROSS1_M  0xff //  13: Sende 0 bei Über, 1 bei Unter, 7: Sende 1 bei Über, 0 bei Unter
#define APP_CURRTHEN2_O       63 // Stromschaltschwelle 2 aktivieren
#define APP_CURRTHEN2_B        1 //  0: Nein, 1: Ja
#define APP_CURRTHEN2_M     0x02
#define APP_CURRTH2_O         10 // Stromwert Schaltschwelle 2
#define APP_CURRTH2_B          0 //  Wert 1..250, Skalierung 100mA
#define APP_CURRTH2_M       0xff
#define APP_CURRHYST2_O       11 // Stromwert Hysterese Schaltschwelle 2
#define APP_CURRHYST2_B        0 //  3: 3mA, 1: 25mA, 2: 50mA, 4: 100mA, 8: 200mA, 20: 500mA, 40: 1A, 80: 2A, 200: 5A
#define APP_CURRHYST2_M     0xff
#define APP_CURRSNDCROSS2_O   12 // Bei Schaltschwellenquerung senden (Schaltschwelle 2)
#define APP_CURRSNDCROSS2_B    0 //  1: Sende 0 bei Übersch., 3: Sende 1 bei Übersch., 4: Sende 0 bei Unter, 12: Sende 1 bei Unter,
#define APP_CURRSNDCROSS2_M 0xff //  13: Sende 0 bei Über, 1 bei Unter, 7: Sende 1 bei Über, 0 bei Unter

#define APP_HEATCTRLVALTYPE_O 20 // Heizung Ansteuerung Datentyp
#define APP_HEATCTRLVALTYPE_B  0 //  0: 1bit Wert (PWM oder Ein/Aus), 1: 1Byte-Wert
#define APP_HEATCTRLVALTYPE_M 0x01
// Aus XML
//#define APP_ENAFUNTHRESH_O    20 // Freigeben der Schaltschwellen-Funktionalität
//#define APP_ENAFUNTHRESH_B     2 //  0: Nein, 1: Ja
//#define APP_ENAFUNTHRESH_M  0x04
//#define APP_ENAFUNCSAFETY_O   20 // Freigeben der Sicherheits-/Prioritätsfunktionen
//#define APP_ENAFUNCSAFETY_B    3 //  0: Nein, 1: Ja
//#define APP_ENAFUNCSAFETY_M 0x08
//#define APP_ENAFUNCLOGIC_O    20 // Freigeben der Logikfunktionen
//#define APP_ENAFUNCLOGIC_B     4 //  0: Nein, 1: Ja
//#define APP_ENAFUNCLOGIC_M  0x10
//#define APP_ENAFUNCSCENE_O    20 // Freigeben der Szenenfunktionen
//#define APP_ENAFUNCSCENE_B     5 //  0: Nein, 1: Ja
//#define APP_ENAFUNCSCENE_M  0x20
//#define APP_ENAFUNCPRESET_O   20 // Freigeben der Preset-Funktionen
//#define APP_ENAFUNCPRESET_B    6 //  0: Nein, 1: Ja
//#define APP_ENAFUNCPRESET_M 0x40
//#define APP_ENAFUNCTIME_O     20 // Freigeben der Zeitfunktionen
//#define APP_ENAFUNCTIME_B      7 //  0: Nein, 1: Ja
//#define APP_ENAFUNCTIME_M   0x80
#define APP_ENAFUNTHRESH_O    20 // Freigeben der Schaltschwellen-Funktionalität
#define APP_ENAFUNTHRESH_B     5 //  0: Nein, 1: Ja
#define APP_ENAFUNTHRESH_M  0x20
#define APP_ENAFUNCSAFETY_O   20 // Freigeben der Sicherheits-/Prioritätsfunktionen
#define APP_ENAFUNCSAFETY_B    4 //  0: Nein, 1: Ja
#define APP_ENAFUNCSAFETY_M 0x10 // Merkwürdigerweise steht Sicherheit und Logik im XML von den Bitwertigkeiten vertauscht, die Konf zum Gerät entspricht dann aber diesem hier.
#define APP_ENAFUNCLOGIC_O    20 // Freigeben der Logikfunktionen
#define APP_ENAFUNCLOGIC_B     3 //  0: Nein, 1: Ja
#define APP_ENAFUNCLOGIC_M  0x08
#define APP_ENAFUNCSCENE_O    20 // Freigeben der Szenenfunktionen
#define APP_ENAFUNCSCENE_B     2 //  0: Nein, 1: Ja
#define APP_ENAFUNCSCENE_M  0x04
#define APP_ENAFUNCPRESET_O   20 // Freigeben der Preset-Funktionen
#define APP_ENAFUNCPRESET_B    1 //  0: Nein, 1: Ja
#define APP_ENAFUNCPRESET_M 0x02
#define APP_ENAFUNCTIME_O     20 // Freigeben der Zeitfunktionen
#define APP_ENAFUNCTIME_B      0 //  0: Nein, 1: Ja
#define APP_ENAFUNCTIME_M   0x01
//#define APP_INVISIBLEPARAM_O  21 // Bezeichnet mit "Zeitfunktion nicht freigeben"
//#define APP_INVISIBLEPARAM_B   0 //  Jedoch unsichtbar und mit Defaultwert 1
//#define APP_INVISIBLEPARAM_M 0x01 // Ist das lediglich ein Defaultwert für den Heizungsbetrieb, d.h. wenn der andere Parameter "Zeitfunktion" nicht dargestellt wird???
#define APP_SW_TIMEFUNC_O     21 // Zeitfunktion
#define APP_SW_TIMEFUNC_B      0 //  1: Treppenlicht, 2: Ein/Aus Verzögerung, 3: Blinken
#define APP_SW_TIMEFUNC_M   0xff
#define APP_SW_VODTFABVR_O    22 // Wert des Objekts "Zeitfunktion deaktivieren" nach Bus voltage recovery
#define APP_SW_VODTFABVR_B     6 //  0: nicht-deaktivieren, 1: deaktivieren
#define APP_SW_VODTFABVR_M  0x40
#define APP_SW_TIMRESTAFPON_O 22 // Treppenlichtzeit nach Ende von Dauer-Ein neu starten
#define APP_SW_TIMRESTAFPON_B  5 //  0: Nein, 1: Ja
#define APP_SW_TIMRESTAFPON_M 0x20
#define APP_SW_TIMWARNING_O   22 // Warnung vor Ende der Treppenlichtzeit
#define APP_SW_TIMWARNING_B    3 //  0: Nein, 1: Über Objekt, 2: Über Aus/Ein-Blinken, 3: über beides
#define APP_SW_TIMWARNING_M 0x18
#define APP_SW_TIMSWOFFEN_O   22 // Treppenlicht ausschaltbar
#define APP_SW_TIMSWOFFEN_B    0 //  1: Ein mit "1"-Objekt, Aus mit "0", 3: Ein mit "0" oder "1", 5: Ein mit "1", "0" ohne Funktion
#define APP_SW_TIMSWOFFEN_M 0x07 //
#define APP_SW_TIMDURATION_O  23 // Dauer der Treppenlichtfunktion in Minuten (Sekundenanteil an anderer Stelle)
#define APP_SW_TIMDURATION_B   0 //  0...1000 Minuten
#define APP_SW_TIMDURATION_M 0xffff
#define APP_SW_TIMDURASEC_O   25 // Dauer der Treppenlichtfunktion, Sekundenanteil
#define APP_SW_TIMDURASEC_B    0 //  0..59
#define APP_SW_TIMDURASEC_M 0xff
#define APP_SW_TIMWARNTIME_O  29 // Treppenlicht Vorwarnzeit in Sekunden (addiert sich auf die Gesamtzeit)
#define APP_SW_TIMWARNTIME_B   0 //  0..65535
#define APP_SW_TIMWARNTIME_M 0xffff
#define APP_SW_TIMPUMPUP_O    31 // Treppenlicht: Aufpumpen der Treppenlichtzeit
#define APP_SW_TIMPUMPUP_B     0 //  0: Nein, 1: Ja, Retriggerfunktion, 2: Aufpumpen bis zu 2mal, 3: bis 3mal, 4: bis 4mal, 5: bis 5mal
#define APP_SW_TIMPUMPUP_M  0xff
#define APP_SW_FLASHAFTER_O   22 // Blinken: Zustand nach dem Blinken
#define APP_SW_FLASHAFTER_B    2 //  0: aktueller Zustand, 1: Aus, 3: Ein
#define APP_SW_FLASHAFTER_M 0x0C
#define APP_SW_FLASHOBJFL_O   22 // Blinken: Funktion des Objekts "Blinken"
#define APP_SW_FLASHOBJFL_B    0 //  1: Blinken bei Objektwert "1", 2: bei Objektwert "0", 3: keine Abhängigkeit, immer Blinken
#define APP_SW_FLASHOBJFL_M 0x03
#define APP_SW_FLASHONMIN_O   23 // Blinkzeit Ein-Periode Minutenanteil
#define APP_SW_FLASHONMIN_B    0 //  0..65535
#define APP_SW_FLASHOFFMIN_M 0xffff
#define APP_SW_FLASHONSEC_O   25 // Blinkzeit Ein-Periode Sekundenanteil
#define APP_SW_FLASHONSEC_B    0 //  0..59
#define APP_SW_FLASHONSEC_M 0xff
#define APP_SW_FLASHOFFMIN_O  26 // Blinkzeit Aus-Periode Minutenanteil
#define APP_SW_FLASHOFFMIN_B   0 //  0..65535
#define APP_SW_FLASHOFFMIN_M 0xffff
#define APP_SW_FLASHOFFSEC_O  28 // Blinkzeit Aus-Periode Sekundenanteil
#define APP_SW_FLASHOFFSEC_B  0  //  0..59
#define APP_SW_FLASHOFFSEC_M 0xff
#define APP_SW_FLASHNUMBER_O  31 // Blinken: Anzahl der Ein-Impulse
#define APP_SW_FLASHNUMBER_B   0 //  1..100
#define APP_SW_FLASHNUMBER_M 0xff
#define APP_SW_DELONTIME_O    23 // Verzögerungsfunktion bis zum Einschalten in Minuten (Sekundenanteil an anderer Stelle)
#define APP_SW_DELONTIME_B     0 //  0...65535 Minuten
#define APP_SW_DELONTIME_M 0xffff
#define APP_SW_DELONTSEC_O    25 // Verzögerungsfunktion bis zum Einschalten, Sekundenanteil
#define APP_SW_DELONTSEC_B     0 //  0..59
#define APP_SW_DELONTSEC_M  0xff
#define APP_SW_DELOFFTIME_O   26 // Verzögerungsfunktion bis zum Ausschalten in Minuten (Sekundenanteil an anderer Stelle)
#define APP_SW_DELOFFTIME_B    0 //  0...65535 Minuten
#define APP_SW_DELOFFTIME_M 0xffff
#define APP_SW_DELOFFTSEC_O   28 // Verzögerungsfunktion bis zum Ausschalten, Sekundenanteil
#define APP_SW_DELOFFTSEC_B    0 //  0..59
#define APP_SW_DELOFFTSEC_M 0xff

#define APP_SW_PRESET1VAL0_O  32 // Reaktion auf Preset 1 (Objektwert 0)
#define APP_SW_PRESET1VAL0_B   0 //  0: Ausschalten, 1: Einschalten, 4: Alter Zustand vor Preset 2
#define APP_SW_PRESET1VAL0_M 0xff // 6: Ursprünglich paramaterisierten Zustand von Preset 2 wiederherstellen, 255: Keine Reaktion
#define APP_SW_PRESET2VAL1_O  33 // Reaktion auf Preset 2 (Objektwert 1)
#define APP_SW_PRESET2VAL1_B   0 //  0: Ausschalten, 1: Einschalten, 255: Keine Reaktion
#define APP_SW_PRESET2VAL1_M 0xff

#define APP_SW_SCENEUNUSED_M 0x40
#define APP_SW_SCEPRESET1_O   34 // Szenen Zuordnung 1 Preset
#define APP_SW_SCEPRESET1_B    7 //  0: Aus, 1: Ein
#define APP_SW_SCEPRESET1_M 0x80
#define APP_SW_SCENUMBER1_O   34 // Szenen Zuordnung 1 zu Szene #
#define APP_SW_SCENUMBER1_B    0 //  0..63: Szene 1..64
#define APP_SW_SCENUMBER1_M 0x3f
#define APP_SW_SCEPRESET2_O   35 // Szenen Zuordnung 2 Preset
#define APP_SW_SCEPRESET2_B    7 //  0: Aus, 1: Ein
#define APP_SW_SCEPRESET2_M 0x80
#define APP_SW_SCENUMBER2_O   35 // Szenen Zuordnung 2 zu Szene #
#define APP_SW_SCENUMBER2_B    1 //  0..63: Szene 1..64
#define APP_SW_SCENUMBER2_M 0x3f
#define APP_SW_SCEPRESET3_O   36 // Szenen Zuordnung 3 Preset
#define APP_SW_SCEPRESET3_B    7 //  0: Aus, 1: Ein
#define APP_SW_SCEPRESET3_M 0x80
#define APP_SW_SCENUMBER3_O   36 // Szenen Zuordnung 3 zu Szene #
#define APP_SW_SCENUMBER3_B    1 //  0..63: Szene 1..64
#define APP_SW_SCENUMBER3_M 0x3f
#define APP_SW_SCEPRESET4_O   37 // Szenen Zuordnung 4 Preset
#define APP_SW_SCEPRESET4_B    7 //  0: Aus, 1: Ein
#define APP_SW_SCEPRESET4_M 0x80
#define APP_SW_SCENUMBER4_O   37 // Szenen Zuordnung 4 zu Szene #
#define APP_SW_SCENUMBER4_B    1 //  0..63: Szene 1..64
#define APP_SW_SCENUMBER4_M 0x3f
#define APP_SW_SCEPRESET5_O   38 // Szenen Zuordnung 5 Preset
#define APP_SW_SCEPRESET5_B    7 //  0: Aus, 1: Ein
#define APP_SW_SCEPRESET5_M 0x80
#define APP_SW_SCENUMBER5_O   38 // Szenen Zuordnung 5 zu Szene #
#define APP_SW_SCENUMBER5_B    1 //  0..63: Szene 1..64
#define APP_SW_SCENUMBER5_M 0x3f

// Aus XML
// #define APP_SW_SCEPRESET1_O   34 // Szenen Zuordnung 1 Preset
// #define APP_SW_SCEPRESET1_B    0 //  0: Aus, 1: Ein
// #define APP_SW_SCEPRESET1_M 0x01
// #define APP_SW_SCENUMBER1_O   34 // Szenen Zuordnung 1 zu Szene #
// #define APP_SW_SCENUMBER1_B    1 //  0..63: Szene 1..64
// #define APP_SW_SCENUMBER1_M 0x7e
// etc...

#define APP_SW_THISHYST_O     48 // Schwellwerte: Schwellwerte bilden Hysteresepunkte
#define APP_SW_THISHYST_B      0 //  1: Ja, 2: Nein
#define APP_SW_THISHYST_M   0xff
#define APP_SW_THTYPE_O       49 // Schwellwerte: Datentyp
#define APP_SW_THTYPE_B        0 //  2: 1 Byte Wert, 3: 2 Byte Wert
#define APP_SW_THTYPE_M     0xff
#define APP_SW_THCONST1_O     50 // Schwellwert 1
#define APP_SW_THCONST1_B      0 // 8 oder 16 bit Wert
#define APP_SW_THCONST1_M   0xffff //0xff oder 0xffff
#define APP_SW_THCONST2_O     52 // Schwellwert 2
#define APP_SW_THCONST2_B      0 // 8 oder 16 bit Wert
#define APP_SW_THCONST2_M   0xffff //0xff oder 0xffff
#define APP_SW_THVABVR_O      54 // Wert des Threshold Objekts bei Busspannungswiederkehr
#define APP_SW_THVABVR_B       0 // 8 oder 16 bit Wert
#define APP_SW_THVABVR_M  0xffff //0xff oder 0xffff
#define APP_SW_THREAKTLOW_O   56 // Schwellwert, Funktion unterer Bereich/untere Schwelle
#define APP_SW_THREAKTLOW_B    0 //  0: Ausschalten, 1: Einschalten, 2: keine Reaktion
#define APP_SW_THREAKTLOW_M 0xff
#define APP_SW_THREAKTMID_O   57 // Schwellwert, Funktion der mittlere Bereich
#define APP_SW_THREAKTMID_B    0 //  0: Ausschalten, 1: Einschalten, 2: keine Reaktion
#define APP_SW_THREAKTMID_M 0xff
#define APP_SW_THREAKTHIGH_O  58 // Schwellwert, Funktion oberer Bereich/obere Schwelle
#define APP_SW_THREAKTHIGH_B   0 //  0: Ausschalten, 1: Einschalten, 2: keine Reaktion
#define APP_SW_THREAKTHIGH_M 0xff

#define APP_SW_LOG1ENA_O      39 // Logik 1 (de)aktivieren
#define APP_SW_LOG1ENA_B       0 //  0: Disable, 1: Enable
#define APP_SW_LOG1ENA_M    0x01
#define APP_SW_LOG1FCT_O      40 // Logik 1, Funktion
#define APP_SW_LOG1FCT_B       0 //  1: Und, 2: Oder, 3: Xor, 4: Gate
#define APP_SW_LOG1FCT_M    0xff
#define APP_SW_LOG1GATEDISVAL_O 39 // Logikfunktion 1, Gate, Gate deaktivieren bei Objektwert "Logik1"
#define APP_SW_LOG1GATEDISVAL_B  3 // 0: 0, 1: 1
#define APP_SW_LOG1GATEDISVAL_M 0x08
#define APP_SW_LOG1XORINV_O   39 // Logikfunktion 1, Ergebnis invertieren
#define APP_SW_LOG1XORINV_B    2 //  0: Nein, 1: Ja
#define APP_SW_LOG1XORINV_M 0x04
#define APP_SW_LOG1OVLCABV_O  39 // Objektwert "Logik1" nach Busspannungswiederkehr
#define APP_SW_LOG1OVLCABV_B   1 //  0: 0, 1: 1
#define APP_SW_LOG1OVLCABV_M 0x02
// Aus XML
//#define APP_SW_LOG1ENA_O      39 // Logik 1 (de)aktivieren
//#define APP_SW_LOG1ENA_B       7 //  0: Disable, 1: Enable
//#define APP_SW_LOG1ENA_M    0x80
//#define APP_SW_LOG1XORINV_O   39 // Logikfunktion 1, Ergebnis invertieren
//#define APP_SW_LOG1XORINV_B    5 //  0: Nein, 1: Ja
//#define APP_SW_LOG1XORINV_M 0x20

#define APP_SW_LOG2ENA_O      41 // Logik 2 (de)aktivieren
#define APP_SW_LOG2ENA_B       0 //  0: Disable, 1: Enable
#define APP_SW_LOG2ENA_M    0x01
#define APP_SW_LOG2FCT_O      42 // Logik 2, Funktion
#define APP_SW_LOG2FCT_B       0 //  1: Und, 2: Oder, 3: Xor, 4: Gate
#define APP_SW_LOG2FCT_M    0xff
#define APP_SW_LOG2GATEDISVAL_O 41 // Logikfunktion 2, Gate, Gate deaktivieren bei Objektwert "Logik2"
#define APP_SW_LOG2GATEDISVAL_B  3 // 0: 0, 1: 1
#define APP_SW_LOG2GATEDISVAL_M 0x08
#define APP_SW_LOG2XORINV_O   41 // Logikfunktion 2, Ergebnis invertieren
#define APP_SW_LOG2XORINV_B    2 //  0: Nein, 1: Ja
#define APP_SW_LOG2XORINV_M 0x04
#define APP_SW_LOG2OVLCABV_O  41 // Objektwert "Logik2" nach Busspannungswiederkehr
#define APP_SW_LOG2OVLCABV_B   1 //  0: 0, 1: 1
#define APP_SW_LOG2OVLCABV_M 0x02

#define APP_SW_FORCEDEND_O    43 // Aktion nach Ende der Zwangsstellung
#define APP_SW_FORCEDEND_B     0 //  0: Ausschalten, 1: Einschalten, 2: unverändert, 3: bestimme momentane Kontaktpos.
#define APP_SW_FORCEDEND_M  0xff
#define APP_SW_FORCEDPR3_O    44 // Kontaktstellung bei Sicherheit/Priorität 3
#define APP_SW_FORCEDPR3_B     0 //  0: Ausschalten, 1: Einschalten, 2: unverändert, 255: inaktiv
#define APP_SW_FORCEDPR3_M  0xff
#define APP_SW_FORCEDPR2_O    45 // Kontaktstellung bei Sicherheit/Priorität 2
#define APP_SW_FORCEDPR2_B     0 //  0: Ausschalten, 1: Einschalten, 2: unverändert, 255: inaktiv
#define APP_SW_FORCEDPR2_M  0xff
#define APP_SW_FORCEDPOS_O    46 // Kontaktstellung bei Zwangsfunktion
#define APP_SW_FORCEDPOS_B     0 //  0: Aus mit 1Bit Objekt, 1: Ein mit 1Bit Objekt, 2: unverändert mit 1Bit Objekt,
#define APP_SW_FORCEDPOS_M  0xff //  7: Schaltzustand mit 2bit Objekt, 255: inaktiv
#define APP_SW_FORCEDPR1_O    47 // Kontaktstellung bei Sicherheit/Priorität 1
#define APP_SW_FORCEDPR1_B     0 //  0: Ausschalten, 1: Einschalten, 2: unverändert, 255: inaktiv
#define APP_SW_FORCEDPR1_M  0xff
#define APP_SW_FORCEDOVBVR_O  59 // Zwangsfunktion: Objektwert bei Busspannungswiederkehr
#define APP_SW_FORCEDOVBVR_B   0 //  Bei 1Bit-Wert: 0: disable, 1: enable
#define APP_SW_FORCEDOVBVR_M 0x  //  Bei 2Bit-Wert: 0: Inaktiv, 2: Aus, 3: Ein

#define APP_SENDSWDELAYPO_O  768 // Sende-/Schaltverzögerung nach Busspannungswiederkehr in Sekunden
#define APP_SENDSWDELAYPO_B    0 //  2..255
#define APP_SENDSWDELAYPO_M 0xff
#define APP_SENDALIVE_O      769 // Sende zyklisch "In Operation" Objekte
#define APP_SENDALIVE_B        0 //  0: Inaktiv, 1..65535 Sendeperiode in Sekunden
#define APP_SENDALIVE_M   0xffff
//#define APP_SAFENABLEOBJ_O   799 // Freischalten der Sicherheitsobjekte im Schaltaktorbetrieb
//#define APP_SAFENABLEOBJ_B     7 //  0: Nein, 1: Ja
//#define APP_SAFENABLEOBJ_M  0x80
#define APP_SAFPRIO3TIM_O    774 // Sicherheit 3: Überwachungszeit in Sekunden
#define APP_SAFPRIO3TIM_B      0 //  0: inaktiv, 1..65535 Zeit in Sekunden
#define APP_SAFPRIO3TIM_M 0xffff
#define APP_SAFPRIO3FKT_O    776 // Sicherheit 3: Funktion
#define APP_SAFPRIO3FKT_B      0 //  0: Auslösen Sicherheit durch Objektwert "0", 1: Auslösen Sicherheit durch Objektwert "1",
#define APP_SAFPRIO3FKT_M   0xff //  255: Inaktiv     Bei anderem Objektwert wird die Überwachungszeit neu gestartet!
#define APP_SAFPRIO2TIM_O    777 // Sicherheit 2: Überwachungszeit in Sekunden
#define APP_SAFPRIO2TIM_B      0 //  0: inaktiv, 1..65535 Zeit in Sekunden
#define APP_SAFPRIO2TIM_M 0xffff
#define APP_SAFPRIO2FKT_O    779 // Sicherheit 2: Funktion
#define APP_SAFPRIO2FKT_B      0 //  0: Auslösen Sicherheit durch Objektwert "0", 1: Auslösen Sicherheit durch Objektwert "1",
#define APP_SAFPRIO2FKT_M   0xff //  255: Inaktiv
#define APP_SAFPRIO1TIM_O    783 // Sicherheit 1: Überwachungszeit in Sekunden
#define APP_SAFPRIO1TIM_B      0 //  0: inaktiv, 1..65535 Zeit in Sekunden
#define APP_SAFPRIO1TIM_M 0xffff
#define APP_SAFPRIO1FKT_O    785 // Sicherheit 1: Funktion
#define APP_SAFPRIO1FKT_B      0 //  0: Auslösen Sicherheit durch Objektwert "0", 1: Auslösen Sicherheit durch Objektwert "1",
#define APP_SAFPRIO1FKT_M   0xff //  255: Inaktiv
#define APP_TELRATELIMIT_O   786 // Telegrammratenbegrenzung, max Telegramme je Sekunde
#define APP_TELRATELIMIT_B     0 //  1: 1, 2: 2, 3: 3, 5: 5, 10: 10, 20: 20, 255: ohne Begrenzung
#define APP_TELRATELIMIT_M  0xff
