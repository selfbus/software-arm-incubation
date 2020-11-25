# out8-bcu1

english:
This is an application that uses the [SBLib](https://selfbus.org) with BCU1 emulation.
The emulated device is a Jung 2138 KNX switch actuator with 8 digital outputs.

Please use the [Jung 2138 VD database](http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10) for ETS configuration from this site:
http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10
-------------
german:
Das ist eine Applikation welche die [SBLib](https://selfbus.org) mit BCU1 Emulation verwendet.
Das simulierte Gerät ist ein Jung 2138 KNX Schaltaktor mit 8 digitalen Ausgängen.

Zur ETS-Konfiguration die [Jung 2138 VD Produktdatenbank](http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10) verwenden. Download unter:
http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10
-------------


## 4TE-ARM Controller ([PCB](https://github.com/selfbus/hardware-incubation/tree/master/Controller/lpc1115_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/Controller_1115_4TE)) + out8_16A (bi-stable relays, bi-stabile relais) ([PCB](https://github.com/selfbus/hardware/tree/master/Apps/out8_16A_1.0), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/8fach%20Bin%C3%A4rausgang%20230V%2016A%20f%C3%BCr%20LPC1115%20Controller/))
### used IO Ports (verwendete IO Ports):
|Relais#|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)               |
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

## 4TE-ARM Controller ([PCB](https://github.com/selfbus/hardware-incubation/tree/master/Controller/lpc1115_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/Controller_1115_4TE)) + relais8_4te ([PCB](https://github.com/selfbus/hardware/tree/master/Apps/relais8_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/Bin%C3%A4rausgang_8x230_4TE))
### used IO Ports (verwendete IO Ports):
|Relais#|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)      |
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

## optional hand actuation LedTaster_4TE ([PCB](https://github.com/selfbus/hardware/tree/master/Controller/lpc922_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/LedTasterBoard_4TE))
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

See inc/config.h for additional information.