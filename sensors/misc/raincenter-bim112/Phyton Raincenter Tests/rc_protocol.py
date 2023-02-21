#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# Raincenter protocol
# created: August 16, 2020

import time


def bcd2int(b):
    # print("bcd2int")
    # print(b)
    nibble_left = (b & 0xF0) >> 4
    nibble_right = (b & 0x0F)
    return nibble_left * 10 + nibble_right


def int2bcd(i):
    nibble_left = int((i / 10))
    nibble_right = int((i % 10))
    b = ((nibble_left << 4) + nibble_right)
    # print('int2bcd {0:02d} {1:02x}'.format(int(i), b))
    return b


def bit_is_set(x, n):
    return x & 2 ** n != 0


def convert_to_readable_byte_format(s):
    result = ''
    for x in s:
        result += ' {0:02x}'.format(x)
    return result


def get_msg_from_telegram(telegram_bytes):
    new_msg = None
    if (telegram_bytes[0] == ord(RCDisplayMessage.msgIdentifier)) and (
            len(telegram_bytes) >= RCDisplayMessage.msgLength):
        new_msg = RCDisplayMessage()
    elif (telegram_bytes[0] == ord(RCParameterMessage.msgIdentifier)) and (
            len(telegram_bytes) >= RCParameterMessage.msgLength):
        new_msg = RCParameterMessage()
    else:
        pass

    if new_msg:
        new_msg.telegram_bytes = telegram_bytes
        new_msg.telegram = convert_to_readable_byte_format(new_msg.telegram_bytes)
        return new_msg
    else:
        return None


class RCMessage(object):
    msgTeleExample = []
    msgName = "RCMessage"

    def __init__(self):
        self.telegram = ''
        self.telegram_bytes = []
        self.data = []
        self.msgPrintLongInfo = True
        self.msgTimeStamp = time.strftime("%Y-%m-%d ") + time.strftime("%H:%M:%S")

    def get_byte_numbering_text(self):
        byte_numbering_text = ''
        for num, bt in enumerate(self.telegram_bytes, start=1):
            byte_numbering_text += " {:02d}".format(num)
        return byte_numbering_text

    def get_long_to_information_text(self):
        return self.to_information_text(False, False)

    def decode(self):
        self.data = self.telegram_bytes  # extract the data bytes from the telegram

    def encode(self):
        self.telegram_bytes = self.data  # copy the data to the telegram bytes

    def to_information_text(self, print_bytes, long_output):
        information_text = '{0:s} - {1:20s}\n'.format(self.msgTimeStamp, self.msgName)
        if print_bytes:
            information_text += 'Byte#:    {0:s}\n'.format(self.get_byte_numbering_text())
            information_text += 'Daten:    {0:s}'.format(self.telegram)
        if self.msgPrintLongInfo and long_output:
            tmp = self.get_long_to_information_text()
            if (len(information_text) > 0) and (len(tmp) > 0):
                information_text += '\n'
            information_text += tmp
        return information_text


