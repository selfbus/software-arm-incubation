#KNX USB-Interface / Busmonitor / Programmer

###Switch
S1: Switch through the different operation modes (KNX-Interface, KNX-Busmonitor, USB-Monitor, Programmer)

###Jumper
| Jumper | Function   | Description                 |
|:------:|------------|-----------------------------|
|  JP1   | USB ISP    | enable ISP 11uxx ISP enable |
|  JP2   | ISP UART   | off=ISP USB, on=ISP UART    |
|        |            |                             |
|  JP4   | KNX ISP    | KNX-Side KNX-uC ISP enable  |
|  JP5   | KNX ISP    | KNX-Side ISP enable         |
|  JP6   | ISP USR-uC | VCC enable for TS_ARM       |

###Connectors
| Conn. | Function   | Description |
|-------|------------|-------------|
| P1    | ISP KNX-uC | ISP TS_ARM  |
| P2    | ISP USB-uc | ISP 11uxx   |
| P3    | Prog-If    | ISP USR-uC  |

- for bus monitor connect the ISP-header of the TS_ARM (JP7) with a 10pole cable to P1
- to flash the TS_ARM set jumpers JP4 and JP5
- to program/flash the 11uxx via ISP set JP1 & JP2

###Firmware

| MCU    | Github                                                                                                          |
|--------|-----------------------------------------------------------------------------------------------------------------|
| TS_ARM | [USB-IF_Knx](https://github.com/selfbus/software-arm-incubation/tree/master/misc/USB-Interface-bcu1/USB-IF_Knx) |
| 11uxx  | [USB-IF_Usb](https://github.com/selfbus/software-arm-incubation/tree/master/misc/USB-Interface-bcu1/USB-IF_Usb) |

### Hardware / PCB
[USB-Interface ARM](https://github.com/selfbus/hardware-incubation/tree/master/misc/USB-Interface%20ARM)