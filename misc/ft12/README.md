# ft12 RaspiHAT (ARM-Version) 

This is an application that uses the [SBLib](https://selfbus.org) to emulate a KNX ft1.2 device with a Raspberry Pi.

## [PCB RPi-HAT_BUS-Controller](https://github.com/selfbus/hardware-incubation/tree/master/module/rpi_bus_controller)

## [Wiki entry](https://selfbus.org/wiki/devices/schnittstellen/37-ft1-2-adapter-fuer-raspberry-pi)

Example command line for knxd:  
```
knxd --trace=1023 -e 2.3.240 -E 2.3.241:9 -D -R -T -S -i -B log -b ft12:/dev/ttyKNX1
```

Example /etc/knxd.ini
```
[A.tcp]
server = knxd_tcp
systemd-ignore = true

[B.ft12]
device = /dev/ttyKNX1
driver = ft12
filters = log,C.pace

[C.pace]
delay = 10
filter = pace
[debug-main]
trace-mask = 0x3ff
[debug-server]
name = mcast:knxd

[main]
addr = 2.3.240
client-addrs = 2.3.241:9
connections = server,A.tcp,B.ft12
debug = debug-main
systemd = systemd

[server]
debug = debug-server
discover = true
router = router
server = ets_router
tunnel = tunnel
```