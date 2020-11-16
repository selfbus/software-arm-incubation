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
|Relais#|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)      |
|:-----:|:------:|:--------:|:------------:|---------|--------------------------------|
|K1     |Reset   |IO2       |8             |PIO0_7   |                                |
|K1     |Set     |IO3       |9             |PIO2_10  |                                |
|K2     |Reset   |IO5       |13            |PIO0_2   |                                |
|K2     |Set     |I2C-SDA   |11            |PIO0_5   |why R2 (3K3) to VCC 3,3V ??     |
|K3     |Reset   |PWM       |3             |PIO3_2   |                                |
|K3     |Set     |Prog. LED |4             |PIO0_6   |                                |
|K4     |Reset   |IO1       |7             |PIO2_2   |                                |
|K4     |Set     |IO4       |10            |PIO2_9   |                                |
|       |        |          |              |         |                                |
|K5     |Reset   |IO9       |19            |PIO1_0   |                                |
|K5     |Set     |IO13      |24            |PIO1_5   |                                |
|K6     |Reset   |IO14      |25            |PIO1_7   |                                |
|K6     |Set     |IO15      |26            |PIO1_6   |                                |
|K7     |Reset   |IO10      |20            |PIO1_1   |                                |
|K7     |Set     |UART-RxD  |18            |PIO3_1   |                                |
|K8     |Reset   |UART-TxD  |17            |PIO3_0   |                                |
|K8     |Set     |IO11      |22            |PIO1_2   |                                |
|       |        |          |              |         |                                |
|       |VDD     |          |6             |         |EIB_DC (R1 680R to VDD)         |

-------------

## 4TE-ARM Controller ([PCB](https://github.com/selfbus/hardware-incubation/tree/master/Controller/lpc1115_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Technik/Controller_1115_4TE)) + relais8_4te ([PCB](https://github.com/selfbus/hardware/tree/master/Apps/relais8_4te), [Wiki](https://selfbus.myxwiki.org/xwiki/bin/view/Ger%C3%A4te/Ausg%C3%A4nge/Bin%C3%A4rausgang_8x230_4TE))
### used IO Ports (verwendete IO Ports):
|Relais#|Function|Selfbus IO|Selfbus 26-pol|ARM Port |Description (Beschreibung)      |
|:-----:|:------:|:--------:|:------------:|---------|--------------------------------|
|REL1   |        |IO1       |7             |PIO2_2   |                                |
|       |        |IO2       |8             |PIO0_7   |                                |
|       |        |IO3       |9             |PIO2_10  |                                |
|       |        |IO4       |10            |PIO2_9   |                                |
|       |        |IO5       |13            |PIO0_2   |                                |
|       |        |IO6       |14            |PIO0_8   |                                |
|       |        |IO7       |15            |PIO0_9   |                                |
|       |        |IO8       |16            |PIO2_11  |                                |
|       |        |          |              |         |                                |
|       |        |          |              |         |                                |
|       |        |          |3             |         |PWM                             |

See config.h for addtional information.