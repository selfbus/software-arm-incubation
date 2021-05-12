# raincenter-bim112

### Implementation of the Selfbus ESPA Tacomat Comfort Raincenter.

It brings the already in the ESPA Tacomat Comfort existing serial RS232 to the KNX-Bus.

for addition infos see also:  
- inc/config.h          additional configuration  
- inc/rc_protocol.h     contains serial protocol information  



### ETS objects used for testing:

#####  JAL-410.01 Kanal A Rolladen
|#Object | Name                      |Function                                            |DPT-Type |
|:------:|:--------------------------|:---------------------------------------------------|:-------:|
|13      |Rolladen Auf/Ab            | TapWaterRefill (on/off)                            |1.001    |
|15      |Stop                       | TapWaterExchangeActive (on/off)                    |1.001    |
|17      |Status aktuelle Richtung   | TapWaterRefill Status (on/off)                     |1.001    |
|        |                           |                                                    |         |
|20      |Status aktuelle Position   | Calibrated Fill Level in m3 * 10 (43 means 4.3m³)  |5.005    |
|22      |Akt. Position gueltig      | Alarm (on/off)                                     |1.001    |
|23      |Referenzfahrt starten      | Pump active (on/off)                               |1.001    |
|25      |Status obere Position      | AutomaticTapwaterRefill Status (on/off)            |1.001    |
|26      |Status untere Position     | ManualTapwaterRefill Status (on/off)               |1.001    |