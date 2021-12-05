Multi-Sensor-Aktor
==================

Generic Firmware for LPC115-based controller, like TS-ARM or 4TE-Controller.
Features:
- Each available pin can be independently used as:
  -> Input
     --> with pullup, pulldown or floating
     --> inverted or not
     --> Support for long keypress detection with separate action
     --> Input debouncer
     --> Actions on Hi/Lo: On, Off, Toggle or nothing
  -> Output
     --> inverted or not
     --> blinking support, on 1, on 0, on separate object
     --> blink phase on/off configurable (don't use to low number, this is no PWM!)
  -> DHT11/DHT22
     --> Temperature as DPT9 (16bit) and DTP14 (32bit) float
     --> Temperature offset configurable
     --> optional Fan-phase before measurement for bad ventilated environments
     --> configurable measurement interval
     --> Humidity as DPT9
  -> PWM
     --> 0-100% via ComObject
     --> inverted or not
     --> fixed 100Hz
     
     
Work in progress:
- Support for sensors/actors on I2C, SPI
  -> PCA955D 16 Bit IO extender, Input/Output/PWM as on ARM-pins
  -> CCS811
  -> BH1750
  -> DS3231
  -> iAQ-Core
  -> SHT2x
  -> radio based sensors via RFM69