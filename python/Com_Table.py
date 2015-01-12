# -*- coding: utf-8 -*-
# Copyright (C) 2014 Martin Glueck All rights reserved
# Langstrasse 4, A--2244 Spannberg, Austria. martin@mangari.org
# #*** <License> ************************************************************#
# This module is part of the library selfbus.
#
# This module is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module. If not, see <http://www.gnu.org/licenses/>.
# #*** </License> ***********************************************************#
#
#++
# Name
#    Com_Table
#
# Purpose
#    The communication object table of the EEPROM
#
# Revision Dates
#    04-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
from    _Table_   import _Table_
from    _Object_  import _Object_, M_Object
import   sys
import   Telegram

class Bit_Mask_Flag (object) :
    """Descriptor converting True/False values into the bit values"""

    def __init__ (self, bit_value) :
        self.bit_value = bit_value
    # end def __init__

    def __get__ (self, obj, cls) :
        if obj is None :
            return self
        return getattr (obj, self.name) == self.bit_value
    # end def __get__

    def __set__ (self, obj, value) :
        setattr (obj, self.name, self.bit_value if value else 0)
    # end def __set__

    def __call__ (self, attr) :
        import pdb; pdb.set_attr ()
    # end def __call__

# end class Bit_Mask_Flag

class _M_Com_Object_ (M_Object) :
    """Meta class for com objects combining the `Defaults` dict of all bases"""

    def __new__ (mcls, name, bases, dct) :
        Defaults       = dict ()
        if name == "Com_Object" :
            dct ["Flags"]  = Flags = ["priority"]
            for n, v in dct.items () :
                if isinstance (v, Bit_Mask_Flag) :
                    v.name = "_a_" + n
                    Flags.append (n)
        for b in reversed (bases) :
            Defaults.update (getattr (b, "Defaults", {}))
        Defaults.update (dct.get ("Defaults", {}))
        dct ["Defaults"] = Defaults
        return super (_M_Com_Object_, mcls).__new__ (mcls, name, bases, dct)
    # end def __new__

# end class _M_Com_Object_

