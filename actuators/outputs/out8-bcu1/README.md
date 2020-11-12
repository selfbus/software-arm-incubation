out8-bcu1
=========

This is an example application that uses the SBLib with BCU1 emulation.
The emulated device is a Jung 2138 with 8 digital outputs.

Please use the [Jung 2138 VD](http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10) for ETS configuration from this site:
http://www.jung.de/722/downloads/technische-downloads/technische-downloads/?search=2138&rpp=10


THIS CODE IS UNTESED AND WILL NOT WORK AS OF NOW.

Used IO Ports
-------------

|Selfbus IO|ARM Port |Function                        |
|----------|---------|--------------------------------|
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |Output pin                      |
|          |         |                                |
|          |PIO0_0   |Reset                           |
|          |         |LPC-Link clock                  |
|          |         |LPC-Link data I/O               |
|          |PIO2_0   |Programming mode led and button |
|          |PIO1_8   |EIB input                       |
|          |PIO1_9   |EIB output                      |
