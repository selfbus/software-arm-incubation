# 4Sense
This project aims replace the old [Selfbus](https://selfbus.myxwiki.org/xwiki/bin/view/Main/) LPC922 based 
[4Sense](https://selfbus.myxwiki.org/xwiki/bin/view/Geräte/Sensoren/4Sense_Temp_Feuchte) project with its ARM counter part.

## Status
Currently the implementation has most functionality of the DS18x20 series Temperature 
Sensors covered, but only the first Sensor is implemented. 

Diagnostics are currently not implemented because i have no idea what the specs are. 


## Building
I use cmake for building, but a sparsely maintained MCUxpresso project is there
as i use that occasionally for stepping through code.
There is a logging facility which uses the ARM serial port, so no semihosting is
needed. Specifying `-DLOGGING` for cmake or adding `LOGGING` to the C++ Preprocessor 
settings in Eclipse will activate it.

This project requires my version of sblib as i made several changes to the ds18x20
and onewire classes. It also should be built with `-DSERIAL_WRITE_DIRECT` if logging
is to succeed.
