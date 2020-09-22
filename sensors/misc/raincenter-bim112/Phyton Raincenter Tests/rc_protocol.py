#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# Raincenter protocol
# created: August 16, 2020

import time

def bcd2int(b):
    #print("bcd2int")
    #print(b)
    nibleft = (b & 0xF0) >> 4
    nibright = (b & 0x0F)
    return (nibleft * 10 + nibright)

def bit_is_set(x, n):
    return x & 2**n != 0


class rcMessage(object):
    def __init__(self):
        self.telegram = ''
        self.telegram_bytes = []
        self.msgPrintLongInfo = True
        self.msgTimeStamp = time.strftime("%Y-%m-%d ") + time.strftime("%H:%M:%S")

    def ConvertToReadableByteFormat(s):
        result = ''
        for x in s:
            result += ' {0:02x}'.format(x)
        return result
        
    def GetByteNumberingText(self):
        ByteNumberingText = ''
        for num, bt in enumerate(self.telegram_bytes, start=1):
            ByteNumberingText += " {:02d}".format(num)
        return ByteNumberingText

    def GetLongToInformationText(self):
        return self.ToInformationText()
        
    def Decode(self):
        self.data = self.telegram_bytes # extract the data bytes from the telegram
        
    def ToInformationText(self, printbytes, longoutput):
        InformationText = '{0:s} - {1:20s}\n'.format(self.msgTimeStamp, self.msgName)
        if printbytes :
            InformationText += 'Byte#:    {0:s}\n'.format(self.GetByteNumberingText())
            InformationText += 'Daten:    {0:s}'.format(self.telegram)
        if self.msgPrintLongInfo and longoutput:
            tmp = self.GetLongToInformationText()
            if (len(InformationText) > 0) and (len(tmp) > 0):
                InformationText += '\n'
            InformationText += tmp
        return InformationText
    def GetMsgFromTelegram(telegram_bytes):
        newmsg = None
        if (telegram_bytes[0] == ord(rcDisplayMessage.msgIdentifier)) and (len(telegram_bytes) >= rcDisplayMessage.msgLength):
            newmsg = rcDisplayMessage();
        elif (telegram_bytes[0] == ord(rcParameterMessage.msgIdentifier)) and (len(telegram_bytes) >= rcParameterMessage.msgLength):
            newmsg = rcParameterMessage();
        else:
            pass
            
        if (newmsg):
            newmsg.telegram_bytes = telegram_bytes
            newmsg.telegram = rcMessage.ConvertToReadableByteFormat(telegram_bytes)
            return newmsg
        else:
            return None

