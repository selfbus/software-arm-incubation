/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "config.h"
#include "Hoermann.h"
#include "hoermann_protocol.h"
#include "hoermann_dump.h"
#include <sblib/serial.h>
#include <sblib/serial_registers.h>
#include <sblib/timer.h>
#include <sblib/utils.h>
#include <sblib/interrupt.h>

/**
 * @brief The default baudrate of the UAP1.
 */
constexpr SerialBaudRate hoermannBaudrate19200 = SERIAL_BAUD_RATE_19200;

/**
 * @brief BREAK condition duration in microseconds based of the baudrate.
 * 
 * 676 microseconds at 19200 baud with 13 low bits.
 */
constexpr uint32_t breakConditionMicroseconds = 13 * (1000000/hoermannBaudrate19200);

/**
 * @brief Duration in microseconds after the BREAK was released.
 * 
 * 104 microseconds at 19200 baud with 2 high bits.
 */
constexpr uint32_t breakReleaseMicroseconds = 2 * (1000000/hoermannBaudrate19200);

void Hoermann::begin(const uint32_t pinTx, const uint32_t pinRx, const uint32_t pinRTS)
{
    if ((pinRTS != PIO1_5) && (pinRTS != 0))
    {
        fatalError(); // Only PIO1_5 supports hardware RS485 direction control on this board
    }

    pinRx_ = pinRx;
    pinRxPort_ = digitalPinToPort(pinRx_);
    serial.setRxPin(pinRx_);
    serial.setTxPin(pinTx);
    pinMode(pinRx_, rxDefaultPinMode);
    serial.setErrorCallback([](uint8_t errorFlags, const uint8_t faultyByte, void* context)
            {
                static_cast<Hoermann*>(context)->onSerialError(errorFlags, faultyByte);
            }, this);

    constexpr auto  lvl = RxTriggerLevel::CHAR_1;
    serial.begin(hoermannBaudrate19200, SERIAL_8N1, lvl);

    serial.setTimeout(0);
    if (pinRTS != 0)
    {
        pinMode(pinRTS, SERIAL_RTS); // RTS for hardware RS485 driver enable
        LPC_UART->RS485CTRL = DCTRL; // Enable RS485 direction control on RTS pin
    }

    if (pinRTS == 0) ///\todo delete whole if on release
    {
        pinMode(PIO_BREAK_TRIGGER, OUTPUT);
        digitalWrite(PIO_BREAK_TRIGGER, false);
    }

    hoermannDump.begin();
}

void Hoermann::gpioHandler() const
{
    LPC_GPIO_TypeDef* port = gpioPorts[pinRxPort_];
    const uint32_t mask = digitalPinToBitMask(pinRx_);

    if (!(port->MIS & mask))
    {
        return; // Not our pin
    }

    // Clear and disable the GPIO interrupt for this pin
    port->IC  = mask;     // Clear the interrupt
    port->IE &= ~mask;    // Disable interrupt for this pin

    // Switch pin back to UART Rx function.
    // The physical pin is now high (marking state), so the UART receiver
    // will immediately see marking and exit its post-break idle state.
    pinMode(pinRx_, rxDefaultPinMode);
    digitalWrite(PIO_BREAK_TRIGGER, false);
    digitalWrite(PIO_LED_1, false);
}

void Hoermann::onBreakIndication() const
{
    // Switch RX pin from UART RXD function to GPIO INPUT with pull-up.
    // The UART receiver is now disconnected from the physical pin and stays idle.
    // When the sender releases the break, RXD goes high (via pull-up or sender driving high),
    // triggering the rising edge GPIO interrupt.
    pinMode(pinRx_, rxOnBreakPinMode);

    // Clear the Rx FIFO to discard any garbage from the break condition
    serial.resetUartRxFifo();

    // Configure rising edge interrupt on the RX pin
    LPC_GPIO_TypeDef* port = gpioPorts[pinRxPort_];
    const uint32_t mask = digitalPinToBitMask(pinRx_);
    port->IS  &= ~mask;  // Edge-sensitive (not level)
    port->IBE &= ~mask;  // Single edge (not both)
    port->IEV |=  mask;  // Rising edge
    port->IC   =  mask;  // Clear any pending interrupt for this pin
    port->IE  |=  mask;  // Enable interrupt for this pin

    // Enable the interrupt for the corresponding GPIO port
    const auto gpioIrq = digitalPinToIRQn(pinRx_);
    clearPendingInterrupt(gpioIrq);
    enableInterrupt(gpioIrq);
}

