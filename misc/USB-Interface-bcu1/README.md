# KNX USB-Interface / Busmonitor / Programmer

For normal operation, connect the connector JP7 (ISP) of the KNX-module (TS_ARM) with a 10pole cable to connector P1 of the USB-interface

### Switch
S1: Change operation modes

| Mode           | Description                                                                                 |
|----------------|---------------------------------------------------------------------------------------------|
| KNX-Interface  | USB-HID device which can be used in ETS                                                     |
| KNX-Busmonitor | KNX traffic is output in readable form on the virtual serial port                           |
| USB-Monitor    | Packet transfer between ETS and KNX is send to the virtual serial port                      |
| Programmer     | Serial port for programming a Selfbus ARM device connected to P3 (Prog-If) with Flashmagic. |

Virtual serial port settings: 115200 baud, 8 data bits, no parity, 1 stop bit

[Thread in Selfbus forum](https://selfbus.org/forum/viewtopic.php?f=6&t=487)

### Jumper

| Jumper | Function   | Description                  |
|:------:|------------|------------------------------|
|  JP1   | USB ISP    | enable ISP 11uxx             |
|  JP2   | ISP UART   | off=ISP USB, on=ISP UART     |
|        |            |                              |
|  JP4   | KNX ISP    | KNX module KNX-uC ISP enable |
|  JP5   | KNX ISP    | KNX-Side ISP enable          |
|  JP6   | ISP USR-uC | VCC enable for KNX module    |

- To update the KNX-module (P4, TS_ARM) firmware disconnect USB and KNX-supply,  
  set jumpers JP4 and JP5,  
  connect USB and KNX-supply,   
  all mode LEDs should be off, indicating the firmware update mode. 

- To program/flash the MCU U1 (11uxx ,USB) via ISP set JP1 & JP2.

### Connectors

| Conn. | Function   | Description |
|-------|------------|-------------|
| P1    | ISP KNX-uC | ISP TS_ARM  |
| P2    | ISP USB-uc | ISP 11uxx   |
| P3    | Prog-If    | ISP USR-uC  |


### Firmware

| MCU         | Github                                                                                                          |
|-------------|-----------------------------------------------------------------------------------------------------------------|
| P4 (TS_ARM) | [USB-IF_Knx](https://github.com/selfbus/software-arm-incubation/tree/master/misc/USB-Interface-bcu1/USB-IF_Knx) |
| U1 (11uxx)  | [USB-IF_Usb](https://github.com/selfbus/software-arm-incubation/tree/master/misc/USB-Interface-bcu1/USB-IF_Usb) |

### Hardware / PCB
[USB-Interface ARM](https://github.com/selfbus/hardware-incubation/tree/master/misc/USB-Interface%20ARM)