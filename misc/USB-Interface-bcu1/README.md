# KNX USB-Interface / Busmonitor / Programmer

<!-- TOC -->
* [KNX USB-Interface / Busmonitor / Programmer](#knx-usb-interface--busmonitor--programmer)
    * [Switch](#switch)
    * [Jumper](#jumper)
    * [Connectors](#connectors)
    * [Firmware](#firmware)
    * [Hardware / PCB](#hardware--pcb)
  * [Usage with knxd](#usage-with-knxd)
    * [Example `knxd.ini`](#example-knxdini)
    * [Start in terminal](#start-in-terminal)
    * [Create simple knxd.ini for USB-Interface](#create-simple-knxdini-for-usb-interface)
    * [udev rule](#udev-rule)
<!-- TOC -->

For normal operation, connect the connector JP7 (ISP) of the KNX-module (TS_ARM) with a 10pole cable to connector P1 of the USB-interface.

### Switch
S1: Change operation modes

| Mode           | Description                                                                                                                    |
|----------------|--------------------------------------------------------------------------------------------------------------------------------|
| KNX-Interface  | USB-HID device which can be used in ETS                                                                                        |
| KNX-Busmonitor | KNX traffic is output in readable form on the virtual serial port                                                              |
| USB-Monitor    | Packet transfer between ETS and KNX is send to the virtual serial port                                                         |
| Programmer     | Serial port for programming a Selfbus ARM device connected to P3 (Prog-If) with [Flashmagic](https://www.flashmagictool.com/). |

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

| MCU         | Github                                                                                                        |
|-------------|---------------------------------------------------------------------------------------------------------------|
| P4 (TS_ARM) | [USB-IF_Knx](https://github.com/selfbus/software-arm-incubation/tree/main/misc/USB-Interface-bcu1/USB-IF_Knx) |
| U1 (11uxx)  | [USB-IF_Usb](https://github.com/selfbus/software-arm-incubation/tree/main/misc/USB-Interface-bcu1/USB-IF_Usb) |

### Hardware / PCB
[USB-Interface ARM](https://github.com/selfbus/hardware-merged/tree/main/misc/usb_knx_interface_lpc1115)

## Usage with knxd

### Example `knxd.ini`

An example `knxd.ini` can be found [here](doc/knxd.ini).

### Start in terminal

`sudo knxd -e 0.0.1 -E 0.0.2:8 -D -T -R -S -f9 -B log -t 1023 -b usb:`

### Create simple knxd.ini for USB-Interface
`/usr/lib/knxd_args -e 0.0.1 -E 0.0.2:8 -D -T -R -S -f9 -B log -t 1023 -b usb: >> knxd.ini`

### udev rule

In case the USB-Interface is not listed in udev rules installed by knxd `/usr/lib/udev/rules.d/60-knxd.rules`<br>
create file `/etc/udev/rules.d/70-knxd.rules` and add following line<br>
`ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="147b", ATTR{idProduct}=="5120", OWNER="knxd", MODE="0600"` to it,<br>
or run in a terminal<br>
`echo 'ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="147b", ATTR{idProduct}=="5120", OWNER="knxd", MODE="0600"' | sudo tee --append /etc/udev/rules.d/70-knxd.rules`



