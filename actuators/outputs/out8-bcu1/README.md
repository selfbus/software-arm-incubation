# out8-bcu1

english:
This is an application that uses the [sbLib](https://selfbus.org) with BCU1 emulation.
The emulated device is a Jung 2138 KNX switch actuator with 8 digital outputs.

Please use the [Jung 2138 VD database](https://downloads.jung.de/public/de/software/produktdatenbanken/ets2_ets3/de_2138_10.vd1) for ETS configuration from this site:
https://downloads.jung.de/public/de/software/produktdatenbanken/ets2_ets3/de_2138_10.vd1

deutsch:
Das ist eine Applikation welche die [sbLib](https://selfbus.org) mit BCU1 Emulation verwendet.
Das simulierte Gerät ist ein Jung 2138 KNX Schaltaktor mit 8 digitalen Ausgängen.

Zur ETS-Konfiguration die [Jung 2138 VD Produktdatenbank](https://downloads.jung.de/public/de/software/produktdatenbanken/ets2_ets3/de_2138_10.vd1) verwenden. Download unter:
https://downloads.jung.de/public/de/software/produktdatenbanken/ets2_ets3/de_2138_10.vd1

### Build configuration naming (Build-Konfiguration Namen) 
| Abbreviation<br>Abkürzung | Meaning<br>Bedeutung                                                             |
|---------------------------|----------------------------------------------------------------------------------|
| Debug                     | Only for debugging<br>Nur zum debuggen                                           |
| Flashstart                | For use with the Selfbus bootloader<br>Zur Nutzung mit dem Selfbus Bootloader    |
| Release                   | Stand alone firmware<br>Lauffähige Firmware ohne Nutzung des Selfbus Bootloaders |
| o8                        | out8-bcu1<br>                                                                    |
| 4t                        | 4 mounting units (4TE)<br>Teilungseinheiten (4TE)                                |
| bf                        | bus-fail / bus voltage detection<br>Busspannungsausfallerkennnung                |
| ha                        | hand actuation<br>mit Handbedienung                                              |
| bi                        | bi-stable relays<br>für bi-stabile Relais                                        |

## 4MU-ARM Controller ([PCB](https://github.com/selfbus/hardware-merged/tree/main/controller_lpc1115/lpc1115_4MU_MID), [Wiki](https://selfbus.org/wiki/hardware/controller/49-4te-controller-arm-lpc1115)) + out_8x_16A_bistab_4MU (bi-stable relays, bi-stabile relays) ([PCB](https://github.com/selfbus/hardware-merged/tree/main/applications_din/out_8x_16A_bistab_4MU), [Wiki](https://selfbus.org/wiki/devices/outputs/25-8-fold-binary-output-4module-units-lpc1115))
### used IO Ports (verwendete IO Ports):
|Relay #|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)               |
|:-----:|:------:|:--------:|:------------:|---------|-----------------------------------------|
|K1     |Reset   |IO2       |8             |PIO0_7   |                                         |
|K1     |Set     |IO3       |9             |PIO2_10  |                                         |
|K2     |Reset   |IO5       |13            |PIO0_2   |                                         |
|K2     |Set     |I2C-SDA   |11            |PIO0_5   |PIO0_5=open-drain pin: R2 (3K3)-VCC 3,3V |
|K3     |Reset   |PWM       |3             |PIO3_2   |                                         |
|K3     |Set     |Prog. LED |4             |PIO0_6   |                                         |
|K4     |Reset   |IO1       |7             |PIO2_2   |                                         |
|K4     |Set     |IO4       |10            |PIO2_9   |                                         |
|       |        |          |              |         |                                         |
|K5     |Reset   |IO9       |19            |PIO1_0   |                                         |
|K5     |Set     |IO13      |24            |PIO1_5   |                                         |
|K6     |Reset   |IO14      |25            |PIO1_7   |                                         |
|K6     |Set     |IO15      |26            |PIO1_6   |                                         |
|K7     |Reset   |IO10      |20            |PIO1_1   |                                         |
|K7     |Set     |UART-RxD  |18            |PIO3_1   |                                         |
|K8     |Reset   |UART-TxD  |17            |PIO3_0   |                                         |
|K8     |Set     |IO11      |22            |PIO1_2   |                                         |
|       |        |          |              |         |                                         |
|       |VDD     |          |6             |         |EIB_DC (R1 680R to VDD)                  |

-------------

## 4MU-ARM Controller ([PCB](https://github.com/selfbus/hardware-merged/tree/main/controller_lpc1115/lpc1115_4MU_MID), [Wiki](https://selfbus.org/wiki/hardware/controller/49-4te-controller-arm-lpc1115)) + out_8x_10A_4MU ([PCB](https://github.com/selfbus/hardware-merged/tree/main/applications_din/out_8x_10A_4MU), [Wiki](https://selfbus.org/wiki/devices/outputs/23-switching-actuator-8x-230v-10a-4mount-units))
### used IO Ports (verwendete IO Ports):
|Relay #|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)      |
|:-----:|:------:|:--------:|:------------:|---------|--------------------------------|
|REL 1  |        |IO1       |7             |PIO2_2   |                                |
|REL 2  |        |IO2       |8             |PIO0_7   |                                |
|REL 3  |        |IO3       |9             |PIO2_10  |                                |
|REL 4  |        |IO4       |10            |PIO2_9   |                                |
|REL 5  |        |IO5       |13            |PIO0_2   |                                |
|REL 6  |        |IO6       |14            |PIO0_8   |                                |
|REL 7  |        |IO7       |15            |PIO0_9   |                                |
|REL 8  |        |IO8       |16            |PIO2_11  |                                |
|       |        |          |              |         |                                |
|       |        |          |3             |         |PWM                             |

## optional hand actuation LedTaster_4TE ([PCB](https://github.com/selfbus/hardware-merged/tree/main/addons/leds_buttons_lpc922ctrl_4MU), [Wiki (legacy)](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/LedTasterBoard_4TE))
### used IO Ports (verwendete IO Ports):
|Function     |Selfbus IO|Selfbus LT|ARM Port |Description (Beschreibung)      |
|:-----------:|:--------:|:--------:|---------|--------------------------------|
|LED 1 / Btn 1|PIN_LT1   |1         |PIO2_1   |                                |
|LED 2 / Btn 2|PIN_LT2   |2         |PIO0_3   |                                |
|LED 3 / Btn 3|PIN_LT3   |3         |PIO2_4   |                                |
|LED 4 / Btn 4|PIN_LT4   |4         |PIO2_5   |                                |
|LED 5 / Btn 5|PIN_LT5   |5         |PIO3_5   |                                |
|LED 6 / Btn 6|PIN_LT6   |6         |PIO3_4   |                                |
|LED 7 / Btn 7|PIN_LT7   |7         |PIO1_10  |                                |
|LED 8 / Btn 8|PIN_LT8   |8         |PIO0_11  |                                |
|Readback     |PIN_LT9   |9         |PIO2_3   |                                |

## See [config.h](inc/config.h) for additional information.