class rcParameterMessage(rcMessage):
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
    msgTeleExample = [0x70, 0x30, 0x07, 0x01, 0x03, 0x01, 0x42, 0x00, 0x31, 0x01, 0x30, 0x06, 0x51, 0x51, 0x97, 0x00, 0x00]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        self.WaterExchangePeriod_days = -1
        self.TapWaterSwitchOnHeight_cm = -1
        self.TapWaterSwitchOnHysteresis_cm = -1
        self.WaterExchangeDuration_min = -1
        self.TapWaterSupplyType = -1
        self.FillingLevelMax_cm = -1
        self.ReservoirType = -1
        self.ReservoirArea_m2 = -1
        self.OptionalRelaisFunction = -1
        self.AutomaticTimerInterval_days = -1
        self.AutomaticTimerDuration_seconds = -1
        self.LevelCalibrationFactory = -1
        self.LevelCalibrationUser = -1
        self.LevelMeasured = -1
        self.LevelCalibrated = -1
        
    def Decode(self):
        rcMessage.Decode(self)
        if (self.data[0] == ord(rcParameterMessage.msgIdentifier)) and (len(self.data) >= rcParameterMessage.msgLength):
            self.WaterExchangePeriod_days = bcd2int(self.data[1])
            self.TapWaterSwitchOnHeight_cm = 5*bcd2int(self.data[2])
            self.TapWaterSwitchOnHysteresis_cm = 2*bcd2int(self.data[3])
            self.WaterExchangeDuration_min = bcd2int(self.data[4])
            self.TapWaterSupplyType = bcd2int(self.data[5])
            self.FillingLevelMax_cm = 5*bcd2int(self.data[6])
            self.ReservoirType = bcd2int(self.data[7])
            self.ReservoirArea_m2 = 0.1*bcd2int(self.data[8])
            self.OptionalRelaisFunction = bcd2int(self.data[9])
            self.AutomaticTimerInterval_days = bcd2int(self.data[10])
            self.AutomaticTimerDuration_seconds = 10*bcd2int(self.data[11])
            self.LevelCalibrationFactory = bcd2int(self.data[12])
            self.LevelCalibrationUser = bcd2int(self.data[13])
            self.LevelMeasured = bcd2int(self.data[14]) + 100 * bcd2int(self.data[15])
            self.LevelCalibrated = self.LevelCalibrationUser-51+self.LevelMeasured


    def GetLongToInformationText(self):
        InformationText =  ' Wasseraustauschperiode:                 {0:0d} Tage\n'.format(self.WaterExchangePeriod_days)
        InformationText += ' Einschalthoehe Trinkwassernachspeisung: {0:0d} cm\n'.format(self.TapWaterSwitchOnHeight_cm)
        InformationText += ' Hysterese Trinkwassernachspeisung:      {0:0d} cm\n'.format(self.TapWaterSwitchOnHysteresis_cm)
        InformationText += ' Dauer des Wasserwechselmodus:           {0:0d} Minuten\n'.format(self.WaterExchangeDuration_min)
        InformationText += ' Trinkwassernachspeise-Typ:              {0:0d}\n'.format(self.TapWaterSupplyType)
        InformationText += ' Maximaler Fuellstand:                   {0:0d} cm\n'.format(self.FillingLevelMax_cm)
        if self.ReservoirType == 0 :
            InformationText += ' Behaeltertyp:                           {0:0d} (Speicher mit konstantem Querschnitt)\n'.format(self.ReservoirType)
        else:
            InformationText += ' Behaeltertyp:                           {0:0d} (kugelfoermiger Speicher)\n'.format(self.ReservoirType)
        InformationText += ' Behaelterquerschnitt:                   {0:3.1f} m^2\n'.format(self.ReservoirArea_m2)
        if self.OptionalRelaisFunction == 0:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Keine Funktion)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 1:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Rueckstaualarm)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 2:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Leckwarnalarm)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 3:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Reverse Leckwarn)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 4:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Parallelpumpe)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 5:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (Rueckspuel)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 6:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (LW + mit Anzeige)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 7:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (LW - mit Anzeige)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 8:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (LW + ohne Anzeige)\n'.format(self.OptionalRelaisFunction)
        elif self.OptionalRelaisFunction == 9:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (LW - ohne Anzeige)\n'.format(self.OptionalRelaisFunction)
        else:
            InformationText += ' Funktion des Optionalrelais:            {0:0d} (unbekannt)\n'.format(self.OptionalRelaisFunction)
            
        InformationText += ' Intervall der Zeitschaltautomatik:      {0:0d} Tage\n'.format(self.AutomaticTimerInterval_days)
        InformationText += ' Dauer der Zeitschaltautomatik:          {0:0d} Sekunden\n'.format(self.AutomaticTimerDuration_seconds)
        InformationText += ' Werks-Kalibrierung:                     {0:0d}\n'.format(self.LevelCalibrationFactory)
        InformationText += ' Benutzer-Kalibrierung:                  {0:0d}\n'.format(self.LevelCalibrationUser)
        InformationText += ' Real-Messwert:                          {0:0d} cm\n'.format(self.LevelMeasured)
        InformationText += ' Kalibrierter-Messwert:                  {0:0d} cm\n'.format(self.LevelCalibrated)
        return InformationText

