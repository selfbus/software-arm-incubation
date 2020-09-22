/*
 * rc_protocol.h
 *
 *  Created on: 11.09.2020
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

/*
ESPA Tacomat Comfort serial-protocol:
Baudrate settings: 2400 8N1 (2400 Baud, 8 Databits, No Parity, 1 Stopbit)

so far known commands (single char):
Command         Description
p               Parameter request
s               Parameter Set (raincenter acknowledges command with 's', then it waits for 11 configuration bytes
w               Display data request
c               Switch Display (m³, cm, percent)
a               Switch to tap water refill
b               Switch to reservoir

-------------------------------------------------------------------------------
p Parameter request Answer:
Example in hex
Byte#:     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16
Data :     70 30 07 01 03 01 42 00 31 01 30 06 51 51 97 00 00

byte numbering according to PROFIRAIN PCA documentation
#byte   Description                             Multiplier  Unit    Code
 0.     0x70 = 'p'
 1.     water exchange period                       1       days    BCD
 2.     tap water switch on height                  5       cm      BCD
 3.     tap water switch on hysteresis              2       cm      BCD
 4.     water exchange duration                     1       min     BCD
 5.     tap water supply type                       -       0-2     BCD
 6.     maximum fill level                          5       cm      BCD
 7.     reservoir type                              -       0-1     BCD
 8.     reservoir area                              0.1     m2      BCD
 9.     optional relay function                     -       0-9     BCD
10.     automatic timer interval                    1       days    BCD
11.     automatic timer duration                    10      s       BCD
12.     fill level factory calibration              1       -       BCD
13.     fill level user calibration                 1       -       BCD
14.     fill level measured (raw data)              1       cm      BCD
15.     fill level measured (raw data)              100     cm      BCD
16.     0x00 (end of message ?)

-------------------------------------------------------------------------------
w Display data request Answer:
Example in hex
Byte#:   00 01 02 03 04
Data:    77 95 00 08 02

byte numbering according to PROFIRAIN PCA documentation
#byte.bit   Description                         Multiplier  Unit    Code
0.          0x77 = 'w'
1.          display Value (low byte)                100     -       BCD
2.          display Value (high byte)               1       -       BCD

3.0         OptionalLEDAlwaysOn
3.1         OptionalLEDBlinking ?? Alarm 1 ??
3.2         automatically switched to tap water refill
3.3         manually switched to tap water refill
3.4         water exchange active
3.5         optional relais always on
3.6         optional relais blinking
3.7         buzzer active ?? in PROFIRAIN PCA documentation refered to bit 3.6 which should be for optional relais

4.0         optional input active
4.1         tap water refill input active
4.2         pump running
4.3         unused / this bit "flickers/blinks"
4.4         on button pressed
4.5         off button pressed
4.6         display value is in percent
4.7         display value is in qubic meters

*/

#ifndef RC_PROTOCOL_H_
#define RC_PROTOCOL_H_

#include <sblib/types.h>

enum eTapWaterSupplyType {none, automatic_filllevel, external_conductance_sensor};
enum eReservoirType {cylindrical, spherical};
enum eOptionalRelaisFunction{no_function,
                             backwater_alarm,
                             leakage_warn_alarm,
                             reverse_leakage_warn_alarm,
                             parallel_pump,
                             filter_back_flushing,
                             max_level_alarm,
                             min_level_alarm,
                             reservoir_empty_alarm,
                             reservoir_filling};
enum eDisplayUnit {invalid, cm, percent, m3};

const static char RC_INVALID_COMMAND = (char)0;
const static byte RC_LEVEL_CALIBRATION_FACTOR = 50;
const static int RC_INVALID_MESSAGE_LENGTH = 30;

class RCMessage
{
public:
    RCMessage();
    virtual ~RCMessage();
    virtual bool Decode(byte * msg, int msg_len) = 0;
    bool IsValid() const {return _IsValid;}
protected:
    const static char msgIdentifier = RC_INVALID_COMMAND;
    const static byte msgLength;
    bool _IsValid;
};

class RCParameterMessage : public RCMessage
{
// Example in hex
// #Byte#:     01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
// #Data :     70 30 07 01 03 01 42 00 31 01 30 06 51 51 97 00 00
public:
    static const char msgIdentifier = 'p'; // 01. byte
    static const byte msgLength = 17;
    static constexpr byte msgExample[msgLength] = {0x70, 0x30, 0x07, 0x01, 0x03, 0x01, 0x42, 0x00, 0x31, 0x01, 0x30, 0x06, 0x51, 0x51, 0x97, 0x00, 0x00};