class RCParameterMessage(RCMessage):
    # p Parameter request Answer:
    # Example in hex
    # Byte#:     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16
    # Data :     70 30 07 01 03 01 42 00 31 01 30 06 51 51 97 00 00

    # byte numbering according to PROFIRAIN PCA documentation
    # #byte   Description                             Multiplier  Unit    Code
    #  0.     0x70 = 'p'
    #  1.     water exchange period                       1       days    BCD
    #  2.     tap water switch on height                  5       cm      BCD
    #  3.     tap water switch on hysteresis              2       cm      BCD
    #  4.     water exchange duration                     1       min     BCD
    #  5.     tap water supply type                       -       0-2     BCD
    #  6.     maximum fill level                          5       cm      BCD
    #  7.     reservoir type                              -       0-1     BCD
    #  8.     reservoir area                              0.1     m2      BCD
    #  9.     optional relay function                     -       0-9     BCD
    # 10.     automatic timer interval                    1       days    BCD
    # 11.     automatic timer duration                    10      s       BCD
    # 12.     fill level factory calibration              1       -       BCD
    # 13.     fill level user calibration                 1       -       BCD
    # 14.     fill level measured (raw data)              1       cm      BCD
    # 15.     fill level measured (raw data)              100     cm      BCD
    # 16.     0x00 (end of message ?)

    msgIdentifier = b'p'
    msgLength = 17
    msgName = "rcParameter"
    msgTeleExample = [0x70, 0x30, 0x07, 0x01, 0x03, 0x01, 0x42, 0x00, 0x31, 0x01, 0x30, 0x06, 0x51, 0x51, 0x52, 0x01,
                      0x00]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True
        self.WaterExchangePeriod_days = -1
        self.TapWaterSwitchOnHeight_cm = -1
        self.TapWaterSwitchOnHysteresis_cm = -1
        self.WaterExchangeDuration_min = -1
        self.TapWaterSupplyType = -1
        self.FillingLevelMax_cm = -1
        self.ReservoirType = -1
        self.ReservoirArea_m2 = -1
        self.OptionalRelaysFunction = -1
        self.AutomaticTimerInterval_days = -1
        self.AutomaticTimerDuration_seconds = -1
        self.LevelCalibrationFactory = -1
        self.LevelCalibrationUser = -1
        self.LevelMeasured = -1
        self.LevelCalibrated = -1

    def decode(self):
        RCMessage.decode(self)
        if (self.data[0] == ord(RCParameterMessage.msgIdentifier)) and (len(self.data) >= RCParameterMessage.msgLength):
            self.WaterExchangePeriod_days = bcd2int(self.data[1])
            self.TapWaterSwitchOnHeight_cm = 5 * bcd2int(self.data[2])
            self.TapWaterSwitchOnHysteresis_cm = 2 * bcd2int(self.data[3])
            self.WaterExchangeDuration_min = bcd2int(self.data[4])
            self.TapWaterSupplyType = bcd2int(self.data[5])
            self.FillingLevelMax_cm = 5 * bcd2int(self.data[6])
            self.ReservoirType = bcd2int(self.data[7])
            self.ReservoirArea_m2 = 0.1 * bcd2int(self.data[8])
            self.OptionalRelaysFunction = bcd2int(self.data[9])
            self.AutomaticTimerInterval_days = bcd2int(self.data[10])
            self.AutomaticTimerDuration_seconds = 10 * bcd2int(self.data[11])
            self.LevelCalibrationFactory = bcd2int(self.data[12])
            self.LevelCalibrationUser = bcd2int(self.data[13])
            self.LevelMeasured = bcd2int(self.data[14]) + 100 * bcd2int(self.data[15])
            self.LevelCalibrated = self.LevelCalibrationUser - 51 + self.LevelMeasured

    def encode(self):
        self.data = bytearray(RCParameterMessage.msgLength)
        self.data[0] = RCParameterMessage.msgIdentifier[0]
        self.data[1] = int2bcd(self.WaterExchangePeriod_days)
        self.data[2] = int2bcd(self.TapWaterSwitchOnHeight_cm / 5)
        self.data[3] = int2bcd(self.TapWaterSwitchOnHysteresis_cm / 2)
        self.data[4] = int2bcd(self.WaterExchangeDuration_min)
        self.data[5] = int2bcd(self.TapWaterSupplyType)
        self.data[6] = int2bcd(self.FillingLevelMax_cm / 5)
        self.data[7] = int2bcd(self.ReservoirType)
        self.data[8] = int2bcd(self.ReservoirArea_m2 * 10)
        self.data[9] = int2bcd(self.OptionalRelaysFunction)
        self.data[10] = int2bcd(self.AutomaticTimerInterval_days)
        self.data[11] = int2bcd(self.AutomaticTimerDuration_seconds / 10)
        self.data[12] = int2bcd(self.LevelCalibrationFactory)
        self.data[13] = int2bcd(self.LevelCalibrationUser)
        self.data[14] = int2bcd(self.LevelMeasured % 100)
        self.data[15] = int2bcd(self.LevelMeasured / 100)
        RCMessage.encode(self)

    def get_long_to_information_text(self):
        information_text = ' Wasseraustauschperiode:                 {0:0d} Tage\n' \
            .format(self.WaterExchangePeriod_days)
        information_text += ' Einschalthoehe Trinkwassernachspeisung: {0:0d} cm\n' \
            .format(self.TapWaterSwitchOnHeight_cm)
        information_text += ' Hysterese Trinkwassernachspeisung:      {0:0d} cm\n'.format(
            self.TapWaterSwitchOnHysteresis_cm)
        information_text += ' Dauer des Wasserwechselmodus:           {0:0d} Minuten\n'.format(
            self.WaterExchangeDuration_min)
        information_text += ' Trinkwassernachspeise-Typ:              {0:0d}\n'.format(self.TapWaterSupplyType)
        information_text += ' Maximaler Fuellstand:                   {0:0d} cm\n'.format(self.FillingLevelMax_cm)
        if self.ReservoirType == 0:
            information_text += 'Behaeltertyp:                           {0:0d} (Speicher mit konstantem ' \
                                'Querschnitt)\n'.format(self.ReservoirType)
        else:
            information_text += ' Behaeltertyp:                           {0:0d} (kugelfoermiger Speicher)\n'.format(
                self.ReservoirType)
        information_text += ' Behaelterquerschnitt:                   {0:3.1f} m^2\n'.format(self.ReservoirArea_m2)
        if self.OptionalRelaysFunction == 0:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Keine Funktion)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 1:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Rueckstaualarm)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 2:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Leckwarnalarm)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 3:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Reverse Leckwarn)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 4:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Parallelpumpe)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 5:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (Rueckspuel)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 6:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (LW + mit Anzeige)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 7:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (LW - mit Anzeige)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 8:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (LW + ohne Anzeige)\n'.format(
                self.OptionalRelaysFunction)
        elif self.OptionalRelaysFunction == 9:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (LW - ohne Anzeige)\n'.format(
                self.OptionalRelaysFunction)
        else:
            information_text += ' Funktion des Optionalrelais:            {0:0d} (unbekannt)\n'.format(
                self.OptionalRelaysFunction)

        information_text += ' Intervall der Zeitschaltautomatik:      {0:0d} Tage\n'.format(
            self.AutomaticTimerInterval_days)
        information_text += ' Dauer der Zeitschaltautomatik:          {0:0d} Sekunden\n'.format(
            self.AutomaticTimerDuration_seconds)
        information_text += ' Werks-Kalibrierung:                     {0:0d}\n'.format(self.LevelCalibrationFactory)
        information_text += ' Ebenezer-Kalibrierung:                  {0:0d}\n'.format(self.LevelCalibrationUser)
        information_text += ' Real-Messwert:                          {0:0d} cm\n'.format(self.LevelMeasured)
        information_text += ' Kalibrierter-Messwert:                  {0:0d} cm\n'.format(self.LevelCalibrated)
        return information_text


