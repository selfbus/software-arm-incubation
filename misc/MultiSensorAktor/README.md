Multi-Sensor-Aktor
==================

Generic Firmware for LPC115-based controller, like TS-ARM or 4TE-Controller.
Features:
<ul>
<li> Each available pin can be independently used as:
 <ul>
   <li> Input
     <ul>
       <li> with pullup, pulldown or floating</li>
       <li> inverted or not</li>
       <li> Support for long keypress detection with separate action</li>
       <li> Input debouncer</li>
       <li> Actions on Hi/Lo: On, Off, Toggle or nothing</li>
     </ul>
   </li>
  <li> Output
    <ul>
      <li> inverted or not</li>
      <li> blinking support, on 1, on 0, on separate object</li>
      <li> blink phase on/off configurable (don't use to low number, this is no PWM!)</li>
    </ul>
   </li>
  <li> DHT11/DHT22
    <ul>
      <li> Temperature as DPT9 (16bit) and DTP14 (32bit) float</li>
      <li> Temperature offset configurable</li>
      <li> optional Fan-phase before measurement for bad ventilated environments</li>
      <li> configurable measurement interval</li>
      <li> Humidity as DPT9</li>
    </ul>
   </li>
  <li> PWM
    <ul>
      <li> 0-100% via ComObject</li>
      <li> inverted or not</li>
      <li> fixed 100Hz, on every pin, hardware timers where possible (see "PWM outputs" below)</li>
    </ul>
   </li>
  </ul>
 </li>
</ul>
  
Work in progress:
<ul>
<li> Support for sensors/actors on I2C, SPI
  <ul>
    <li> PCA955D 16 Bit IO extender, Input/Output/PWM as on ARM-pins</li>
    <li> CCS811</li>
    <li> BH1750</li>
    <li> DS3231</li>
    <li> iAQ-Core</li>
    <li> SHT2x</li>
    <li> radio based sensors via RFM69</li>
  </ul>
</li>
</ul>

Knxprod:
the contents of the folder KnxProd (not the folder itself) have to zipped to MultiSensorAktor.knxprod. This file can be signed with the KnxProdEditor (https://github.com/knxprodeditor/KNXprodEditor), just open and save. Do not edit in the editor, it doesn't support all necessary features! The signed knxprod can be imported into ETS5 & 6 (maybe also 4)

## Building with CMake

Without MCUXpresso, e.g. on Linux with Debian's `gcc-arm-none-eabi` (see `cmake/toolchains/README.md` of the sblib):

```sh
cmake --preset arm -DCMAKE_TOOLCHAIN_FILE=<sblib repository>/cmake/toolchains/arm-none-eabi_gnu.cmake -DSB_PATH_TO_SBLIB_REPO=<sblib repository>
cmake --build --preset release-arm
```

The result in `cmake-build/arm/Release` starts at 0x3000 like the configuration `Flashstart 0x3000 Release`.
`linkscripts/MultiSensorAktor_Flashstart_0x3000_combined.ld` ends the flash at 0xE000, where the MemMapper starts.
Only the Release build fits, the Debug build (-O0) needs about 72 KB and fails at link time.

## PWM outputs

Every ARM pin can be a PWM output (100 Hz, value 0-255 from the 1-byte object, 255 = on all the time), all at once
if need be. `PwmController` (`inc/PwmController.h`) drives a pin with a timer in its PWM mode if the pin has a match
output of a free timer, and switches all other pins in software from the interrupt of CT32B1: one interrupt at the
start of each period and one per point in time a pin changes. CT16B1 belongs to the bus and is never used.

| timer | period | match outputs used for PWM | on the io-16x-fused-fm |
| --- | --- | --- | --- |
| CT16B0 | MR3 | MR0 PIO0_8/PIO3_0, MR1 PIO0_9/PIO3_1, MR2 PIO3_2 | IO9, IO12 |
| CT32B0 | MR1 | MR0 PIO1_6/PIO2_5, MR2 PIO0_1/PIO2_7, MR3 PIO0_11/PIO2_8 | IO3, IO15, IO16 |
| CT32B1 | MR0 | MR3 PIO1_4 (MR1: edges of the software pins) | IO13 |

A match register drives all of its pins alike, so only the first pin of a register (in the order of the channels)
gets it; the others become software pins. On the io-16x-fused-fm 6 of the 16 IOs are hardware pins, of all 36 pins
of the application 7.

Differences between the two kinds:
- A hardware pin runs without the CPU, also while the flash is written (interrupts off for about 100 ms per page,
  during a download). It takes a new value at once; if the pin had not switched in that period yet and the new switch
  point is already past, it stays low for the rest of this one period (the start level, "on" if inverted).
- A software pin takes a new value at the start of the next period. Its interrupt has the low priority, below the bus:
  its edges come some microseconds late while the bus interrupt runs, and they stand still while the interrupts are
  off (writing the flash). It costs at most one interrupt per period and pin.

`Invert` ("Ausgang invertieren", 0x80 from ETS) swaps the levels. The lock and the fixed value work as before
(`PWMPin::targetValue()`); the status object isn't sent. PWM pins of a PCA9555D still output nothing.

## Unit tests

`test/` builds the unit tests on the host (x64) against the CPU emulation and Catch of the sblib repository,
see `test/README.md`.