    RCParameterMessage();
    bool Decode(byte * msg, int msg_len);
    void operator=(const RCParameterMessage &msg);
    bool operator==(const RCParameterMessage &msg);
    bool operator!=(const RCParameterMessage &msg);
    int WaterExchangePeriod_days() const {return _WaterExchangePeriod_days;}
    int TapWaterSwitchOnHeight_cm() const {return _TapWaterSwitchOnHeight_cm;}
    int TapWaterSwitchOnHysteresis_cm() const {return _TapWaterSwitchOnHysteresis_cm;}
    int WaterExchangeDuration_min() const {return _WaterExchangeDuration_min;}
    eTapWaterSupplyType TapWaterSupplyType() const {return _TapWaterSupplyType;}
    int FillingLevelMax_cm() const {return _FillingLevelMax_cm;}
    eReservoirType ReservoirType() const {return _ReservoirType;}
    float ReservoirArea_m2() const {return _ReservoirArea_m2;}
    eOptionalRelaisFunction OptionalRelaisFnuction() const {return _OptionalRelaisFunction;}
    int FilterBackFlushingAutomaticTimerInterval_days() const {return _FilterBackFlushingAutomaticTimerInterval_days;}
    int FilterBackFlushingAutomaticTimerDuration_seconds() const {return _FilterBackFlushingAutomaticTimerDuration_seconds;}
    int LevelCalibrationFactory() const {return _LevelCalibrationFactory;}
    int LevelCalibrationUser() const {return _LevelCalibrationUser;}
    int LevelMeasuredcm() const {return _LevelMeasured_cm;}
    int LevelCalibratedcm() const {return _LevelCalibrated_cm;}
    float Level_m3_Calibrated() const {return _Level_m3_Calibrated;}
protected:
    int _WaterExchangePeriod_days; //02. byte how many days
    int _TapWaterSwitchOnHeight_cm;
    int _TapWaterSwitchOnHysteresis_cm;
    int _WaterExchangeDuration_min;
    eTapWaterSupplyType _TapWaterSupplyType;
    int _FillingLevelMax_cm;
    eReservoirType _ReservoirType;
    float _ReservoirArea_m2;
    eOptionalRelaisFunction _OptionalRelaisFunction;
    int _FilterBackFlushingAutomaticTimerInterval_days;
    int _FilterBackFlushingAutomaticTimerDuration_seconds;
    int _LevelCalibrationFactory;
    int _LevelCalibrationUser;
    int _LevelMeasured_cm;
    int _LevelCalibrated_cm;
    float _Level_m3_Calibrated;
};

class RCDisplayMessage : public RCMessage
{
// Example in hex
// #Byte#:   00 01 02 03 04
// #Data:    77 95 00 08 02
public:
    static const char msgIdentifier = 'w'; // 01. byte
    static const byte msgLength = 5;
    static constexpr byte msgExample[msgLength] = {0x77, 0x95, 0x00, 0x08, 0x02};

    RCDisplayMessage();
    bool Decode(byte * msg, int msg_len);
    void operator=(const RCDisplayMessage &msg);
    bool operator==(const RCDisplayMessage &msg);
    bool operator!=(const RCDisplayMessage &msg);

    float DisplayValue() const {return _DisplayValue;}
    eDisplayUnit DisplayUnit() const {return _DisplayUnit;}
    bool IsSwitchedToTapWater() {return (ManualSwitchedToTapWater() || AutomaticallySwitchedToTapWater() || WaterExchangeActive());}

    // byte 3 decodings
    bool OptionalLEDAlwaysOn() const {return _OptionalLEDAlwaysOn;} // byte 3.0
    bool OptionalLEDBlinking() const {return _OptionalLEDBlinking;} // byte 3.1
    bool AutomaticallySwitchedToTapWater() const {return _AutomaticallySwitchedToTapWater;}  // byte 3.2
    bool ManualSwitchedToTapWater() const {return _ManualSwitchedToTapWater;}  // byte 3.3
    bool WaterExchangeActive() const {return _WaterExchangeActive;} // byte 3.4
    bool OptionalRelaisAlwaysOn() const {return _OptionalRelaisAlwaysOn;} // byte 3.5
    bool OptionalRelaisBlinking() const {return _OptionalRelaisBlinking;} // byte 3.6
    bool AlarmBuzzerActive() const {return _AlarmBuzzerActive;} // byte 3.7

    // byte 4 decodings
    bool OptionalInputActive() const {return _OptionalInputActive;}
    bool TapWaterRefillInputActive() const {return _TapWaterRefillInputActive;}
    bool PumpActive() const {return _PumpActive;}
    bool unused() const {return _unused;}
    bool OnButtonPressed() const {return _OnButtonPressed;}
    bool OffButtonPressed() const {return _OffButtonPressed;}
    bool DisplayInPercent() const {return _DisplayInPercent;}
    bool DisplayInQubicMeters() const {return _DisplayInQubicMeters;}

protected:
    float _DisplayValue;
    eDisplayUnit _DisplayUnit;
    bool _AlarmBuzzerActive;
    bool _OptionalRelaisBlinking;
    bool _OptionalRelaisAlwaysOn;
    bool _WaterExchangeActive;
    bool _ManualSwitchedToTapWater;
    bool _AutomaticallySwitchedToTapWater;
    bool _OptionalLEDBlinking;
    bool _OptionalLEDAlwaysOn;

    bool _DisplayInQubicMeters;
    bool _DisplayInPercent;
    bool _OffButtonPressed;
    bool _OnButtonPressed;
    bool _unused;
    bool _PumpActive;
    bool _TapWaterRefillInputActive;
    bool _OptionalInputActive;

};

class RCSwitchDisplayMessage : public RCMessage
{
public:
    static const char msgIdentifier = 'c';
    static const byte msgLength = 1;
    static constexpr byte msgExample[msgLength] = {msgIdentifier};
    RCSwitchDisplayMessage();
    bool Decode(byte * msg, int msg_len);
protected:
};

class RCSwitchToTapWaterRefillMessage : public RCMessage
{
public:
    static const char msgIdentifier = 'a';
    static const byte msgLength = 1;
    static constexpr byte msgExample[msgLength] = {msgIdentifier};
    RCSwitchToTapWaterRefillMessage();
    bool Decode(byte * msg, int msg_len);
protected:
};

class RCSwitchToReservoirMessage : public RCMessage
{
public:
    static const char msgIdentifier = 'b';
    static const byte msgLength = 1;
    static constexpr byte msgExample[msgLength] = {msgIdentifier};
    RCSwitchToReservoirMessage();
    bool Decode(byte * msg, int msg_len);
protected:
};

#endif /* RC_PROTOCOL_H_ */
