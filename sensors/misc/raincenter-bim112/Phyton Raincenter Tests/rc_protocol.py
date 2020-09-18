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
#Byte#:     01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
#Daten:     70 30 07 01 03 01 42 00 31 01 30 06 51 51 97 00 00

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
#w Display data request Answer:
# Example in hex
# #Byte#:   01 02 03 04 05
# #Data:    77 95 00 08 02

#byte.bit   Description                         Multiplier  Unit    Code
#1.          0x77 = 'w'
#2.          display Value                           100     -       BCD
#3.          display Value                           1       -       BCD
#4.0         unknown
#4.1         alarm
#4.2         ?? automatically switched to tap water refill
#4.3         manually switched to tap water refill
#4.4         unknown
#4.5         ?? Alarm ??
#4.6         unknown
#4.7         unknown
#5.0         unknown
#5.1         unknown
#5.2         unknown
#5.3         unknown / this bit "flickers"
#5.4         unknown
#5.5         unknown
#5.6         display value is in percent
#5.7         display value is in qubic meters

    msgIdentifier = b'w'
    msgLength = 5
    msgName = "rcDisplay"
    msgTeleExample = [0x77, 0x95, 0x00, 0x08, 0x02]

    def __init__(self):
        rcMessage.__init__(self)
        self.msgPrintLongInfo = True
        self.DisplayValue = -1
        self.DisplayUnit = '-'
        self.byte4_7 = -1
        self.byte4_6 = -1
        self.Alarm1 = -1 #byte4_5
        self.byte4_4 = -1
        self.ManualSwitchedToTapWater = -1 #byte4_3
        self.AutomaticallySwitchedToTapWater = -1 #byt4_2
        self.Alarm2 = -1 #byte4_1
        self.byte4_0 = -1
        
        self.QubicMeters = -1
        self.Percent = -1
        self.byte5_5 = -1
        self.byte5_4 = -1
        self.byte5_3 = -1
        self.byte5_2 = -1
        self.byte5_1 = -1
        self.byte5_0 = -1
        
    def Decode(self):
        rcMessage.Decode(self)
        if (self.data[0] == ord(rcDisplayMessage.msgIdentifier)) and (len(self.data) >= rcDisplayMessage.msgLength):
            self.DisplayValue = bcd2int(self.data[1]) + 100 * bcd2int(self.data[2])
            
            self.byte4_7 = bit_is_set(self.data[3], 7)
            self.byte4_6 = bit_is_set(self.data[3], 6)
            self.Alarm1 = bit_is_set(self.data[3], 5)
            self.byte4_4 = bit_is_set(self.data[3], 4)
            
            self.ManualSwitchedToTapWater = bit_is_set(self.data[3], 3)
            self.AutomaticallySwitchedToTapWater = bit_is_set(self.data[3], 2)
            self.Alarm2 = bit_is_set(self.data[3], 1)
            self.byte4_0 = bit_is_set(self.data[3], 0)
            
            self.QubicMeters = bit_is_set(self.data[4], 7)
            self.Percent = bit_is_set(self.data[4], 6)
            self.byte5_5 = bit_is_set(self.data[4], 5)
            self.byte5_4 = bit_is_set(self.data[4], 4)
            
            self.byte5_3 = bit_is_set(self.data[4], 3) #flackert
            self.byte5_2 = bit_is_set(self.data[4], 2)
            self.byte5_1 = bit_is_set(self.data[4], 1)
            self.byte5_0 = bit_is_set(self.data[4], 0)
            
            if self.QubicMeters == True:
                self.DisplayUnit = 'm^3'
                self.DisplayValue = self.DisplayValue / 10
            elif self.Percent == True:
                self.DisplayUnit = '%'
            else:
                self.DisplayUnit = 'cm'

    def GetLongToInformationText(self):
        if self.DisplayUnit == 'm^3':
            InformationText =  ' Anzeige:                 {0:.1f} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)
        else:
            InformationText =  ' Anzeige:                 {0:d} {1:s}\n'.format(self.DisplayValue, self.DisplayUnit)
        
        InformationText +=  ' manuelle Nachspeisung:   {0:0d}\n'.format(self.ManualSwitchedToTapWater)
        InformationText +=  ' autom. Nachspeisung:     {0:0d}\n'.format(self.AutomaticallySwitchedToTapWater)
        InformationText +=  ' Alarm 1:                 {0:0d}\n'.format(self.Alarm1)
        InformationText +=  ' Alarm 2:                 {0:0d}\n'.format(self.Alarm2)
        #InformationText +=  ' Qubikmeter:              {0:0d}\n'.format(self.QubicMeters)
        #InformationText +=  ' Prozent:                 {0:0d}\n'.format(self.Percent)

        InformationText += ' Byte 4:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n'.format(self.byte4_7, self.byte4_6, self.Alarm1, self.byte4_4, self.ManualSwitchedToTapWater, self.AutomaticallySwitchedToTapWater, self.Alarm2, self.byte4_0)
        InformationText += ' Byte 5:                  {0:0d} {1:0d} {2:0d} {3:0d}    {4:0d} {5:0d} {6:0d} {7:0d}\n'.format(self.QubicMeters, self.Percent, self.byte5_5, self.byte5_4, self.byte5_3, self.byte5_2, self.byte5_1, self.byte5_0)
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