/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "Hoermann.h"
#include <sblib/serial.h>
#include <sblib/digital_pin.h>

Hoermann::Hoermann()
{
    LPC_UART->RS485CTRL = 1 << DCTRL;
    serial.setRxPin(PIO1_6);
    serial.setTxPin(PIO1_7);
    serial.begin(19200, SERIAL_8N1);
    LPC_UART->RS485CTRL = 1 << DCTRL;
}

void Hoermann::loop()
{
    int c = serial.read();

    if (c  != -1)
    {
    	switch (stateMachine)
    	{
    	case 0:	// Wait for sync
    		if (c == 0x55)
    		{
    			crc.reset();
    			crc.update(c);
    			stateMachine++;
    		}
    		break;
    	case 1: // address
			crc.update(c);
    		address = c;
    		stateMachine++;
    		break;
    	case 2: // counter + length
			crc.update(c);
    		counter = c >> 4;
    		length = c & 0x0F;
    		position = 0;
    		if (length == 0)
			{
    			stateMachine = 4;
			}
    		else
    		{
    			stateMachine = 3;
    		}
    		break;
    	case 3:
			crc.update(c);
    		data[position++] = c;
    		if (position == length)
    		{
    			stateMachine++;
    		}
    		break;
    	case 4:
    		if (crc.matches(c))
    		{
    			switch (address)
    			{
    			case 0x00: // Broadcast
    				if (length == 2)
    				{
    					state.doorOpen = data[0] & (1 << 0);
    					state.doorClosed = data[0] & (1 << 1);
    					state.optionRelay = data[0] & (1 << 2);
    					state.lightRelay = data[0] & (1 << 3);
    					state.error = data[0] & (1 << 4);
    					state.directionDown = data[0] & (1 << 5);
    					state.moving = data[0] & (1 << 6);
    					state.ventingPos = data[0] & (1 << 7);
    					state.preWarning = data[1] & (1 << 0);
    					break;
    				}
    				break;
    			case 0x28: // UAP1
    				switch (data[0])
    				{
    				case 0x01: // Slave scan
    				{
    					uint8_t buf[2] = { 0x14, 0x28 };
    					sendResponse(data[1], buf, 2);
    					break;
    				}
    				case 0x20: // Slave status request
    				{
    					uint8_t buf[] = { 0x29, slaveResponse[0], slaveResponse[1] };
    					sendResponse(data[1], buf, 3);
    					slaveResponse[0] = 0;
    					break;
    				}
    				}
    				break;

    			}
    		}
    		stateMachine = 0;
    		break;
    	}
    }
}

void Hoermann::sendResponse(uint8_t addr, uint8_t bytes[], uint8_t len)
{
	uint8_t temp = (uint8_t)((myCounter++ & 0x0F) << 4 | len);

	CRC innerCRC;
	innerCRC.reset();
	innerCRC.update(0x55);
	innerCRC.update(addr);
	innerCRC.update(temp);
	for (int i = 0; i < len; i++)
	{
		innerCRC.update(bytes[i]);
	}

	serial.write(0x55);
	serial.write(addr);

	serial.write(temp);

	serial.write(bytes, len);
	serial.write(innerCRC.current);
}

void Hoermann::open()
{
	if (!state.doorOpen && (!state.moving || state.directionDown))
	{
		 slaveResponse[0] = (slaveResponse[0] & 0xFFE8) | 0x01;
	}
}

void Hoermann::close()
{
	if (!state.doorClosed && (!state.moving || !state.directionDown))
	{
		 slaveResponse[0] = (slaveResponse[0] & 0xFFE8) | 0x02;
	}
}

void Hoermann::stop()
{
	if (state.moving)
	{
		slaveResponse[0] = (slaveResponse[0] & 0xFFE8) | 0x04;
	}
}

void Hoermann::venting()
{
	slaveResponse[0] = (slaveResponse[0] & 0xFFE8) | 0x10;
}

void Hoermann::light(bool on)
{
	if (on != state.lightRelay)
	{
		slaveResponse[0] |= 0x08;
	}
}

void Hoermann::emergencyStop(bool on)
{
	if (on)
	{
		slaveResponse[1] = 0x00;
	}
	else
	{
		slaveResponse[1] = 0x10;
	}
}
