#pragma once

#define paramDelay(time) (uint32_t)( \
            (time & 0xC000) == 0xC000 ? (time & 0x3FFF) * 100 : \
            (time & 0xC000) == 0x0000 ? (time & 0x3FFF) * 1000 : \
            (time & 0xC000) == 0x4000 ? (time & 0x3FFF) * 60000 : \
            (time & 0xC000) == 0x8000 ? ((time & 0x3FFF) > 1000 ? 3600000 : \
                                            (time & 0x3FFF) * 3600000 ) : 0 )
//--------------------Allgemein---------------------------
#define MAIN_ApplicationNumber 0x0034
#define MAIN_ApplicationVersion 0x11
#define MAIN_OrderNumber "SB_owgw_1.0"
#define MAIN_ParameterSize 513
#define MAIN_MaxKoNumber 23


#define APP_SearchSendAddressChannelD		0x00FF
// Offset: 255, Size: 1 Bit, Text: Kanal D: Unbekannte Adresse suchen und senden
#define ParamAPP_SearchSendAddressChannelD knx.paramBit(APP_SearchSendAddressChannelD, 0)
#define APP_SearchSendAddressChannelC		0x00FF
// Offset: 255, BitOffset: 1, Size: 1 Bit, Text: Kanal C: Unbekannte Adresse suchen und senden
#define ParamAPP_SearchSendAddressChannelC knx.paramBit(APP_SearchSendAddressChannelC, 1)
#define APP_SearchSendAddressChannelB		0x00FF
// Offset: 255, BitOffset: 2, Size: 1 Bit, Text: Kanal B: Unbekannte Adresse suchen und senden
#define ParamAPP_SearchSendAddressChannelB knx.paramBit(APP_SearchSendAddressChannelB, 2)
#define APP_SearchSendAddressChannelA		0x00FF
// Offset: 255, BitOffset: 3, Size: 1 Bit, Text: Kanal A: Unbekannte Adresse suchen und senden
#define ParamAPP_SearchSendAddressChannelA knx.paramBit(APP_SearchSendAddressChannelA, 3)
#define APP_ParasiteModeChannelD		0x00FF
// Offset: 255, BitOffset: 4, Size: 1 Bit, Text: Kanal D: Parasitäre Versorgung
#define ParamAPP_ParasiteModeChannelD knx.paramBit(APP_ParasiteModeChannelD, 4)
#define APP_ParasiteModeChannelC		0x00FF
// Offset: 255, BitOffset: 5, Size: 1 Bit, Text: Kanal C: Parasitäre Versorgung
#define ParamAPP_ParasiteModeChannelC knx.paramBit(APP_ParasiteModeChannelC, 5)
#define APP_ParasiteModeChannelB		0x00FF
// Offset: 255, BitOffset: 6, Size: 1 Bit, Text: Kanal B: Parasitäre Versorgung
#define ParamAPP_ParasiteModeChannelB knx.paramBit(APP_ParasiteModeChannelB, 6)
#define APP_ParasiteModeChannelA		0x00FF
// Offset: 255, BitOffset: 7, Size: 1 Bit, Text: Kanal A: Parasitäre Versorgung
#define ParamAPP_ParasiteModeChannelA knx.paramBit(APP_ParasiteModeChannelA, 7)
#define APP_OWDevice1Address		0x0100
// Offset: 256, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice1Address knx.paramData(APP_OWDevice1Address)
#define APP_Device1Type		0x0110
#define APP_Device1Type_Shift	4
#define APP_Device1Type_Mask	0x000F
// Offset: 272, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device1Type ((uint)((knx.paramByte(APP_Device1Type) >> APP_Device1Type_Shift) & APP_Device1Type_Mask))
#define APP_Device1CyclicSend		0x0110
// Offset: 272, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device1CyclicSend knx.paramBit(APP_Device1CyclicSend, 4)
#define APP_Device1ValueChangeSend		0x0110
// Offset: 272, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device1ValueChangeSend knx.paramBit(APP_Device1ValueChangeSend, 5)
#define APP_Device1CyclicTime		0x0111
// Offset: 273, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device1CyclicTime ((uint)((knx.paramByte(APP_Device1CyclicTime))))
#define APP_Device1ValueChangeValue		0x0112
// Offset: 274, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device1ValueChangeValue knx.paramFloat(APP_Device1ValueChangeValue, Float_Enc_DPT9)
#define APP_Device1CorrectionValue		0x0114
// Offset: 276, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device1CorrectionValue knx.paramFloat(APP_Device1CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice2Address		0x0116
// Offset: 278, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice2Address knx.paramData(APP_OWDevice2Address)
#define APP_Device2Type		0x0126
#define APP_Device2Type_Shift	4
#define APP_Device2Type_Mask	0x000F
// Offset: 294, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device2Type ((uint)((knx.paramByte(APP_Device2Type) >> APP_Device2Type_Shift) & APP_Device2Type_Mask))
#define APP_Device2CyclicSend		0x0126
// Offset: 294, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device2CyclicSend knx.paramBit(APP_Device2CyclicSend, 4)
#define APP_Device2ValueChangeSend		0x0126
// Offset: 294, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device2ValueChangeSend knx.paramBit(APP_Device2ValueChangeSend, 5)
#define APP_Device2CyclicTime		0x0127
// Offset: 295, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device2CyclicTime ((uint)((knx.paramByte(APP_Device2CyclicTime))))
#define APP_Device2ValueChangeValue		0x0128
// Offset: 296, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device2ValueChangeValue knx.paramFloat(APP_Device2ValueChangeValue, Float_Enc_DPT9)
#define APP_Device2CorrectionValue		0x012A
// Offset: 298, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device2CorrectionValue knx.paramFloat(APP_Device2CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice3Address		0x012C
// Offset: 300, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice3Address knx.paramData(APP_OWDevice3Address)
#define APP_Device3Type		0x013C
#define APP_Device3Type_Shift	4
#define APP_Device3Type_Mask	0x000F
// Offset: 316, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device3Type ((uint)((knx.paramByte(APP_Device3Type) >> APP_Device3Type_Shift) & APP_Device3Type_Mask))
#define APP_Device3CyclicSend		0x013C
// Offset: 316, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device3CyclicSend knx.paramBit(APP_Device3CyclicSend, 4)
#define APP_Device3ValueChangeSend		0x013C
// Offset: 316, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device3ValueChangeSend knx.paramBit(APP_Device3ValueChangeSend, 5)
#define APP_Device3CyclicTime		0x013D
// Offset: 317, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device3CyclicTime ((uint)((knx.paramByte(APP_Device3CyclicTime))))
#define APP_Device3ValueChangeValue		0x013E
// Offset: 318, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device3ValueChangeValue knx.paramFloat(APP_Device3ValueChangeValue, Float_Enc_DPT9)
#define APP_Device3CorrectionValue		0x0140
// Offset: 320, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device3CorrectionValue knx.paramFloat(APP_Device3CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice4Address		0x0142
// Offset: 322, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice4Address knx.paramData(APP_OWDevice4Address)
#define APP_Device4Type		0x0152
#define APP_Device4Type_Shift	4
#define APP_Device4Type_Mask	0x000F
// Offset: 338, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device4Type ((uint)((knx.paramByte(APP_Device4Type) >> APP_Device4Type_Shift) & APP_Device4Type_Mask))
#define APP_Device4CyclicSend		0x0152
// Offset: 338, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device4CyclicSend knx.paramBit(APP_Device4CyclicSend, 4)
#define APP_Device4ValueChangeSend		0x0152
// Offset: 338, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device4ValueChangeSend knx.paramBit(APP_Device4ValueChangeSend, 5)
#define APP_Device4CyclicTime		0x0153
// Offset: 339, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device4CyclicTime ((uint)((knx.paramByte(APP_Device4CyclicTime))))
#define APP_Device4ValueChangeValue		0x0154
// Offset: 340, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device4ValueChangeValue knx.paramFloat(APP_Device4ValueChangeValue, Float_Enc_DPT9)
#define APP_Device4CorrectionValue		0x0156
// Offset: 342, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device4CorrectionValue knx.paramFloat(APP_Device4CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice5Address		0x0158
// Offset: 344, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice5Address knx.paramData(APP_OWDevice5Address)
#define APP_Device5Type		0x0168
#define APP_Device5Type_Shift	4
#define APP_Device5Type_Mask	0x000F
// Offset: 360, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device5Type ((uint)((knx.paramByte(APP_Device5Type) >> APP_Device5Type_Shift) & APP_Device5Type_Mask))
#define APP_Device5CyclicSend		0x0168
// Offset: 360, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device5CyclicSend knx.paramBit(APP_Device5CyclicSend, 4)
#define APP_Device5ValueChangeSend		0x0168
// Offset: 360, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device5ValueChangeSend knx.paramBit(APP_Device5ValueChangeSend, 5)
#define APP_Device5CyclicTime		0x0169
// Offset: 361, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device5CyclicTime ((uint)((knx.paramByte(APP_Device5CyclicTime))))
#define APP_Device5ValueChangeValue		0x016A
// Offset: 362, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device5ValueChangeValue knx.paramFloat(APP_Device5ValueChangeValue, Float_Enc_DPT9)
#define APP_Device5CorrectionValue		0x016C
// Offset: 364, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device5CorrectionValue knx.paramFloat(APP_Device5CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice6Address		0x016E
// Offset: 366, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice6Address knx.paramData(APP_OWDevice6Address)
#define APP_Device6Type		0x017E
#define APP_Device6Type_Shift	4
#define APP_Device6Type_Mask	0x000F
// Offset: 382, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device6Type ((uint)((knx.paramByte(APP_Device6Type) >> APP_Device6Type_Shift) & APP_Device6Type_Mask))
#define APP_Device6CyclicSend		0x017E
// Offset: 382, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device6CyclicSend knx.paramBit(APP_Device6CyclicSend, 4)
#define APP_Device6ValueChangeSend		0x017E
// Offset: 382, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device6ValueChangeSend knx.paramBit(APP_Device6ValueChangeSend, 5)
#define APP_Device6CyclicTime		0x017F
// Offset: 383, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device6CyclicTime ((uint)((knx.paramByte(APP_Device6CyclicTime))))
#define APP_Device6ValueChangeValue		0x0180
// Offset: 384, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device6ValueChangeValue knx.paramFloat(APP_Device6ValueChangeValue, Float_Enc_DPT9)
#define APP_Device6CorrectionValue		0x0182
// Offset: 386, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device6CorrectionValue knx.paramFloat(APP_Device6CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice7Address		0x0184
// Offset: 388, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice7Address knx.paramData(APP_OWDevice7Address)
#define APP_Device7Type		0x0194
#define APP_Device7Type_Shift	4
#define APP_Device7Type_Mask	0x000F
// Offset: 404, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device7Type ((uint)((knx.paramByte(APP_Device7Type) >> APP_Device7Type_Shift) & APP_Device7Type_Mask))
#define APP_Device7CyclicSend		0x0194
// Offset: 404, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device7CyclicSend knx.paramBit(APP_Device7CyclicSend, 4)
#define APP_Device7ValueChangeSend		0x0194
// Offset: 404, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device7ValueChangeSend knx.paramBit(APP_Device7ValueChangeSend, 5)
#define APP_Device7CyclicTime		0x0195
// Offset: 405, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device7CyclicTime ((uint)((knx.paramByte(APP_Device7CyclicTime))))
#define APP_Device7ValueChangeValue		0x0196
// Offset: 406, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device7ValueChangeValue knx.paramFloat(APP_Device7ValueChangeValue, Float_Enc_DPT9)
#define APP_Device7CorrectionValue		0x0198
// Offset: 408, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device7CorrectionValue knx.paramFloat(APP_Device7CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice8Address		0x019A
// Offset: 410, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice8Address knx.paramData(APP_OWDevice8Address)
#define APP_Device8Type		0x01AA
#define APP_Device8Type_Shift	4
#define APP_Device8Type_Mask	0x000F
// Offset: 426, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device8Type ((uint)((knx.paramByte(APP_Device8Type) >> APP_Device8Type_Shift) & APP_Device8Type_Mask))
#define APP_Device8CyclicSend		0x01AA
// Offset: 426, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device8CyclicSend knx.paramBit(APP_Device8CyclicSend, 4)
#define APP_Device8ValueChangeSend		0x01AA
// Offset: 426, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device8ValueChangeSend knx.paramBit(APP_Device8ValueChangeSend, 5)
#define APP_Device8CyclicTime		0x01AB
// Offset: 427, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device8CyclicTime ((uint)((knx.paramByte(APP_Device8CyclicTime))))
#define APP_Device8ValueChangeValue		0x01AC
// Offset: 428, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device8ValueChangeValue knx.paramFloat(APP_Device8ValueChangeValue, Float_Enc_DPT9)
#define APP_Device8CorrectionValue		0x01AE
// Offset: 430, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device8CorrectionValue knx.paramFloat(APP_Device8CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice9Address		0x01B0
// Offset: 432, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice9Address knx.paramData(APP_OWDevice9Address)
#define APP_Device9Type		0x01C0
#define APP_Device9Type_Shift	4
#define APP_Device9Type_Mask	0x000F
// Offset: 448, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device9Type ((uint)((knx.paramByte(APP_Device9Type) >> APP_Device9Type_Shift) & APP_Device9Type_Mask))
#define APP_Device9CyclicSend		0x01C0
// Offset: 448, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device9CyclicSend knx.paramBit(APP_Device9CyclicSend, 4)
#define APP_Device9ValueChangeSend		0x01C0
// Offset: 448, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device9ValueChangeSend knx.paramBit(APP_Device9ValueChangeSend, 5)
#define APP_Device9CyclicTime		0x01C1
// Offset: 449, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device9CyclicTime ((uint)((knx.paramByte(APP_Device9CyclicTime))))
#define APP_Device9ValueChangeValue		0x01C2
// Offset: 450, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device9ValueChangeValue knx.paramFloat(APP_Device9ValueChangeValue, Float_Enc_DPT9)
#define APP_Device9CorrectionValue		0x01C4
// Offset: 452, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device9CorrectionValue knx.paramFloat(APP_Device9CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice10Address		0x01C6
// Offset: 454, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice10Address knx.paramData(APP_OWDevice10Address)
#define APP_Device10Type		0x01D6
#define APP_Device10Type_Shift	4
#define APP_Device10Type_Mask	0x000F
// Offset: 470, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device10Type ((uint)((knx.paramByte(APP_Device10Type) >> APP_Device10Type_Shift) & APP_Device10Type_Mask))
#define APP_Device10CyclicSend		0x01D6
// Offset: 470, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device10CyclicSend knx.paramBit(APP_Device10CyclicSend, 4)
#define APP_Device10ValueChangeSend		0x01D6
// Offset: 470, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device10ValueChangeSend knx.paramBit(APP_Device10ValueChangeSend, 5)
#define APP_Device10CyclicTime		0x01D7
// Offset: 471, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device10CyclicTime ((uint)((knx.paramByte(APP_Device10CyclicTime))))
#define APP_Device10ValueChangeValue		0x01D8
// Offset: 472, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device10ValueChangeValue knx.paramFloat(APP_Device10ValueChangeValue, Float_Enc_DPT9)
#define APP_Device10CorrectionValue		0x01DA
// Offset: 474, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device10CorrectionValue knx.paramFloat(APP_Device10CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice11Address		0x01DC
// Offset: 476, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice11Address knx.paramData(APP_OWDevice11Address)
#define APP_Device11Type		0x01EC
#define APP_Device11Type_Shift	4
#define APP_Device11Type_Mask	0x000F
// Offset: 492, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device11Type ((uint)((knx.paramByte(APP_Device11Type) >> APP_Device11Type_Shift) & APP_Device11Type_Mask))
#define APP_Device11CyclicSend		0x01EC
// Offset: 492, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device11CyclicSend knx.paramBit(APP_Device11CyclicSend, 4)
#define APP_Device11ValueChangeSend		0x01EC
// Offset: 492, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device11ValueChangeSend knx.paramBit(APP_Device11ValueChangeSend, 5)
#define APP_Device11CyclicTime		0x01ED
// Offset: 493, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device11CyclicTime ((uint)((knx.paramByte(APP_Device11CyclicTime))))
#define APP_Device11ValueChangeValue		0x01EE
// Offset: 494, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device11ValueChangeValue knx.paramFloat(APP_Device11ValueChangeValue, Float_Enc_DPT9)
#define APP_Device11CorrectionValue		0x01F0
// Offset: 496, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device11CorrectionValue knx.paramFloat(APP_Device11CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice12Address		0x01F2
// Offset: 498, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice12Address knx.paramData(APP_OWDevice12Address)
#define APP_Device12Type		0x0202
#define APP_Device12Type_Shift	4
#define APP_Device12Type_Mask	0x000F
// Offset: 514, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device12Type ((uint)((knx.paramByte(APP_Device12Type) >> APP_Device12Type_Shift) & APP_Device12Type_Mask))
#define APP_Device12CyclicSend		0x0202
// Offset: 514, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device12CyclicSend knx.paramBit(APP_Device12CyclicSend, 4)
#define APP_Device12ValueChangeSend		0x0202
// Offset: 514, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device12ValueChangeSend knx.paramBit(APP_Device12ValueChangeSend, 5)
#define APP_Device12CyclicTime		0x0203
// Offset: 515, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device12CyclicTime ((uint)((knx.paramByte(APP_Device12CyclicTime))))
#define APP_Device12ValueChangeValue		0x0204
// Offset: 516, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device12ValueChangeValue knx.paramFloat(APP_Device12ValueChangeValue, Float_Enc_DPT9)
#define APP_Device12CorrectionValue		0x0206
// Offset: 518, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device12CorrectionValue knx.paramFloat(APP_Device12CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice13Address		0x0208
// Offset: 520, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice13Address knx.paramData(APP_OWDevice13Address)
#define APP_Device13Type		0x0218
#define APP_Device13Type_Shift	4
#define APP_Device13Type_Mask	0x000F
// Offset: 536, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device13Type ((uint)((knx.paramByte(APP_Device13Type) >> APP_Device13Type_Shift) & APP_Device13Type_Mask))
#define APP_Device13CyclicSend		0x0218
// Offset: 536, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device13CyclicSend knx.paramBit(APP_Device13CyclicSend, 4)
#define APP_Device13ValueChangeSend		0x0218
// Offset: 536, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device13ValueChangeSend knx.paramBit(APP_Device13ValueChangeSend, 5)
#define APP_Device13CyclicTime		0x0219
// Offset: 537, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device13CyclicTime ((uint)((knx.paramByte(APP_Device13CyclicTime))))
#define APP_Device13ValueChangeValue		0x021A
// Offset: 538, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device13ValueChangeValue knx.paramFloat(APP_Device13ValueChangeValue, Float_Enc_DPT9)
#define APP_Device13CorrectionValue		0x021C
// Offset: 540, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device13CorrectionValue knx.paramFloat(APP_Device13CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice14Address		0x021E
// Offset: 542, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice14Address knx.paramData(APP_OWDevice14Address)
#define APP_Device14Type		0x022E
#define APP_Device14Type_Shift	4
#define APP_Device14Type_Mask	0x000F
// Offset: 558, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device14Type ((uint)((knx.paramByte(APP_Device14Type) >> APP_Device14Type_Shift) & APP_Device14Type_Mask))
#define APP_Device14CyclicSend		0x022E
// Offset: 558, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device14CyclicSend knx.paramBit(APP_Device14CyclicSend, 4)
#define APP_Device14ValueChangeSend		0x022E
// Offset: 558, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device14ValueChangeSend knx.paramBit(APP_Device14ValueChangeSend, 5)
#define APP_Device14CyclicTime		0x022F
// Offset: 559, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device14CyclicTime ((uint)((knx.paramByte(APP_Device14CyclicTime))))
#define APP_Device14ValueChangeValue		0x0230
// Offset: 560, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device14ValueChangeValue knx.paramFloat(APP_Device14ValueChangeValue, Float_Enc_DPT9)
#define APP_Device14CorrectionValue		0x0232
// Offset: 562, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device14CorrectionValue knx.paramFloat(APP_Device14CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice15Address		0x0234
// Offset: 564, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice15Address knx.paramData(APP_OWDevice15Address)
#define APP_Device15Type		0x0244
#define APP_Device15Type_Shift	4
#define APP_Device15Type_Mask	0x000F
// Offset: 580, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device15Type ((uint)((knx.paramByte(APP_Device15Type) >> APP_Device15Type_Shift) & APP_Device15Type_Mask))
#define APP_Device15CyclicSend		0x0244
// Offset: 580, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device15CyclicSend knx.paramBit(APP_Device15CyclicSend, 4)
#define APP_Device15ValueChangeSend		0x0244
// Offset: 580, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device15ValueChangeSend knx.paramBit(APP_Device15ValueChangeSend, 5)
#define APP_Device15CyclicTime		0x0245
// Offset: 581, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device15CyclicTime ((uint)((knx.paramByte(APP_Device15CyclicTime))))
#define APP_Device15ValueChangeValue		0x0246
// Offset: 582, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device15ValueChangeValue knx.paramFloat(APP_Device15ValueChangeValue, Float_Enc_DPT9)
#define APP_Device15CorrectionValue		0x0248
// Offset: 584, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device15CorrectionValue knx.paramFloat(APP_Device15CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice16Address		0x024A
// Offset: 586, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice16Address knx.paramData(APP_OWDevice16Address)
#define APP_Device16Type		0x025A
#define APP_Device16Type_Shift	4
#define APP_Device16Type_Mask	0x000F
// Offset: 602, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device16Type ((uint)((knx.paramByte(APP_Device16Type) >> APP_Device16Type_Shift) & APP_Device16Type_Mask))
#define APP_Device16CyclicSend		0x025A
// Offset: 602, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device16CyclicSend knx.paramBit(APP_Device16CyclicSend, 4)
#define APP_Device16ValueChangeSend		0x025A
// Offset: 602, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device16ValueChangeSend knx.paramBit(APP_Device16ValueChangeSend, 5)
#define APP_Device16CyclicTime		0x025B
// Offset: 603, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device16CyclicTime ((uint)((knx.paramByte(APP_Device16CyclicTime))))
#define APP_Device16ValueChangeValue		0x025C
// Offset: 604, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device16ValueChangeValue knx.paramFloat(APP_Device16ValueChangeValue, Float_Enc_DPT9)
#define APP_Device16CorrectionValue		0x025E
// Offset: 606, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device16CorrectionValue knx.paramFloat(APP_Device16CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice17Address		0x0260
// Offset: 608, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice17Address knx.paramData(APP_OWDevice17Address)
#define APP_Device17Type		0x0270
#define APP_Device17Type_Shift	4
#define APP_Device17Type_Mask	0x000F
// Offset: 624, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device17Type ((uint)((knx.paramByte(APP_Device17Type) >> APP_Device17Type_Shift) & APP_Device17Type_Mask))
#define APP_Device17CyclicSend		0x0270
// Offset: 624, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device17CyclicSend knx.paramBit(APP_Device17CyclicSend, 4)
#define APP_Device17ValueChangeSend		0x0270
// Offset: 624, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device17ValueChangeSend knx.paramBit(APP_Device17ValueChangeSend, 5)
#define APP_Device17CyclicTime		0x0271
// Offset: 625, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device17CyclicTime ((uint)((knx.paramByte(APP_Device17CyclicTime))))
#define APP_Device17ValueChangeValue		0x0272
// Offset: 626, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device17ValueChangeValue knx.paramFloat(APP_Device17ValueChangeValue, Float_Enc_DPT9)
#define APP_Device17CorrectionValue		0x0274
// Offset: 628, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device17CorrectionValue knx.paramFloat(APP_Device17CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice18Address		0x0276
// Offset: 630, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice18Address knx.paramData(APP_OWDevice18Address)
#define APP_Device18Type		0x0286
#define APP_Device18Type_Shift	4
#define APP_Device18Type_Mask	0x000F
// Offset: 646, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device18Type ((uint)((knx.paramByte(APP_Device18Type) >> APP_Device18Type_Shift) & APP_Device18Type_Mask))
#define APP_Device18CyclicSend		0x0286
// Offset: 646, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device18CyclicSend knx.paramBit(APP_Device18CyclicSend, 4)
#define APP_Device18ValueChangeSend		0x0286
// Offset: 646, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device18ValueChangeSend knx.paramBit(APP_Device18ValueChangeSend, 5)
#define APP_Device18CyclicTime		0x0287
// Offset: 647, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device18CyclicTime ((uint)((knx.paramByte(APP_Device18CyclicTime))))
#define APP_Device18ValueChangeValue		0x0288
// Offset: 648, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device18ValueChangeValue knx.paramFloat(APP_Device18ValueChangeValue, Float_Enc_DPT9)
#define APP_Device18CorrectionValue		0x028A
// Offset: 650, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device18CorrectionValue knx.paramFloat(APP_Device18CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice19Address		0x028C
// Offset: 652, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice19Address knx.paramData(APP_OWDevice19Address)
#define APP_Device19Type		0x029C
#define APP_Device19Type_Shift	4
#define APP_Device19Type_Mask	0x000F
// Offset: 668, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device19Type ((uint)((knx.paramByte(APP_Device19Type) >> APP_Device19Type_Shift) & APP_Device19Type_Mask))
#define APP_Device19CyclicSend		0x029C
// Offset: 668, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device19CyclicSend knx.paramBit(APP_Device19CyclicSend, 4)
#define APP_Device19ValueChangeSend		0x029C
// Offset: 668, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device19ValueChangeSend knx.paramBit(APP_Device19ValueChangeSend, 5)
#define APP_Device19CyclicTime		0x029D
// Offset: 669, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device19CyclicTime ((uint)((knx.paramByte(APP_Device19CyclicTime))))
#define APP_Device19ValueChangeValue		0x029E
// Offset: 670, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device19ValueChangeValue knx.paramFloat(APP_Device19ValueChangeValue, Float_Enc_DPT9)
#define APP_Device19CorrectionValue		0x02A0
// Offset: 672, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device19CorrectionValue knx.paramFloat(APP_Device19CorrectionValue, Float_Enc_DPT9)
#define APP_OWDevice20Address		0x02A2
// Offset: 674, Size: 128 Bit (16 Byte), Text: OneWire Adresse
#define ParamAPP_OWDevice20Address knx.paramData(APP_OWDevice20Address)
#define APP_Device20Type		0x02B2
#define APP_Device20Type_Shift	4
#define APP_Device20Type_Mask	0x000F
// Offset: 690, Size: 4 Bit, Text: OneWire Gerät Typ
#define ParamAPP_Device20Type ((uint)((knx.paramByte(APP_Device20Type) >> APP_Device20Type_Shift) & APP_Device20Type_Mask))
#define APP_Device20CyclicSend		0x02B2
// Offset: 690, BitOffset: 4, Size: 1 Bit, Text: Zyklisch senden
#define ParamAPP_Device20CyclicSend knx.paramBit(APP_Device20CyclicSend, 4)
#define APP_Device20ValueChangeSend		0x02B2
// Offset: 690, BitOffset: 5, Size: 1 Bit, Text: Senden nach Wertänderung
#define ParamAPP_Device20ValueChangeSend knx.paramBit(APP_Device20ValueChangeSend, 5)
#define APP_Device20CyclicTime		0x02B3
// Offset: 691, Size: 8 Bit (1 Byte), Text: Wert senden Zykluszeit [min]
#define ParamAPP_Device20CyclicTime ((uint)((knx.paramByte(APP_Device20CyclicTime))))
#define APP_Device20ValueChangeValue		0x02B4
// Offset: 692, Size: 16 Bit (2 Byte), Text: Wertänderung
#define ParamAPP_Device20ValueChangeValue knx.paramFloat(APP_Device20ValueChangeValue, Float_Enc_DPT9)
#define APP_Device20CorrectionValue		0x02B6
// Offset: 694, Size: 16 Bit (2 Byte), Text: Korrekturwert
#define ParamAPP_Device20CorrectionValue knx.paramFloat(APP_Device20CorrectionValue, Float_Enc_DPT9)
//!< Number: 0, Text: OneWire Testanschluss Kanal A, Function: Adresse senden
#define APP_KoOneWire_Testanschluss_Kanal_A 0
#define KoAPP_OneWire_Testanschluss_Kanal_A knx.getGroupObject(APP_KoOneWire_Testanschluss_Kanal_A)
//!< Number: 1, Text: OneWire Testanschluss Kanal B, Function: Adresse senden
#define APP_KoOneWire_Testanschluss_Kanal_B 1
#define KoAPP_OneWire_Testanschluss_Kanal_B knx.getGroupObject(APP_KoOneWire_Testanschluss_Kanal_B)
//!< Number: 2, Text: OneWire Testanschluss Kanal C, Function: Adresse senden
#define APP_KoOneWire_Testanschluss_Kanal_C 2
#define KoAPP_OneWire_Testanschluss_Kanal_C knx.getGroupObject(APP_KoOneWire_Testanschluss_Kanal_C)
//!< Number: 3, Text: OneWire Testanschluss Kanal D, Function: Adresse senden
#define APP_KoOneWire_Testanschluss_Kanal_D 3
#define KoAPP_OneWire_Testanschluss_Kanal_D knx.getGroupObject(APP_KoOneWire_Testanschluss_Kanal_D)
//!< Number: 4, Text: Kanal A: OneWire Temperatursensor 1, Function: Sensorwert
#define APP_KoOneWireTempChannelADevice1 4
#define KoAPP_OneWireTempChannelADevice1 knx.getGroupObject(APP_KoOneWireTempChannelADevice1)
//!< Number: 5, Text: Kanal A: OneWire Temperatursensor 2, Function: Sensorwert
#define APP_KoOneWireTempChannelADevice2 5
#define KoAPP_OneWireTempChannelADevice2 knx.getGroupObject(APP_KoOneWireTempChannelADevice2)
//!< Number: 6, Text: Kanal A: OneWire Temperatursensor 3, Function: Sensorwert
#define APP_KoOneWireTempChannelADevice3 6
#define KoAPP_OneWireTempChannelADevice3 knx.getGroupObject(APP_KoOneWireTempChannelADevice3)
//!< Number: 7, Text: Kanal A: OneWire Temperatursensor 4, Function: Sensorwert
#define APP_KoOneWireTempChannelADevice4 7
#define KoAPP_OneWireTempChannelADevice4 knx.getGroupObject(APP_KoOneWireTempChannelADevice4)
//!< Number: 8, Text: Kanal A: OneWire Temperatursensor 5, Function: Sensorwert
#define APP_KoOneWireTempChannelADevice5 8
#define KoAPP_OneWireTempChannelADevice5 knx.getGroupObject(APP_KoOneWireTempChannelADevice5)
//!< Number: 9, Text: Kanal B: OneWire Temperatursensor 1, Function: Sensorwert
#define APP_KoOneWireTempChannelBDevice1 9
#define KoAPP_OneWireTempChannelBDevice1 knx.getGroupObject(APP_KoOneWireTempChannelBDevice1)
//!< Number: 10, Text: Kanal B: OneWire Temperatursensor 2, Function: Sensorwert
#define APP_KoOneWireTempChannelBDevice2 10
#define KoAPP_OneWireTempChannelBDevice2 knx.getGroupObject(APP_KoOneWireTempChannelBDevice2)
//!< Number: 11, Text: Kanal B: OneWire Temperatursensor 3, Function: Sensorwert
#define APP_KoOneWireTempChannelBDevice3 11
#define KoAPP_OneWireTempChannelBDevice3 knx.getGroupObject(APP_KoOneWireTempChannelBDevice3)
//!< Number: 12, Text: Kanal B: OneWire Temperatursensor 4, Function: Sensorwert
#define APP_KoOneWireTempChannelBDevice4 12
#define KoAPP_OneWireTempChannelBDevice4 knx.getGroupObject(APP_KoOneWireTempChannelBDevice4)
//!< Number: 13, Text: Kanal B: OneWire Temperatursensor 5, Function: Sensorwert
#define APP_KoOneWireTempChannelBDevice5 13
#define KoAPP_OneWireTempChannelBDevice5 knx.getGroupObject(APP_KoOneWireTempChannelBDevice5)
//!< Number: 14, Text: Kanal C: OneWire Temperatursensor 1, Function: Sensorwert
#define APP_KoOneWireTempChannelCDevice1 14
#define KoAPP_OneWireTempChannelCDevice1 knx.getGroupObject(APP_KoOneWireTempChannelCDevice1)
//!< Number: 15, Text: Kanal C: OneWire Temperatursensor 2, Function: Sensorwert
#define APP_KoOneWireTempChannelCDevice2 15
#define KoAPP_OneWireTempChannelCDevice2 knx.getGroupObject(APP_KoOneWireTempChannelCDevice2)
//!< Number: 16, Text: Kanal C: OneWire Temperatursensor 3, Function: Sensorwert
#define APP_KoOneWireTempChannelCDevice3 16
#define KoAPP_OneWireTempChannelCDevice3 knx.getGroupObject(APP_KoOneWireTempChannelCDevice3)
//!< Number: 17, Text: Kanal C: OneWire Temperatursensor 4, Function: Sensorwert
#define APP_KoOneWireTempChannelCDevice4 17
#define KoAPP_OneWireTempChannelCDevice4 knx.getGroupObject(APP_KoOneWireTempChannelCDevice4)
//!< Number: 18, Text: Kanal C: OneWire Temperatursensor 5, Function: Sensorwert
#define APP_KoOneWireTempChannelCDevice5 18
#define KoAPP_OneWireTempChannelCDevice5 knx.getGroupObject(APP_KoOneWireTempChannelCDevice5)
//!< Number: 19, Text: Kanal D: OneWire Temperatursensor 1, Function: Sensorwert
#define APP_KoOneWireTempChannelDDevice1 19
#define KoAPP_OneWireTempChannelDDevice1 knx.getGroupObject(APP_KoOneWireTempChannelDDevice1)
//!< Number: 20, Text: Kanal D: OneWire Temperatursensor 2, Function: Sensorwert
#define APP_KoOneWireTempChannelDDevice2 20
#define KoAPP_OneWireTempChannelDDevice2 knx.getGroupObject(APP_KoOneWireTempChannelDDevice2)
//!< Number: 21, Text: Kanal D: OneWire Temperatursensor 3, Function: Sensorwert
#define APP_KoOneWireTempChannelDDevice3 21
#define KoAPP_OneWireTempChannelDDevice3 knx.getGroupObject(APP_KoOneWireTempChannelDDevice3)
//!< Number: 22, Text: Kanal D: OneWire Temperatursensor 4, Function: Sensorwert
#define APP_KoOneWireTempChannelDDevice4 22
#define KoAPP_OneWireTempChannelDDevice4 knx.getGroupObject(APP_KoOneWireTempChannelDDevice4)
//!< Number: 23, Text: Kanal D: OneWire Temperatursensor 5, Function: Sensorwert
#define APP_KoOneWireTempChannelDDevice5 23
#define KoAPP_OneWireTempChannelDDevice5 knx.getGroupObject(APP_KoOneWireTempChannelDDevice5)