class RCDisplayMessage(RCMessage):
    # w Display data request Answer:
    # Example in hex
    # Byte#:   00 01 02 03 04
    # Data:    77 95 00 08 02

    # byte numbering according to PROFIRAIN PCA documentation
    # #byte.bit   Description                         Multiplier  Unit    Code
    # 0.          0x77 = 'w'
    # 1.          display Value (low byte)                100     -       BCD
    # 2.          display Value (high byte)               1       -       BCD

    # 3.0         OptionalLEDAlwaysOn
    # 3.1         OptionalLEDBlinking ?? Alarm 1 ??
    # 3.2         automatically switched to tap water refill
    # 3.3         manually switched to tap water refill
    # 3.4         water exchange active
    # 3.5         optional relays always on
    # 3.6         optional relays blinking
    # 3.7         buzzer active ?? in PROFIRAIN PCA documentation referred to bit 3.6 should be for optional relays

    # 4.0         optional input active
    # 4.1         tap water refill input active
    # 4.2         pump running
    # 4.3         unused / this bit "flickers/blinks"
    # 4.4         on button pressed
    # 4.5         off button pressed
    # 4.6         display value is in percent
    # 4.7         display value is in qubic meters

    msgIdentifier = b'w'
    msgLength = 5
    msgName = "rcDisplay"
    msgTeleExample = [0x77, 0x52, 0x01, 0x08, 0x02]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True
        self.DisplayValue = -1  # byte 1 & 2
        self.DisplayUnit = '-'

        self.OptionalLEDAlwaysOn = -1  # byte 3.0
        self.OptionalLEDBlinking = -1  # byte 3.1
        self.AutomaticallySwitchedToTapWater = -1  # byte 3.2
        self.ManualSwitchedToTapWater = -1  # byte 3.3
        self.WaterExchangeActive = -1  # byte 3.4
        self.OptionalRelaysAlwaysOn = -1  # byte 3.5
        self.OptionalRelaysBlinking = -1  # byte 3.6
        self.AlarmBuzzerActive = -1  # byte 3.7

        self.OptionalInputActive = -1  # byte 4.0
        self.TapWaterRefillInputActive = -1  # byte 4.1
        self.PumpActive = -1  # byte 4.2
        self.unused = -1  # byte 4.3
        self.OnButtonPressed = -1  # byte 4.4
        self.OffButtonPressed = -1  # byte 4.5
        self.DisplayInPercent = -1  # byte 4.6
        self.DisplayInQubicMeters = -1  # byte 4.7

    def decode(self):
        RCMessage.decode(self)
        if (self.data[0] == ord(RCDisplayMessage.msgIdentifier)) and (len(self.data) >= RCDisplayMessage.msgLength):
            self.DisplayValue = bcd2int(self.data[1]) + 100 * bcd2int(self.data[2])
            # byte 3 decoding
            self.OptionalLEDAlwaysOn = bit_is_set(self.data[3], 0)
            self.OptionalLEDBlinking = bit_is_set(self.data[3], 1)
            self.AutomaticallySwitchedToTapWater = bit_is_set(self.data[3], 2)
            self.ManualSwitchedToTapWater = bit_is_set(self.data[3], 3)
            self.WaterExchangeActive = bit_is_set(self.data[3], 4)
            self.OptionalRelaysAlwaysOn = bit_is_set(self.data[3], 5)
            self.OptionalRelaysBlinking = bit_is_set(self.data[3], 6)
            self.AlarmBuzzerActive = bit_is_set(self.data[3], 7)
            # byte 4 decoding
            self.OptionalInputActive = bit_is_set(self.data[4], 0)
            self.TapWaterRefillInputActive = bit_is_set(self.data[4], 1)
            self.PumpActive = bit_is_set(self.data[4], 2)
            self.unused = bit_is_set(self.data[4], 3)  # flickers
            self.OnButtonPressed = bit_is_set(self.data[4], 4)
            self.OffButtonPressed = bit_is_set(self.data[4], 5)
            self.DisplayInPercent = bit_is_set(self.data[4], 6)
            self.DisplayInQubicMeters = bit_is_set(self.data[4], 7)

            if self.DisplayInQubicMeters:
                self.DisplayUnit = 'm^3'
                self.DisplayValue = self.DisplayValue / 10
            elif self.DisplayInPercent:
                self.DisplayUnit = '%'
            else:
                self.DisplayUnit = 'cm'

    def encode(self):
        self.data = bytearray(RCDisplayMessage.msgLength)
        self.data[0] = RCDisplayMessage.msgIdentifier[0]
        self.data[1] = int2bcd(self.DisplayValue % 100)
        self.data[2] = int2bcd(self.DisplayValue / 100)

        self.data[3] = 0
        self.data[3] = (self.data[3] | (self.AlarmBuzzerActive & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.OptionalRelaysBlinking & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.OptionalRelaysAlwaysOn & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.WaterExchangeActive & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.ManualSwitchedToTapWater & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.AutomaticallySwitchedToTapWater & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.OptionalLEDBlinking & 0x01)) << 1
        self.data[3] = (self.data[3] | (self.OptionalLEDAlwaysOn & 0x01))

        self.data[4] = 0
        self.data[4] = (self.data[4] | (self.DisplayInQubicMeters & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.DisplayInPercent & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.OffButtonPressed & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.OnButtonPressed & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.unused & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.PumpActive & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.TapWaterRefillInputActive & 0x01)) << 1
        self.data[4] = (self.data[4] | (self.OptionalInputActive & 0x01))
        RCMessage.encode(self)

    def get_long_to_information_text(self):
        if self.DisplayUnit == 'm^3':
            information_text = ' Anzeige:                 {0:.1f} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)
        else:
            information_text = ' Anzeige:                 {0:d} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)

        information_text += ' opt. LED an:             {0:0d}\n'.format(self.OptionalLEDAlwaysOn)
        information_text += ' opt. LED blinkt:         {0:0d}\n'.format(self.OptionalLEDBlinking)
        information_text += ' autom. Nachspeisung:     {0:0d}\n'.format(self.AutomaticallySwitchedToTapWater)
        information_text += ' manuelle Nachspeisung:   {0:0d}\n'.format(self.ManualSwitchedToTapWater)
        information_text += ' Wasseraustausch aktiv:   {0:0d}\n'.format(self.WaterExchangeActive)
        information_text += ' opt. Relais an:          {0:0d}\n'.format(self.OptionalRelaysAlwaysOn)
        information_text += ' opt. Relais blinkt:      {0:0d}\n'.format(self.OptionalRelaysBlinking)
        information_text += ' Alarmbuzzer an:          {0:0d}\n'.format(self.AlarmBuzzerActive)

        information_text += ' opt. Eingang an:         {0:0d}\n'.format(self.OptionalInputActive)
        information_text += ' Nachspeise-Eingang an:   {0:0d}\n'.format(self.TapWaterRefillInputActive)
        information_text += ' Pumpe laeuft:            {0:0d}\n'.format(self.PumpActive)
        information_text += ' unbenutzt (flackert):    {0:0d}\n'.format(self.unused)
        information_text += ' ON Button gedrueckt:     {0:0d}\n'.format(self.OnButtonPressed)
        information_text += ' OFF Button gedrueckt:    {0:0d}\n'.format(self.OffButtonPressed)
        information_text += ' Anzeige ist in %:        {0:0d}\n'.format(self.DisplayInPercent)
        information_text += ' Anzeige ist in m^3:      {0:0d}\n'.format(self.DisplayInQubicMeters)
        # information_text += ' Byte 3:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n' \
        #    .format(self.byte4_7, self.byte4_6, self.Alarm1, self.byte4_4, self.ManualSwitchedToTapWater,
        #            self.AutomaticallySwitchedToTapWater, self.Alarm2, self.byte4_0)
        # information_text += ' Byte 4:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n' \
        #    .format(self.QubicMeters, self.Percent, self.byte5_5, self.byte5_4,
        #            self.byte5_3, self.byte5_2, self.byte5_1, self.byte5_0)
        return information_text