class Com_Object (_Object_, metaclass = _M_Com_Object_) :
    """Base class for a comm object"""

    UINT1   =  0,  1
    UINT2   =  1,  1
    UINT3   =  2,  1
    UINT4   =  3,  1
    UINT5   =  4,  1
    UINT6   =  5,  1
    UINT7   =  6,  1
    UINT8   =  7,  1
    UINT16  =  8,  2
    BYTE3   =  9,  3
    FLOAT   = 10,  4
    DATA4   = 10,  4
    DATA6   = 11,  6
    DATA8   = 12,  8
    DOUBLE  = 12,  8
    DATA10  = 13, 10
    MAXDATA = 14, 14
    VARDATA = 15, 15

    KNX_DPT_ID = \
        { 1 : ### Boolean Data/General
          ( "UINT1"
          , {  1  : "Switch"
            ,  2  : "Bool"
            ,  3  : "Enable"
            ,  4  : "Ramp"
            ,  5  : "Alarm"
            ,  6  : "BinaryValue"
            ,  7  : "Step"
            ,  8  : "UpDown"
            ,  9  : "OpenClose"
            , 10  : "Start"
            , 11  : "State"
            , 12  : "Invert"
            , 15  : "Reset"
            , 16  : "Ack"
            , 17  : "Trigger"
            , 18  : "Occupancy"
            , 19  : "Window_Dorr"
            , 21  : "LogicalFunction"
            , 22  : "Scene_AB"
            }
          )
        , 2 : ### Boolean with Priority
          ( "UINT2"
          , {  1  : "Switch_Control"
            ,  2  : "Bool_Control"
            ,  3  : "Enable_Control"
            ,  4  : "Ramp_Control"
            ,  5  : "Alarm_Control"
            ,  6  : "BinaryValue_Control"
            ,  7  : "Step_Control"
            , 10  : "Start_Control"
            , 11  : "State_Control"
            , 12  : "Invert_Control"
            }
          )
        , 3 : ### 3 bit with control
          ( "UINT4", { 7 : "Control_Dimming", 8: "Control_Blinds"})
        , 4 : ### Single Character
          ( "UINT8", { 1 : "Char_ASCII",      2 : "Char_8859_1"})
        , 5 : ### 8bit unsigned
          ( "UINT8"
          , { 1   : "Scaling"
            , 3   : "Angle"
            , 4   : "Percent_U8"
            , 10  : "Value_1_Ucount"
            }
          )
        , 6 : ### 8bit signed
          ( "UINT8",  { 1   : "Percent_V8", 10  : "Value_1_count"})
        , 7 : ### 16bit unsigned
          ( "UINT16", { 1: "Value_2_Ucount"})
        , 8 : ### 16bit signed
          ( "UINT16", { 1  : "Value_2_count", 10 : "Percent_V16"})
        , 9 : ### 2 byte Float
          ( "UINT16"
          , {  1  : "Value_Temp"
            ,  2  : "Value_Tempd"
            ,  3  : "Value_Tempa"
            ,  4  : "Value_Lux"
            ,  5  : "Value_Wsp"
            ,  6  : "Value_Pres"
            ,  7  : "Value_Humidity"
            ,  8  : "Value_AirQuality"
            , 10  : "Value_Time1"
            , 11  : "Value_Time2"
            , 20  : "Value_Volt"
            , 21  : "Value_Curr"
            , 22  : "Value_PowerDensity"
            , 23  : "Value_KelvinPerPercent"
            , 24  : "Value_Power"
            , 25  : "Value_Volume_Flow"
            }
          )
        , 10 : ### Time of Day
          ( "BYTE3",  { 1 : "TimeOfDay"})
        , 11 : ### Date
          ( "BYTE3",  { 1 : "Date"})
        , 12 : ### Date and Time
          ( "DATA8",  { 1 : "DateTime"})
        , 13 : ### 32bit signed
          ( "UINT16", { 1  : "Value_4_count"})
        , 14 : ### 32bit Float
          ( "FLOAT"
          , {  7 : "Value_AngleDeg"
            , 19 : "Value_Electric_Current"
            , 27 : "Value_Electric_Potential"
            , 28 : "Value_Electric_PotentialDifference"
            , 31 : "Value_Energy"
            , 32 : "Value_Force"
            , 33 : "Value_Frequency"
            , 36 : "Value_Heat_FlowRate"
            , 37 : "Value_Heat_Quantity"
            , 18 : "Value_Impedance"
            , 39 : "Value_Length"
            , 51 : "Value_Mass"
            , 56 : "Value_Power"
            , 65 : "Value_Speed"
            , 66 : "Value_Stress"
            , 67 : "Value_Surface_Tension"
            , 68 : "Value_Common_Temperature"
            , 69 : "Value_Absolute_Temperature"
            , 70 : "Value_Temperature_Difference"
            , 78 : "Value_Weight"
            , 79 : "Value_Work"
            }
          )
        , 15 : ### Access Control
          ( "DATA4", { 0 : "Access_Data"})
        , 16 : ### String
          ( "MAXDATA", { 1 : "Char_ASCII",      2 : "Char_8859_1"})
        , 17 : ### Scene Control
          ( "UINT8", { 1 : "SceneNumber", })
        , 18 : ### Scene Control
          ( "UINT8", { 1 : "SceneControl", })
        }
    Size    = None, 1

    Defaults = dict \
       ( transmit     = False
       , value_in_eep = False
       , write        = True
       , read         = True
       , comm         = True
       , priority     = 3
       )

    def __init__ (self, name, size = None, ** kw) :
        self.name                 = name
        self.type_code, self.size = size or self.Size
        self._grp_addr            = None
        self.rcv_addresses        = []
        for f in self.Flags :
            setattr (self, f, kw.pop (f, self.Defaults [f]))
        self.dpt                  = self.size
        self.dpt_name             = "<undefined>"
        assert not kw, kw
    # end def __init__

    @property
    def type_code (self) :
        tc = self._type_code
        if tc is None :
            tc = self.UINT8 [0]
        return tc
    # end def type_code

    @type_code.setter
    def type_code (self, value) :
        self._type_code = value
    # end def type_code

    def specialize (self, name, size) :
        if isinstance (size, str) :
            self.dpt = size
            if "." in size : ### a KNX data type ID
                main, sub         = (int (p, 10) for p in size.split ("."))
                size, sub_types   = self.KNX_DPT_ID [main]
                self.dpt_name     = sub_types.get (sub, "<undefined>")
            size                  = getattr (self, size)
        self.name                 = name
        self.type_code, self.size = size
        return self
    # end def specialize

    @property
    def group_address (self) :
        return self._grp_addr
    # end def snd_address

    @group_address.setter
    def group_address (self, value) :
        if self._grp_addr :
            self.device.address_table.remove           (self._grp_addr)
        self._grp_addr = self.device.address_table.add (value)
    # end def snd_address

    def add_to_group (self, * grp_addresses) :
        for g in grp_addresses :
            g = self.device.address_table.add (g)
            self.rcv_addresses.append         (g)
    # end def add_to_group

    def update_eeprom (self, eep, eep_off, ram_off) :
        flags             = 0
        for f in self.Flags :
            flags |= getattr (self, "_a_" + f)
        self.ram_off      = ram_off
        self.flags        = flags
        self.eep_off      = eep_off
        eep [eep_off + 0] = ram_off
        eep [eep_off + 1] = flags
        eep [eep_off + 2] = self.type_code
        return eep_off + 3, ram_off + self.size
    # end def update_eeprom

    ### Flags
    transmit     = Bit_Mask_Flag (0b01000000) # 0x40
    value_in_eep = Bit_Mask_Flag (0b00100000) # 0x20
    write        = Bit_Mask_Flag (0b00010000) # 0x10
    read         = Bit_Mask_Flag (0b00001000) # 0x08
    comm         = Bit_Mask_Flag (0b00000100) # 0x04

    @property
    def priority (self) :
        return self._a_priority
    # end def priority

    @priority.setter
    def priority (self, value) :
        self._a_priority = value & 0x03
    # end def setter

    def send_telegram (self, value) :
        size = self.size
        if self.type_code >= 7 :
            size += 1
        return Telegram.Send_Value \
            ( src       = self.device.own_address
            , dst       = self.rcv_addresses [0]
            , value     = value
            , length    = size
            , type_code = self.type_code
            )
    # end def send_telegram
    def send (self, value, ** kw) :
        from     Test_Case import Send_Telegram, Receive_Telegram
        return Send_Telegram (self.send_telegram (value), ** kw)
    # end def send

    def receive_telegram (self, value, src = "1.1.20") :
        size = self.size
        if self.type_code >= 7 :
            size += 1
        return Telegram.Send_Value \
            ( src       = src
            , dst       = self.rcv_addresses [0]
            , value     = value
            , length    = size
            , type_code = self.type_code
            )
    # end def receive_telegram

    def receive (self, value, ** kw) :
        from     Test_Case import Send_Telegram, Receive_Telegram
        return Receive_Telegram \
            ( self.receive_telegram (value, src = kw.pop ("src", "1.1.20"))
            , ** kw
            )
    # end def receive

