Serial Bus Monitor Example
==========================

This is a bus monitor that outputs all received telegrams to the serial port.
The serial port is used with 115200 baud, 8 data bits, no parity, 1 stop bit.
In addition, one can send telegram on the bus via the serial port by sending 
a sequence of bytes :
	CC aa bb cc dd ee
	
where CC is the number of bytes of the the telegram to send
	  aa,bb,cc,dd,ee   the telegram data (without the checksum)
 