class RCSwitchDisplayMessage(RCMessage):
    msgIdentifier = b'c'
    msgLength = 1
    msgName = "rcSwitchDisplay"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True

    def decode(self):
        RCMessage.decode(self)

    def get_long_to_information_text(self):
        return ""


class RCSwitchToAMessage(RCMessage):
    msgIdentifier = b'a'
    msgLength = 1
    msgName = "rcSwitchToAMessage"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True

    def decode(self):
        RCMessage.decode(self)

    def get_long_to_information_text(self):
        return ""


class RCSwitchToBMessage(RCMessage):
    msgIdentifier = b'b'
    msgLength = 1
    msgName = "rcSwitchToBMessage"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True

    def decode(self):
        RCMessage.decode(self)

    def get_long_to_information_text(self):
        return ""


class RCSetParameterMessage(RCMessage):
    msgIdentifier = b's'
    msgLength = 1
    msgName = "rcSetParameterMessage"
    msgTeleExample = [0x70, 0x33, 0x07, 0x01, 0x03, 0x01, 0x42, 0x00, 0x31, 0x01, 0x30, 0x06, 0x00]

    def __init__(self):
        RCMessage.__init__(self)
        self.msgPrintLongInfo = True

    def decode(self):
        RCMessage.decode(self)

    def get_long_to_information_text(self):
        return ""