# end class Com_Object

for code in ( "UINT1", "UINT2", "UINT3", "UINT4"
            , "UINT5", "UINT6", "UINT7", "UINT8"
            , "UINT16", "BYTE3", "FLOAT", "DATA6"
            , "DOUBLE", "DATA10", "MAXDATA"
            ) :
    cls = Com_Object.__class__ \
        ( "CO_%s" % code
        , (Com_Object, )
        , dict (Size = getattr (Com_Object, code))
        )
    setattr (sys.modules [__name__], cls.__name__, cls)
    for prefix, Defaults in ( ("RO", dict (write = False, transmit = True))
                            , ("WO", dict (read  = False))
                            ) :
        mcls = Com_Object.__class__ \
            ( "CO_%s_%s" % (prefix, code)
            , (cls, )
            , dict (Defaults = Defaults)
            )
        setattr (sys.modules [__name__], mcls.__name__, mcls)

class Com_Object_Table (_Table_) :
    """The communication object table"""

    BCU1 = dict \
        ( ptr   = 0x112
        )

    def __init__ (self, device) :
        self.__super.__init__ (device)
        self._objects = []
    # end def __init__

    def add (self, obj, * args, ** kw) :
        number = kw.pop ("number", len (self._objects))
        if not isinstance (obj, Com_Object) :
            obj    = Com_Object (obj, * args, ** kw)
        obj.device = self.device
        obj.number = number
        self._objects.append (obj)
        self._objects.sort (key = lambda o : o.number)
        return obj
    # end def add

    def update_eeprom (self, offset) :
        offset       = self.__super.update_eeprom (offset)
        eep          = self.device._eeprom
        eep [offset] = len (self._objects)
        offset      += 1
        eep [offset] = 0x61 ### XXX
        offset      += 1
        roffset      = 0x61 + (len (self._objects) + 1) // 2
        for obj in self._objects :
            offset, roffset = obj.update_eeprom (eep, offset, roffset)
        return offset
    # end def update_eeprom

    def __iter__ (self) :
        return iter (self._objects)
    # end def __iter__

    def as_c_comment (self, bytes = None) :
        return self._as_string (bytes, "    // ")
    # end def as_c_comment

    def _as_string (self, bytes = None, head = "", sep = "\n") :
        if bytes is None :
            bytes = self.device._eeprom
        result = \
            [ "Com Object table (0x1%02X):"
            % (bytes [self.spec ["ptr"]], )
            ]
        for obj in self._objects :
            result.append \
                ( "  (%2d) %-20s <%02X, %02X, %02X> @ 0x%03X (%s:%s)"
                % ( obj.number
                  , obj.name
                  , obj.ram_off
                  , obj.flags
                  , obj.type_code
                  , obj.eep_off
                  , obj.dpt, obj.dpt_name
                  )
                )
        return sep.join ("%s%s" % (head, l) for l in result)
    # end def _as_string

    def __str__ (self) :
        return self._as_string ()
    # end def __str__

    def __getitem__ (self, key) :
        return self._objects [key]
    # end def __getitem__

# end class Com_Object_Table

### __END__ Com_Table»
