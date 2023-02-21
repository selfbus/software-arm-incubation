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
      <li> fixed 100Hz</li>
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

Compilation:
the project is based on the Object Oriented version of the sblib, which can be found at https://github.com/gnampf1/software-arm-lib/tree/DEV-Timing+0x07B0%2BProperties. It won't compile with the standard lib!

Knxprod:
the contents of the folder KnxProd (not the folder itself) have to zipped to MultiSensorAktor.knxprod. This file can be signed with the KnxProdEditor (https://github.com/knxprodeditor/KNXprodEditor), just open and save. Do not edit in the editor, it doesn't support all necessary features! The signed knxprod can be imported into ETS5 & 6 (maybe also 4)
