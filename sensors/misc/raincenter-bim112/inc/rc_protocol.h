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

so far known commands (single character):
Command         Description
p               Parameter request
w               Display data request
c               Switch Display (m³, cm, percent)
a               Switch to tap water refill
b               Switch to reseroir

-------------------------------------------------------------------------------
p Parameter request Answer:
Example in hex
#Byte#:     01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
#Data :     70 30 07 01 03 01 42 00 31 01 30 06 51 51 97 00 00

#byte   Description                             Multiplier  Unit    Code
 1.     0x70 = 'p'
 2.     water exchange period                       1       days    BCD
 3.     tap water switch on height                  5       cm      BCD
 4.     tap water switch on hysteresis              2       cm      BCD
 5.     water exchange duration                     1       min     BCD
 6.     tap water supply type                       -       0-2     BCD
 7.     maximum fill level                          5       cm      BCD
 8.     reservoir type                              -       0-1     BCD
 9.     reservoir area                              0.1     m2      BCD
10.     optional relais function                    -       0-9     BCD
11.     automatic timer interval                    1       days    BCD
12.     automatic timer duration                    10      s       BCD
13.     fill level factory calibration              1       -       BCD
14.     fill level user calibration                 1       -       BCD
15.     fill level measured (raw data)              1       cm      BCD
16.     fill level measured (raw data)              100     cm      BCD
17.     0x00 (end of message ?)

-------------------------------------------------------------------------------
w Display data request Answer:
Example in hex
// Example in hex
// #Byte#:   01 02 03 04 05
// #Data:    77 95 00 08 02

#byte.bit   Description                         Multiplier  Unit    Code
1.          0x77 = 'w'
2.          display Value                           100     -       BCD
3.          display Value                           1       -       BCD
4.0         unknown
4.1         alarm
4.2         unknown
4.3         manually switched to tap water refill
4.4         unknown
4.5         unknown
4.6         unknown
4.7         unknown
5.0         unknown
5.1         unknown
5.2         unknown
5.3         unknown / this bit "flickers"
5.4         unknown
5.5         unknown
5.6         display value is in percent
5.7         display value is in qubic meters
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
                             back_flushing,
                             max_level_alarm,
                             min_level_alarm,
                             reservoir_empty_alarm,
                             reservoir_filling};
enum eDisplayUnit {invalid, cm, percent, m3};


class RCMessage
{
public:
    virtual bool Decode(byte * msg, int msg_len) = 0;
protected:
    const static char msgIdentifier;
    const static byte msgLength;
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
    int WaterExchangePeriod_days() const {return _WaterExchangePeriod_days;}
    int TapWaterSwitchOnHeight_cm() const {return _TapWaterSwitchOnHeight_cm;}
    int TapWaterSwitchOnHysteresis_cm() const {return _TapWaterSwitchOnHysteresis_cm;}
    int WaterExchangeDuration_min() const {return _WaterExchangeDuration_min;}
    eTapWaterSupplyType TapWaterSupplyType() const {return _TapWaterSupplyType;}
    int FillingLevelMax_cm() const {return _FillingLevelMax_cm;}
    eReservoirType ReservoirType() const {return _ReservoirType;}
    float ReservoirArea_m2() const {return _ReservoirArea_m2;}
    eOptionalRelaisFunction OptionalRelaisFnuction() const {return _OptionalRelaisFunction;}
    int AutomaticTimerInterval_days() const {return _AutomaticTimerInterval_days;}
    int AutomaticTimerDuration_seconds() const {return _AutomaticTimerDuration_seconds;}
    int LevelCalibrationFactory() const {return _LevelCalibrationFactory;}
    int LevelCalibrationUser() const {return _LevelCalibrationUser;}
    int LevelMeasured() const {return _LevelMeasured;}
    int LevelCalibrated() const {return _LevelCalibrated;}
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
    int _AutomaticTimerInterval_days;
    int _AutomaticTimerDuration_seconds;
    int _LevelCalibrationFactory;
    int _LevelCalibrationUser;
    int _LevelMeasured;
    int _LevelCalibrated;
};

class RCDisplayMessage : public RCMessage
{
// Example in hex
// #Byte#:   01 02 03 04 05
// #Data:    77 95 00 08 02
public:
    static const char msgIdentifier = 'w'; // 01. byte
    static const byte msgLength = 5;
    static constexpr byte msgExample[msgLength] = {0x77, 0x95, 0x00, 0x08, 0x02};
    RCDisplayMessage();
    bool Decode(byte * msg, int msg_len);
    float DisplayValue() const {return _DisplayValue;}
    eDisplayUnit DisplayUnit() const {return _DisplayUnit;}
    bool byte4_7() const {return _byte4_7;}
    bool byte4_6() const {return _byte4_6;}
    bool byte4_5() const {return _byte4_5;}
    bool byte4_4() const {return _byte4_4;}
    bool ManualSwitchedToTapWater() const {return _ManualSwitchedToTapWater;}
    bool byte4_2() const {return _byte4_2;}
    bool Alarm() const {return _Alarm;}
    bool byte4_0() const {return _byte4_0;}

    bool QubicMeters() const {return _QubicMeters;}
    bool Percent() const {return _Percent;}
    bool byte5_5() const {return _byte5_5;}
    bool byte5_4() const {return _byte5_4;}
    bool byte5_3() const {return _byte5_3;}
    bool byte5_2() const {return _byte5_2;}
    bool byte5_1() const {return _byte5_1;}
    bool byte5_0() const {return _byte5_0;}
protected:
    float _DisplayValue;
    eDisplayUnit _DisplayUnit;
    bool _byte4_7;
    bool _byte4_6;
    bool _byte4_5;
    bool _byte4_4;
    bool _ManualSwitchedToTapWater;
    bool _byte4_2;
    bool _Alarm;
    bool _byte4_0;

    bool _QubicMeters;
    bool _Percent;
    bool _byte5_5;
    bool _byte5_4;
    bool _byte5_3;
    bool _byte5_2;
    bool _byte5_1;
    bool _byte5_0;

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