__attribute__((optimize("O3"))) void Hoermann::onSerialError(uint8_t errorFlags, const uint8_t faultyByte)
{
    isrOnErrorCount++;

    // Callback function runs in interrupt context, keep it short
    if (errorFlags & SerialError::SERIAL_BREAK_INDICATION)
    {
        // A break indicates start of a new transmission
        isrBreakIndicatorCount++;
        // break always comes with a framing error, so we clear it here
        errorFlags &= ~SerialError::SERIAL_FRAME_ERROR;
        errorFlags &= ~SerialError::SERIAL_BREAK_INDICATION;

        digitalWrite(PIO_BREAK_TRIGGER, true);
        digitalWrite(PIO_LED_1, true);
        onBreakIndication();
        isrBreakDetected = true;
        if (faultyByte != 0)
        {
            fatalError();
        }
        //hoermannDump.print("(BI) ");
    }

    if (errorFlags & SerialError::SERIAL_FRAME_ERROR)
    {
        isrFrameErrorCount++;
        hoermannDump.print("(FE) ");
    }

    if (errorFlags & SerialError::SERIAL_PARITY_ERROR)
    {
        isrParityErrorCount++;
        hoermannDump.print("(PE) ");
    }


    if (errorFlags & SerialError::SERIAL_OVERRUN_ERROR)
    {
        isrOverrunErrorCount++;
        hoermannDump.print("(OE) ");
    }


    if (errorFlags)
    {
        hoermannDump.print(errorFlags, HEX, 2);
        hoermannDump.println();
    }
}

void Hoermann::decodeBroadcast(const uint8_t * data, const uint8_t dataLength)
{
    HoermannState oldState;
    state.copyTo(oldState);

    if (dataLength >= 1)
    {
        const uint8_t status_0 = data[0];
        state.doorOpen = status_0 & (1 << 0);
        state.doorClosed = status_0 & (1 << 1);
        state.optionRelay = status_0 & (1 << 2);
        state.lightRelay = status_0 & (1 << 3);
        state.error = status_0 & (1 << 4);
        state.directionDown = status_0 & (1 << 5);
        state.moving = status_0 & (1 << 6);
        state.ventingPos = status_0 & (1 << 7);
    }

    if (dataLength == 2)
    {
        state.preWarning = data[1] & (1 << 0);
    }

    if(!state.equals(oldState))
    {
        hoermannDump.dumpRx(true, crc.current, serialBuffer, bufferPosition + 1);
    }
    hoermannDump.dumpRx(true, crc.current, serialBuffer, bufferPosition  + 1);
}

void Hoermann::decodeDeviceCommand(const uint8_t * data, const uint8_t dataLength)
{
    hoermannDump.dumpRx(true, crc.current, serialBuffer, bufferPosition + 1);
    if (dataLength < 2)
    {
        fatalError();
    }

    const auto command = static_cast<DeviceCommand>(data[0]);
    const auto senderAddress = static_cast<BusAddress>(data[1]);
    switch (command)
    {
        case DeviceCommand::scan:
        {
            uint8_t buf[2] = { uap1Device, uap1Address };
            sendResponse(senderAddress, buf, 2);
            break;
        }

        case DeviceCommand::statusRequest:
        {
            uint8_t buf[] = { static_cast<uint8_t>(DeviceCommand::statusResponse),
                    deviceResponse[0], deviceResponse[1] };
            sendResponse(senderAddress, buf, 3);
            deviceResponse[0] = 0;
            break;
        }

        default:
            fatalError();
            break;
    }
}

