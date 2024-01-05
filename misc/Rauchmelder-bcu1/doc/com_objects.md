# List of Com objects

| #  | Name                     | Function   | Description                                                          | Length  | Flags<br>(crtwui) | DPT    |
|----|--------------------------|------------|----------------------------------------------------------------------|---------|-------------------|--------|
| 0  | Alarm                    | Vernetzung | Bus Alarm ein/aus                                                    | 1 bit   | cwt               | 1.005  |
| 1  | Testalarm                | Vernetzung | Bus Testalarm ein/aus                                                | 1 bit   | cwt               | 1.001  |
| 2  | Alarm                    | Rücksetzen | Bus (Test)alarm zurücksetzen/quittieren                              | 1 bit   | cwt               | 1.001  |
| 3  | Alarm                    | Status     | Wird bei Zustandsänderung gesendet<br>(ausgenommen Bus-Alarm)        | 1 bit   | crt               | 1.005  |
| 4  | Verzögerter Alarm        | Status     | Status verzögerter Alarm                                             | 1 bit   | crt               | 1.011  |
| 5  | Testalarm                | Status     | Wird bei Zustandsänderung gesendet<br>(ausgenommen Bus-Alarm)        | 1 bit   | crt               | 1.011  |
| 6  | Seriennummer             | Status     | Seriennummer des Rauchmelders                                        | 4 bytes | crt               | 12.xxx |
| 7  | Betriebszeit Sekunden    | Status     | Betriebsdauer des Rauchmelders in Sekunden                           | 4 bytes | crt               | 12.100 |
| 7  | Betriebsstunden          | Status     | Betriebsdauer des Rauchmelders in Stunden                            | 2 bytes | crt               | 7.007  |
| 8  | Rauchkammerwert          | Status     | Wert des Rauchkammersensors                                          | 2 bytes | crt               | 9.020  |
| 9  | Verschmutzungsgrad       | Status     | Verschmutzungsgrad des Rauchkammersensors                            | 1 byte  | crt               | 5.010  |
| 10 | Batterie Spannung        | Status     | Spannung der Batterie                                                | 2 bytes | crt               | 9.020  |
| 11 | Temperatur               | Status     | Temperatur (Mittelwert beider Sensoren)                              | 2 bytes | crt               | 9.001  |
| 12 | Fehlercode               | Status     | Interner Modul Fehlercode                                            | 1 byte  | crt               | 5.010  |
| 13 | Batterie leer            | Status     | Batterie soll ersetzt werden                                         | 1 bit   | crt               | 1.011  |
| 14 | Rauchmelder Fehlfunktion | Status     | Funktionsstörung am Rauchmelder                                      | 1 bit   | crt               | 1.011  |
| 15 | Anzahl Rauchalarme       | Status     | Anzahl der lokalen Rauch-Alarme                                      | 1 byte  | crt               | 5.010  |
| 16 | Anzahl Temperaturalarme  | Status     | Anzahl der lokalen Temperatur-Alarme                                 | 1 byte  | crt               | 5.010  |
| 17 | Anzahl Testalarme        | Status     | Anzahl der lokalen Testalarme                                        | 1 byte  | crt               | 5.010  |
| 18 | Anzahl Draht Alarme      | Status     | Anzahl der Alarme über Kabel<br>(grüne Klemme / Rauchmelder-Bus)     | 1 byte  | crt               | 5.010  |
| 19 | Anzahl Bus Alarme        | Status     | Anzahl der Alarme über KNX-Bus                                       | 1 byte  | crt               | 5.010  |
| 20 | Anzahl Draht Testalarme  | Status     | Anzahl der Testalarme über Kabel<br>(grüne Klemme / Rauchmelder-Bus) | 1 byte  | crt               | 5.010  |
| 21 | Anzahl Bus Testalarme    | Status     | Anzahl der Testalarme über KNX-Bus                                   | 1 byte  | crt               | 5.010  |