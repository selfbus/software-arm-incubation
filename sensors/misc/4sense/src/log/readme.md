# Logging
This log facility uses the sblib `Serial` class and the default serial port of the ARM chip. I'm using a special connector that connects to an [LPC-Link2](https://www.nxp.com/design/microcontrollers-developer-resources/lpc-microcontroller-utilities/lpc-link2:OM13054) board and has the connector `Pin 5` going to the `TX` port of a USB serial dongle and pin 3 to its `RX` port. `Pin 3` is not used and was installed *just in case*.

Since there was a need to make a cable for the 2mm headers on the selfbus boards this was really just another mod.