/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "Hoermann.h"
#include "config.h"
#include <sblib/serial.h>
#include <sblib/digital_pin.h>
#include <sblib/timer.h>


constexpr uint8_t DCTRL = 1 << 4; //!< RS485 direction control bit

void Hoermann::begin(const uint32_t pinTx, const uint32_t pinRx, const uint32_t pinRTS)
{
    if ((pinRTS != PIO1_5) && (pinRTS != 0))
    {
        fatalError(); // Only PIO1_5 supports hardware RS485 direction control on this board
    }
    serial.setRxPin(pinRx);
    serial.setTxPin(pinTx);
    pinMode(pinRx, SERIAL_RXD | INPUT | PULL_UP | HYSTERESIS);
    serial.setErrorCallback([](const uint32_t lineStatus, void* context) {
        static_cast<Hoermann*>(context)->onSerialError(lineStatus);}, this);

    serial.begin(19200, SERIAL_8N1);
    if (pinRTS != 0)
    {
        ///\todo replace then sblib SERIAL_RTS is committed
        //pinMode(pinRTS, SERIAL_RTS); // RTS for hardware RS485 driver enable
        pinMode(pinRTS, OUTPUT | PinModeFunc(PF_RTS)); // RTS for hardware RS485 driver enable
        LPC_UART->RS485CTRL = DCTRL; // Enable RS485 direction control on RTS pin
    }

#ifdef DEBUG // delete on release
    if (pinRTS == 0)
    {
        serial.println("Selfbus garage door gateway");
        serial.println("frame;count;delta ms;crc status;frame crc;calculated crc;overrun;parity error;frame error");
    }
#endif
}

void Hoermann::onSerialError(uint32_t lineStatus)
{
    // Callback function runs in interrupt context, keep it short
    if (lineStatus & LSR_BI)
    {
        // break always comes with a framing error, so we clear it here
        lineStatus &= ~LSR_FE;

        // A break indicates start of a new transmission
        isrBreakDetected = true;
    }

    if (lineStatus & LSR_FE)
    {
        //serial.print(" frame error: ");
        isrFrameErrorCount++;
        digitalWrite(PIO_LED_2, !digitalRead(PIO_LED_2));
    }

    if (lineStatus & LSR_PE)
    {
        isrParityErrorCount++;
        serial.println(" parity error");
    }

    if (lineStatus & LSR_OE)
    {
        isrOverrunCount++;
        serial.println(" Overrun error");
    }
}

