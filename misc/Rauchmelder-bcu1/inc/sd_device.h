/*  Original written for LPC922:
 *  Copyright (c) 2015-2017 Stefan Haller
 *  Copyright (c) 2013 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  Modified for LPC1115 ARM processor:
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef SMOKE_DETECTOR_DEVICE_H_
#define SMOKE_DETECTOR_DEVICE_H_

#include <stdint.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>

#include "sd_com.h"
#include "sd_shared_enums.h"

class SmokeDetectorAlarm;
class SmokeDetectorConfig;
class SmokeDetectorErrorCode;
class SmokeDetectorGroupObjects;

/**
 * Possible commands to send to the smoke detector
 */
enum class DeviceCommand : uint8_t
{
    serialNumber          = 0,  //!< Gira Command: Read serial number
    operatingTime         = 1,  //!< Gira Command: Query operating time
    smokeboxData          = 2,  //!< Gira Command: Query smokebox data
    batteryAndTemperature = 3,  //!< Gira Command: Query battery voltage and temperatures
    numberAlarms1         = 4,  //!< Gira Command: Query number of alarms #1
    numberAlarms2         = 5,  //!< Gira Command: Query number of alarms #2
    status                = 6,  //!< Gira Command: Query status
};

typedef EnumIterator<DeviceCommand, DeviceCommand::serialNumber, DeviceCommand::status> AllDeviceCommands;

class SmokeDetectorDevice : private SmokeDetectorComCallback
{
public:
    SmokeDetectorDevice(const SmokeDetectorConfig *config, const SmokeDetectorGroupObjects *groupObjects, SmokeDetectorAlarm *alarm, SmokeDetectorErrorCode *errorCode);
    ~SmokeDetectorDevice() = delete;

    void setAlarmState(RmAlarmState newState);
    bool sendCommand(DeviceCommand cmd);
    void loopReceiveBytes();
    void loopCheckState();
    bool isReady() const;

private:
    void receivedMessage(uint8_t *bytes, uint8_t len);
    void timedOut(std::optional<RmCommandByte> command);

private:
    void setSupplyVoltage(bool enable);
    void checkAttachedToBasePlate();

    void readSerialNumberMessage(const uint8_t *bytes) const;
    void readOperatingTimeMessage(const uint8_t *bytes) const;
    void readSmokeboxDataMessage(const uint8_t *bytes) const;
    void readBatteryAndTemperatureMessage(const uint8_t *bytes) const;
    void readNumberAlarms1Message(const uint8_t *bytes) const;
    void readNumberAlarms2Message(const uint8_t *bytes) const;
    void readStatusMessage(const uint8_t *bytes) const;
    uint32_t readOperatingTime(const uint8_t *bytes) const;
    int32_t readSmokeBoxValue(const uint8_t *bytes) const;
    int32_t readVoltage(const uint8_t *bytes) const;
    int32_t readTemperature(const uint8_t *bytes) const;

    static void failHardInDebug();
    static RmCommandByte deviceCommandToRmCommandByte(DeviceCommand command);
    static uint32_t readUInt32(const uint8_t *bytes);
    static uint16_t readUInt16(const uint8_t *bytes);

private:
    enum class DeviceState
    {
        initialized,
        drainCapacitor,
        attachToBasePlate,
        powerUpDevice,
        fillCapacitor,
        ready
    };

    template < volatile int32_t templatePin, bool templateOn, volatile int32_t templatePinLed >
    class DeviceIOPin
    {
    public:
        int32_t pin() const { return templatePin; }
        bool on() const { return templateOn; }
        bool off() const { return !templateOn; }
        int32_t pinLed() const { return templatePinLed; }
    };

    //!< Time in milliseconds the 12V supply needs to drain the capacitor
    static constexpr uint32_t SupplyVoltageOffDelayMs = 500;

    //!< Time in milliseconds the 12V supply needs to fill the capacitor
    static constexpr uint32_t SupplyVoltageOnDelayMs = 5000;

    //!< Maximum waiting time in milliseconds to enable 12V supply
    static constexpr uint32_t SupplyVoltageTimeoutMs = 120000;

    //!< Time in milliseconds we give the smoke detector to startup and measure the battery voltage
    static constexpr uint32_t DevicePowerUpDelayMs = 1000;

    //!< Sentinel value representing an invalid battery voltage
    static constexpr int32_t BatteryVoltageInvalid = -1;

    //!< set low to enable smoke detector's serial communication feature
    static constexpr DeviceIOPin<PIO3_5, false, 0> CommunicationEnable {};

    //!< 12V support voltage for the smoke detector, NPN transistor pulls down the voltage, LED is on while supply voltage is "disconnected"
    static constexpr DeviceIOPin<PIO2_1, false, PIO3_3> SupportVoltage {};

    //!< IO pin where the internal smoke detector voltage is connected, LED is on while base plate is not attached
    static constexpr DeviceIOPin<PIO0_11, true, PIO2_6> AttachedToBasePlate {};

private:
    const SmokeDetectorConfig *config;
    const SmokeDetectorGroupObjects *groupObjects;
    SmokeDetectorAlarm *alarm;
    SmokeDetectorErrorCode *errorCode;
    SmokeDetectorCom *com;
    DeviceState state;
    Timeout timeout;
};

#endif /* SMOKE_DETECTOR_DEVICE_H_ */
