/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef HOERMANN_H_
#define HOERMANN_H_

#include "CRC.h"
#include "HoermannState.h"
#include <sblib/timeout.h>
#include <sblib/digital_pin.h>

extern "C" void PIOINT1_IRQHandler();
extern "C" void PIOINT2_IRQHandler();
extern "C" void PIOINT3_IRQHandler();

class Hoermann {
public:
    Hoermann() = default;

    /**
     * Initializes the serial interface for RS485 communication with the Hoermann drive.
     *
     * Configures the UART peripheral at 19200 baud (8N1), assigns the TX and RX pins,
     * sets up the RTS pin for hardware RS485 driver enable (active-low DE), and enables
     * RS485 direction control via the UART hardware.
     *
     * @param pinTx  Pin to use as UART transmit (TX).
     * @param pinRx  Pin to use as UART receive (RX), configured with pull-up and hysteresis.
     * @param pinRTS Pin to use as RS485 direction control (RTS / DE), driven low during transmission.
     */
    void begin(uint32_t pinTx, uint32_t pinRx, uint32_t pinRTS);

    void loop();
    void open();
    void close();
    void stop();
    void venting();
    void light(bool on);
    void emergencyStop(bool on);

    HoermannState state;

protected:
    uint8_t deviceResponse[2] = { 0x00, 0x10 };
    uint8_t myFrameCounter = 0;
    CRC crc;

    void sendResponse(uint8_t addr, uint8_t bytes[], uint8_t len);

    void decodeBroadcast(const uint8_t * data, uint8_t dataLength);
    void decodeDeviceCommand(const uint8_t * data, uint8_t dataLength);

    friend void PIOINT1_IRQHandler(); // For possible Rx Pin at PIO1_6
    friend void PIOINT2_IRQHandler(); // For possible Rx Pin at PIO2_7
    friend void PIOINT3_IRQHandler(); // For possible Rx Pin at PIO3_1

private:
    constexpr static uint32_t rxDefaultPinMode = SERIAL_RXD | INPUT | PULL_UP | HYSTERESIS;
    constexpr static uint32_t rxOnBreakPinMode = INPUT | PULL_UP | HYSTERESIS;

    // Broadcast 4 = 1 + 1 + 1 + 1 (address + counter/length + data + crc)
    constexpr static uint8_t MIN_FRAME_LENGTH = 3;
    // 18 = 1 + 1 + 15 + 1 (address + counter/length + data + crc)
    constexpr static uint8_t MAX_FRAME_LENGTH = 18;
    constexpr static uint16_t SERIAL_RX_TIMEOUT_MS = 4;
    volatile bool isrBreakDetected = false;

    volatile uint32_t isrBreakIndicatorCount = 0;
    volatile uint32_t isrFrameErrorCount = 0;
    volatile uint32_t isrParityErrorCount = 0;
    volatile uint32_t isrOverrunErrorCount = 0;
    volatile uint32_t isrOnErrorCount = 0;

    uint8_t serialBuffer[MAX_FRAME_LENGTH] = {};
    int16_t bufferPosition = -1;

    Timeout serialRxTimeout;
    uint32_t pinRx_ = 0;
    Port pinRxPort_ = PIO1;

    void onSerialError(uint8_t errorFlags, uint8_t faultyByte);

    /**
     * @brief Start the GPIO-based break recovery workaround.
     *
     * Switches the RX pin from UART function to GPIO INPUT with pull-up and
     * rising edge interrupt. When RXD goes high (sender releases break),
     * the GPIO interrupt fires and breakRecoveryGpioHandler() restores UART mode.
     */
    void onBreakIndication() const;

    /**
     * @brief Handle the GPIO interrupt for break recovery.
     *
     * After a break indication, the UART receiver goes idle until RXD returns to marking state.
     * To work around this, the RX pin is temporarily switched to GPIO mode with a pull-up and
     * rising edge interrupt. When the sender releases the break (RXD goes high), the GPIO ISR
     * fires and this method must be called to restore the RX pin to UART function.
     *
     * @return true if this pin's interrupt was handled, false if the interrupt was not from the RX pin.
     *
     * @note The user must provide the corresponding PIOINTx_IRQHandler for the RX pin's port.
     *       For the default RX pin PIO1_6 (port 1), add:
     * @code
     *      extern "C" void PIOINT1_IRQHandler() {
     *          hoermann.onBreak();
     *      }
     * @endcode
     * @warning Must only be called from the GPIO interrupt context.
     */
    void gpioHandler() const;

    void resetSerialBuffer();
    void putSerialBuffer(uint8_t toPut);
    [[nodiscard]] uint8_t getSerialBufferSize() const;
    bool crcOKofSerialBuffer();
    void processFrame(const uint8_t * frame, uint8_t frameLength);
};

#endif /* HOERMANN_H_ */