void Hoermann::processFrame(const uint8_t * frame, const uint8_t frameLength)
{
    if (frameLength < MIN_FRAME_LENGTH)
    {
        fatalError();
    }

    const uint8_t address = frame[ADDRESS_OFFSET];
    //const uint8_t counter = frame[COUNT_LENGTH_OFFSET] >> 4; // high nibble
    const uint8_t length = frame[COUNT_LENGTH_OFFSET] & 0x0f; // low nibble

    const uint8_t * data = frame + DATA_OFFSET;
    switch (address)
    {
        case broadCastAddress:
            decodeBroadcast(data, length);
            break;

        case uap1Address:
            decodeDeviceCommand(data, length);
            break;

       default:
           // frame is not for us and no broadcast
           break;
    }
}

void Hoermann::loop()
{
    if (isrBreakDetected)
    {
        // Start of a new transmission/frame
        isrBreakDetected = false;
        resetSerialBuffer();
        serial.clearRxBuffer();
    }

    if ((serialRxTimeout.expired() || serialRxTimeout.stopped()) &&
        bufferPosition > 0)
    {
        const bool crcOK = crcOKofSerialBuffer();
        if (crcOK)
        {
            processFrame(serialBuffer, bufferPosition);
        }
        else
        {
            hoermannDump.dumpRx(crcOK, crc.current, serialBuffer, bufferPosition + 1);
        }
        resetSerialBuffer();
    }

    int32_t serialByte = serial.read();
    // uint32_t readCount = 0;
    while(serialByte > -1)
    {
        // readCount++;
        putSerialBuffer(serialByte);
        serialRxTimeout.start(SERIAL_RX_TIMEOUT_MS);
        serialByte = serial.read();
    }
}

void Hoermann::sendResponse(const uint8_t addr, uint8_t bytes[], const uint8_t len)
{
    digitalWrite(PIO_LED_2, !digitalRead(PIO_LED_2));
    const auto counterAndLength = static_cast<uint8_t>((myFrameCounter++ & 0x0F) << 4 | len);

    CRC innerCRC;
    innerCRC.reset();
    innerCRC.update(addr);
    innerCRC.update(counterAndLength);
    for (int i = 0; i < len; i++)
    {
        innerCRC.update(bytes[i]);
    }
    //delayMicroseconds(breakReleaseMicroseconds);
    serial.sendBreak(breakConditionMicroseconds);
    delayMicroseconds(breakReleaseMicroseconds);
    ///\todo enable real serial write on rs485
//    serial.write(addr);
//    serial.write(temp);
//
//    serial.write(bytes, len);
//    serial.write(innerCRC.current);
    hoermannDump.dumpTx(addr, counterAndLength, innerCRC.current, bytes, len);
}

void Hoermann::open()
{
    if (!state.doorOpen && (!state.moving || state.directionDown))
    {
         deviceResponse[0] = (deviceResponse[0] & 0xFFE8) | 0x01;
    }
}

void Hoermann::close()
{
    if (!state.doorClosed && (!state.moving || !state.directionDown))
    {
         deviceResponse[0] = (deviceResponse[0] & 0xFFE8) | 0x02;
    }
}

void Hoermann::stop()
{
    if (state.moving)
    {
        deviceResponse[0] = (deviceResponse[0] & 0xFFE8) | 0x04;
    }
}

void Hoermann::venting()
{
    deviceResponse[0] = (deviceResponse[0] & 0xFFE8) | 0x10;
}

void Hoermann::light(const bool on)
{
    if (on != state.lightRelay)
    {
        deviceResponse[0] |= 0x08;
    }
}

void Hoermann::emergencyStop(const bool on)
{
    if (on)
    {
        deviceResponse[1] = 0x00;
    }
    else
    {
        deviceResponse[1] = 0x10;
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

bool Hoermann::crcOKofSerialBuffer()
{
    if (bufferPosition < MIN_FRAME_LENGTH - 1)
    {
        // We need at least 4 bytes for a valid frame (address, counter/length, command/data, crc)
        return false;
    }

    crc.reset();
    for (uint16_t i = 0; i < bufferPosition; i++) // Exclude last byte, which is the crc
    {
        crc.update(serialBuffer[i]);
    }
    return crc.matches(serialBuffer[bufferPosition]);
}
