# rain

## Rain sensor SCN-RS1R1.01 info
[Manual](https://www.mdt.de/fileadmin/user_upload/user_upload/MDT_THB_Technische_Handbuecher/MDT_THB_Regensensor.pdf)  
[KNX product database](https://www.mdt.de/fileadmin/user_upload/user_upload/MDT_Datenbank/MDT_KP_SCN_01_Rain_Sensor_V11.knxprod)  

## Building
Use cmake for building, but a sparsely maintained MCUxpresso project is there
as i use that occasionally for stepping through code.
There is a logging facility which uses the ARM serial port, so no semihosting is
needed. Specifying `-DLOGGING` for cmake or adding `LOGGING` to the C++ Preprocessor 
settings in Eclipse will activate it.

This project requires my version of sblib as i made several changes to the ds18x20
and onewire classes. It also should be built with `-DSERIAL_WRITE_DIRECT` if logging
is to succeed.