class rcDisplayMessage(rcMessage):
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
# 3.5         optional relais always on
# 3.6         optional relais blinking
# 3.7         buzzer active ?? in PROFIRAIN PCA documentation refered to bit 3.6 which should be for optional relais

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
    msgTeleExample = [0x77, 0x95, 0x00, 0x08, 0x02]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        self.DisplayValue = -1 #byte 1 & 2
        self.DisplayUnit = '-'
        
        self.OptionalLEDAlwaysOn = -1 #byte 3.0
        self.OptionalLEDBlinking = -1 #byte 3.1
        self.AutomaticallySwitchedToTapWater = -1 #byte 3.2
        self.ManualSwitchedToTapWater = -1 #byte 3.3
        self.WaterExchangeActive = -1 #byte 3.4
        self.OptionalRelaisAlwaysOn = -1 #byte 3.5
        self.OptionalRelaisBlinking = -1 #byte 3.6
        self.AlarmBuzzerActive = -1 #byte 3.7

        self.OptionalInputActive = -1 #byte 4.0
        self.TapWaterRefillInputActive = -1 #byte 4.1
        self.PumpActive = -1 #byte 4.2
        self.unused = -1 #byte 4.3
        self.OnButtonPressed = -1 #byte 4.4
        self.OffButtonPressed = -1 #byte 4.5
        self.DisplayInPercent = -1 #byte 4.6
        self.DisplayInQubicMeters = -1 #byte 4.7

    def Decode(self):
        rcMessage.Decode(self)
        if (self.data[0] == ord(rcDisplayMessage.msgIdentifier)) and (len(self.data) >= rcDisplayMessage.msgLength):
            self.DisplayValue = bcd2int(self.data[1]) + 100 * bcd2int(self.data[2])
            # byte 3 decoding
            self.OptionalLEDAlwaysOn = bit_is_set(self.data[3], 0)
            self.OptionalLEDBlinking = bit_is_set(self.data[3], 1)
            self.AutomaticallySwitchedToTapWater = bit_is_set(self.data[3], 2)
            self.ManualSwitchedToTapWater = bit_is_set(self.data[3], 3)
            self.WaterExchangeActive = bit_is_set(self.data[3], 4)
            self.OptionalRelaisAlwaysOn = bit_is_set(self.data[3], 5)
            self.OptionalRelaisBlinking = bit_is_set(self.data[3], 6)
            self.AlarmBuzzerActive = bit_is_set(self.data[3], 7)
            # byte 4 decoding
            self.OptionalInputActive = bit_is_set(self.data[4], 0)
            self.TapWaterRefillInputActive = bit_is_set(self.data[4], 1)
            self.PumpActive = bit_is_set(self.data[4], 2)
            self.unused = bit_is_set(self.data[4], 3) #flickers
            self.OnButtonPressed = bit_is_set(self.data[4], 4)
            self.OffButtonPressed = bit_is_set(self.data[4], 5)
            self.DisplayInPercent = bit_is_set(self.data[4], 6)
            self.DisplayInQubicMeters = bit_is_set(self.data[4], 7)

            if self.DisplayInQubicMeters == True:
                self.DisplayUnit = 'm^3'
                self.DisplayValue = self.DisplayValue / 10
            elif self.DisplayInPercent == True:
                self.DisplayUnit = '%'
            else:
                self.DisplayUnit = 'cm'

    def GetLongToInformationText(self):
        if self.DisplayUnit == 'm^3':
            InformationText =  ' Anzeige:                 {0:.1f} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)
        else:
            InformationText =  ' Anzeige:                 {0:d} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)

        InformationText +=  ' opt. LED an:             {0:0d}\n'.format(self.OptionalLEDAlwaysOn)
        InformationText +=  ' opt. LED blinkt:         {0:0d}\n'.format(self.OptionalLEDBlinking)
        InformationText +=  ' autom. Nachspeisung:     {0:0d}\n'.format(self.AutomaticallySwitchedToTapWater)
        InformationText +=  ' manuelle Nachspeisung:   {0:0d}\n'.format(self.ManualSwitchedToTapWater)
        InformationText +=  ' Wasseraustausch aktiv:   {0:0d}\n'.format(self.WaterExchangeActive)
        InformationText +=  ' opt. Relais an:          {0:0d}\n'.format(self.OptionalRelaisAlwaysOn)
        InformationText +=  ' opt. Relais blinkt:      {0:0d}\n'.format(self.OptionalRelaisBlinking)
        InformationText +=  ' Alarmbuzzer an:          {0:0d}\n'.format(self.AlarmBuzzerActive)

        InformationText +=  ' opt. Eingang an:         {0:0d}\n'.format(self.OptionalInputActive)
        InformationText +=  ' Nachspeise-Eingang an:   {0:0d}\n'.format(self.TapWaterRefillInputActive)
        InformationText +=  ' Pumpe laeuft:            {0:0d}\n'.format(self.PumpActive)
        InformationText +=  ' unbenutzt (flackert):    {0:0d}\n'.format(self.unused)
        InformationText +=  ' ON Button gedrueckt:     {0:0d}\n'.format(self.OnButtonPressed)
        InformationText +=  ' OFF Button gedrueckt:    {0:0d}\n'.format(self.OffButtonPressed)
        InformationText +=  ' Anzeige ist in %:        {0:0d}\n'.format(self.DisplayInPercent)
        InformationText +=  ' Anzeige ist in m^3:      {0:0d}\n'.format(self.DisplayInQubicMeters)
        #InformationText += ' Byte 3:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n'.format(self.byte4_7, self.byte4_6, self.Alarm1, self.byte4_4, self.ManualSwitchedToTapWater, self.AutomaticallySwitchedToTapWater, self.Alarm2, self.byte4_0)
        #InformationText += ' Byte 4:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n'.format(self.QubicMeters, self.Percent, self.byte5_5, self.byte5_4, self.byte5_3, self.byte5_2, self.byte5_1, self.byte5_0)
        return InformationText
        
        
class rcSwitchDisplayMessage(rcMessage):
    msgIdentifier = b'c'
    msgLength = 1
    msgName = "rcSwitchDisplay"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        
    def Decode(self):
        rcMessage.Decode(self)
        
    def GetLongToInformationText(self):
        return ""
        
class rcSwitchToAMessage(rcMessage):
    msgIdentifier = b'a'
    msgLength = 1
    msgName = "rcSwitchToAMessage"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        
    def Decode(self):
        rcMessage.Decode(self)
        
    def GetLongToInformationText(self):
        return ""
        
class rcSwitchToBMessage(rcMessage):
    msgIdentifier = b'b'
    msgLength = 1
    msgName = "rcSwitchToBMessage"
    msgTeleExample = [msgIdentifier]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        
    def Decode(self):
        rcMessage.Decode(self)
        
    def GetLongToInformationText(self):
        return ""
        
class rcSetParameterMessage(rcMessage):
    msgIdentifier = b's'
    msgLength = 1
    msgName = "rcSetParameterMessage"
    msgTeleExample = [0x70, 0x33, 0x07, 0x01, 0x03, 0x01, 0x42, 0x00, 0x31, 0x01, 0x30, 0x06, 0x00]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        
    def Decode(self):
        rcMessage.Decode(self)
        
    def GetLongToInformationText(self):
        return ""