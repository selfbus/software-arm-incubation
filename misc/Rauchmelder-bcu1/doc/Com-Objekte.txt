Aufstellung der Com-Objekte
---------------------------

TODO:  Veraltert, bitte updaten


OBJ_SET_ALARM
  1 Bit
  Schreiben: Bus Alarm ein/aus

OBJ_SET_TALARM
  1 Bit
  Schreiben: Bus Testalarm ein/aus

OBJ_SERIAL
  4 Byte Long
  Lesen: Seriennummer des Rauchmelders

OBJ_OPERATING_TIME
  4 Byte Long
  Lesen: Betriebsdauer des Rauchmelders

OBJ_SMOKEBOX_VALUE
  2 Byte Integer
  Lesen: Wert des Rauchkammer Sensors (?)

OBJ_POLLUTION
  1 Byte
  Lesen: Verschmutzungsgrad des Rauchkammer Sensors (?)

OBJ_BAT_VOLTAGE
  2 Byte Float (DPT 9.001)
  Lesen: Spannung der Batterie

OBJ_TEMP
  2 Byte Float (DPT 9.001)
  Lesen: Temperatur an den Temperatur Sensoren (höchster Wert)

OBJ_CNT_SMOKEALARM
  1 Byte
  Lesen: Anzahl der lokalen Rauch-Alarme

OBJ_CNT_TEMPALARM
  1 Byte
  Lesen: Anzahl der lokalen Temperatur-Alarme

OBJ_CNT_TESTALARM
  1 Byte
  Lesen: Anzahl der lokalen Testalarme

OBJ_CNT_ALARM_WIRE
  1 Byte
  Lesen: Anzahl der Alarme über Kabel (grüne Klemme / Rauchmelder-Bus)

OBJ_CNT_ALARM_WIRELESS
  1 Byte
  Lesen: Anzahl der Alarme über KNX Bus

OBJ_CNT_TALARM_WIRE
  1 Byte
  Lesen: Anzahl der Testalarme über Kabel (grüne Klemme / Rauchmelder-Bus)

OBJ_CNT_TALARM_WIRELESS
  1 Byte
  Lesen: Anzahl der Testalarme über KNX Bus

OBJ_STAT_ALARM
  1 Bit
  Lesen: Alarm Status
  Wird bei Zustandsänderung (ausgenommen Bus-Alarm) gesendet
  Zyklisch senden bei Alarm möglich

OBJ_STAT_ALARM_CENTRAL
  1 Bit
  Lesen: Zentral-Alarm Status
  Schreiben: Zentral-Alarm Status setzen (?)

OBJ_STAT_ALARM_DELAYED
  1 Bit
  Lesen: Status verzögerter Alarm

OBJ_STAT_TALARM
  1 Bit
  Lesen: Testalarm Status
  Wird bei Zustandsänderung (ausgenommen Bus-Alarm) gesendet
  Zyklisch senden bei Alarm möglich

OBJ_STAT_TALARM_CENTRAL
  1 Bit
  Lesen: Zentral-Testalarm Status
  Schreiben: Zentral-Testalarm Status setzen (?)

OBJ_STAT_BAT_LOW
  1 Bit
  Lesen: Batterie soll ersetzt werden
  Wird bei Zustandsänderung gesendet
  Zyklisch senden (wenn gesetzt) möglich

OBJ_STAT_MALFUNCTION
  1 Bit oder 1 Byte mit Typ
  Lesen: Funktionsstörung am Rauchmelder
  Wird bei Zustandsänderung gesendet
  Zyklisch senden (wenn gesetzt) möglich