void Hoermann::loop()
{
#if 0
    int32_t serialByte = serial.read();
    while (serialByte != -1)
    {
//        serial.write(serialByte);
        serial.print(serialByte, HEX, 2);
        serialByte = serial.read();
        lastRxReceiveTick = millis();
    }
    return;
#endif

    if (isrBreakDetected)
    {
        // Start of a new transmission/frame
        isrBreakDetected = false;
        isrFrameErrorCount = 0;
        isrParityErrorCount = 0;
        isrOverrunCount = 0;
        resetSerialBuffer();
        digitalWrite(PIO_LED_1, !digitalRead(PIO_LED_1));
    }

    if ((serialRxTimeout.expired() || serialRxTimeout.stopped()) &&
        bufferPosition > 0)
    {
        printSerialBuffer(); serial.print(SEPARATOR);
        serial.print(bufferPosition + 1); serial.print(SEPARATOR);
        const uint32_t elapsedMs = millis() - lastRxReceiveTick;
        serial.print(elapsedMs); serial.print(SEPARATOR);
        if (!crcOKofSerialBuffer())
        {
            serial.print("error");
        }
        else
        {
            serial.print("ok");
        }
        serial.print(SEPARATOR);
        serial.print(serialBuffer[bufferPosition], HEX, 2); serial.print(SEPARATOR);
        serial.print(crc.current, HEX, 2); serial.print(SEPARATOR);
        serial.print(isrOverrunCount); serial.print(SEPARATOR);
        serial.print(isrParityErrorCount); serial.print(SEPARATOR);
        serial.print(isrFrameErrorCount);
        serial.println();
        resetSerialBuffer();
    }

    int32_t serialByte = serial.read();
    while(serialByte > -1)
    {
        putSerialBuffer(serialByte);
        serialRxTimeout.start(SERIAL_RX_TIMEOUT_MS);
        lastRxReceiveTick = millis();
        serialByte = serial.read();
    }



#if 0
    int c = serial.read();

    if (c  != -1)
    {
        switch (stateMachine)
        {
        case WAIT_FOR_BREAK:
            if (c == 0x55)
            {
                crc.reset();
                crc.update(c);
                stateMachine = WAIT_FOR_ADDRESS;
            }
            break;
        case WAIT_FOR_ADDRESS:
            crc.update(c);
            address = c;
            stateMachine = WAIT_FOR_COUNTER_AND_LENGTH;
            break;
        case WAIT_FOR_COUNTER_AND_LENGTH:
            crc.update(c);
            counter = c >> 4;
            length = c & 0x0F;
            position = 0;
            if (length == 0)
            {
                stateMachine = WAIT_FOR_CRC;
            }
            else
            {
                stateMachine = WAIT_FOR_DATA;
            }
            break;
        case WAIT_FOR_DATA:
            crc.update(c);
            data[position++] = c;
            if (position == length)
            {
                stateMachine = WAIT_FOR_CRC;
            }
            break;
        case WAIT_FOR_CRC:
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
            stateMachine = WAIT_FOR_BREAK;
            break;
        }
    }
#endif
}

void Hoermann::sendResponse(const uint8_t addr, uint8_t bytes[], const uint8_t len)
{
    const auto temp = static_cast<uint8_t>((myCounter++ & 0x0F) << 4 | len);

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

void Hoermann::light(const bool on)
{
    if (on != state.lightRelay)
    {
        slaveResponse[0] |= 0x08;
    }
}

void Hoermann::emergencyStop(const bool on)
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

uint8_t Hoermann::getSerialBufferSize() const
{
    return sizeof(serialBuffer)/sizeof(serialBuffer[0]);
}

void Hoermann::resetSerialBuffer()
{
    bufferPosition = -1;
    memset(serialBuffer, 0, getSerialBufferSize());
    crc.reset();
}

void Hoermann::putSerialBuffer(const uint8_t toPut)
{
    bufferPosition++;
    if ((bufferPosition < 0) || (bufferPosition >= getSerialBufferSize()))
    {
        fatalError();
    }
    serialBuffer[bufferPosition] = toPut;
    // crc.update(serialBuffer[bufferPosition]);
}

void Hoermann::printSerialBuffer() const
{
    for (uint16_t i = 0; i <= bufferPosition; i++)
    {
        serial.print(serialBuffer[i], HEX, 2);
        if (i != bufferPosition)
        {
            serial.print(" ");
        }
    }
}

bool Hoermann::crcOKofSerialBuffer()
{
    if (bufferPosition < 3)
    {
        // We need at least 3 bytes for a valid frame (address, counter/length, crc)
        return false; // No data in buffer
    }

    crc.reset();
    for (uint16_t i = 0; i < bufferPosition; i++) // Exclude last byte, which is the crc
    {
        crc.update(serialBuffer[i]);
    }
    return crc.matches(serialBuffer[bufferPosition]);
}

#ifdef DEBUG // delete on release
void Hoermann::debugSendPeriodic()
{
    const uint32_t elapsedMs = elapsed(lastSysTick);
    if (elapsedMs >= 10)
    {
//        serial.write(static_cast<byte>(0));
//        serial.write(static_cast<byte>(0b01010101));
//        serial.write(static_cast<byte>('0'));
//        serial.write(static_cast<byte>('1'));
//        serial.write(lastSend);
        lastSend++;
        if (lastSend > 'z')
        {
            lastSend = 'a';
        }
        lastSysTick = millis();
    }
}
#endif
