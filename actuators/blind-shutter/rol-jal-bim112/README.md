## rol-jal-bim112 a 4/8-fold Shutter Actuator, 4/8-fach Jalousieaktor

*english:*  
This is an application that uses the [sbLib](https://selfbus.org) with BIM112 (Mask 0x0701) emulation.
The emulated device is a MDT
- JAL-0410.01 Shutter Actuator 4-fold, 4MU, 230VAC, 10A or 
- JAL-0810.01 Shutter Actuator 8-fold, 8MU, 230VAC, 10A

Please use the [MDT_KP_Jalousieaktor_V28.knxprod](https://www.mdt.de/fileadmin/user_upload/user_upload/download/Archiv/MDT_KP_Jalousieaktor_V28.knxprod) product database for ETS configuration.

*deutsch:*  
Das ist eine Applikation welche die [sbLib](https://selfbus.org) mit BIM112 (Mask 0x0701) Emulation verwendet.
Das simulierte Gerät ist ein MDT
- JAL-0410.01 Jalousieaktor JAL 4-fach, 4TE, 230VAC, 10A bzw. 
- JAL-0810.01 Jalousieaktor JAL 8-fach, 8TE, 230VAC, 10A

Zur ETS-Konfiguration die [MDT_KP_Jalousieaktor_V28.knxprod](https://www.mdt.de/fileadmin/user_upload/user_upload/download/Archiv/MDT_KP_Jalousieaktor_V28.knxprod) Produktdatenbank verwenden.

### Build configuration naming (Build-Konfiguration Namen) 
| Abbreviation<br>Abkürzung | Meaning<br>Bedeutung                                                             |
|---------------------------|----------------------------------------------------------------------------------|
| Debug                     | Only for debugging<br>Nur zum debuggen                                           |
| Flashstart                | For use with the Selfbus bootloader<br>Zur Nutzung mit dem Selfbus Bootloader    |
| Release                   | Stand alone firmware<br>Lauffähige Firmware ohne Nutzung des Selfbus Bootloaders |
| 4ch, 8ch                  | Number of channels<br> Anzahl der Kanäle                                         |
| bf                        | bus-fail / bus voltage detection<br>Busspannungsausfallerkennnung                |
| ha                        | hand actuation<br>mit Handbedienung                                              |

## 4MU-ARM Controller ([PCB](https://github.com/selfbus/hardware-merged/tree/main/controller_lpc1115/lpc1115_4MU_MID), [Wiki](https://selfbus.org/wiki/hardware/controller/49-4te-controller-arm-lpc1115)) + shutter_4x_4MU ([PCB](https://github.com/selfbus/hardware-merged/tree/main/applications_din/shutter_4x_4MU), [Wiki](https://selfbus.org/wiki/devices/outputs/25-8-fold-binary-output-4module-units-lpc1115))
### used IO Ports (verwendete IO Ports):
| Relay # | Channel # | Function | Selfbus IO | Selfbus 26-pol | ARM Port | Description (Beschreibung) |
|:-------:|-----------|:--------:|:----------:|:--------------:|----------|----------------------------|
|  REL1   | 1         |  up/auf  |    IO1     |       7        | PIO2_2   |                            |
|  REL2   | 1         | down/zu  |    IO2     |       8        | PIO0_7   |                            |
|  REL3   | 2         |  up/auf  |    IO3     |       9        | PIO2_10  |                            |
|  REL4   | 2         | down/zu  |    IO4     |       10       | PIO2_9   |                            |
|  REL5   | 3         |  up/auf  |    IO5     |       13       | PIO0_2   |                            |
|  REL6   | 3         | down/zu  |    IO6     |       14       | PIO0_8   |                            |
|  REL7   | 4         |  up/auf  |    IO7     |       15       | PIO0_9   |                            |
|  REL8   | 4         | down/zu  |    IO8     |       16       | PIO2_11  |                            |
|         |           |          |            |                |          |                            |
|         |           |   PWM    |  PIN_PWM   |       3        | PIO3_2   | pulse wide modulation      |
|         |           |   VDD    |            |       6        |          | EIB_DC                     |

-------------

## 4MU-ARM Controller ([PCB](https://github.com/selfbus/hardware-merged/tree/main/controller_lpc1115/lpc1115_4MU_MID), [Wiki](https://selfbus.org/wiki/hardware/controller/49-4te-controller-arm-lpc1115)) + shutter_8x_xMU (PCB has yet to be created, [Wiki](https://selfbus.org/wiki/devices/outputs/25-8-fold-binary-output-4module-units-lpc1115))
### proposed IO ports (vorgeschlagene IO Ports):
| Relay # | Channel # | Function | Selfbus IO | Selfbus 26-pol | ARM Port | Description (Beschreibung) |
|:-------:|:---------:|:--------:|:----------:|:--------------:|----------|----------------------------|
|  REL1   |     1     |  up/auf  |    IO1     |       7        | PIO2_2   |                            |
|  REL2   |     1     | down/zu  |    IO2     |       8        | PIO0_7   |                            |
|         |           |          |            |                |          |                            |
|  REL3   |     2     |  up/auf  |    IO3     |       9        | PIO2_10  |                            |
|  REL4   |     2     | down/zu  |    IO4     |       10       | PIO2_9   |                            |
|         |           |          |            |                |          |                            |
|  REL5   |     3     |  up/auf  |    IO5     |       13       | PIO0_2   |                            |
|  REL6   |     3     | down/zu  |    IO6     |       14       | PIO0_8   |                            |
|         |           |          |            |                |          |                            |
|  REL7   |     4     |  up/auf  |    IO7     |       15       | PIO0_9   |                            |
|  REL8   |     4     | down/zu  |    IO8     |       16       | PIO2_11  |                            |
|         |           |          |            |                |          |                            |
|  REL9   |     5     |  up/auf  |  UART-TxD  |       17       | PIO3_0   |                            |
|  REL10  |     5     | down/zu  |  UART-RxD  |       18       | PIO3_1   |                            |
|         |           |          |            |                |          |                            |
|  REL11  |     6     |  up/auf  |    IO9     |       19       | PIO1_0   |                            |
|  REL12  |     6     | down/zu  |    IO10    |       20       | PIO1_1   |                            |
|         |           |          |            |                |          |                            |
|  REL13  |     7     |  up/auf  |    IO12    |       23       | PIO1_2   |                            |
|  REL14  |     7     | down/zu  |    IO13    |       24       | PIO1_4   |                            |
|         |           |          |            |                |          |                            |
|  REL15  |     8     |  up/auf  |    IO14    |       25       | PIO1_7   |                            |
|  REL16  |     8     | down/zu  |    IO15    |       26       | PIO1_6   |                            |
|         |           |          |            |                |          |                            |
|         |           |   PWM    |  PIN_PWM   |       3        | PIO3_2   | pulse wide modulation      |
|         |           |   VDD    |            |       6        |          | EIB_DC                     |

## optional hand actuation LedTaster_4TE ([PCB](https://github.com/selfbus/hardware-merged/tree/main/addons/leds_buttons_lpc922ctrl_4MU), [Wiki (legacy)](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/LedTasterBoard_4TE))
### used IO Ports (verwendete IO Ports):
|    Function    | Selfbus IO | Selfbus LT | ARM Port | Description (Beschreibung) |
|:--------------:|:----------:|:----------:|----------|----------------------------|
| LED 1 / Btn 1  |  PIN_LT1   |     1      | PIO2_1   |                            |
| LED 2 / Btn 2  |  PIN_LT2   |     2      | PIO0_3   |                            |
| LED 3 / Btn 3  |  PIN_LT3   |     3      | PIO2_4   |                            |
| LED 4 / Btn 4  |  PIN_LT4   |     4      | PIO2_5   |                            |
| LED 5 / Btn 5  |  PIN_LT5   |     5      | PIO3_5   |                            |
| LED 6 / Btn 6  |  PIN_LT6   |     6      | PIO3_4   |                            |
| LED 7 / Btn 7  |  PIN_LT7   |     7      | PIO1_10  |                            |
| LED 8 / Btn 8  |  PIN_LT8   |     8      | PIO0_11  |                            |
|    Readback    |  PIN_LT9   |     9      | PIO2_3   |                            |

## See [config.h](inc/config.h) for additional information.

# ToDo for Blind/Shutter

- implement correct handling of upper/lower limit
- implement bus return/bus down handling
- check the meaning of LOCK ABS Positioning
- create 8-fold PCB
