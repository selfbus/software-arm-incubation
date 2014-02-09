# -*- coding: utf-8 -*-
# Copyright (C) 2013-2014 Martin Glueck All rights reserved
# Langstrasse 4, A--2244 Spannberg, Austria. martin@mangari.org
# #*** <License> ************************************************************#
# This module is part of the library selfbus.
#
# This module is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this module. If not, see <http://www.gnu.org/licenses/>.
# #*** </License> ***********************************************************#
#
#++
# Name
#    Telegram
#
# Purpose
#    De/Encoding of a EIB telegram
#
# Revision Dates
#     6-Dec-2013 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
import   struct
import   math

Undefined = object ()

def _m_mangled_attr_name (name, cls_name) :
    if cls_name.startswith ("_") :
        format = "%s__%s"
    else :
        format = "_%s__%s"
    return format % (cls_name, name)
# end def _m_mangled_attr_name

class M_Object (type) :
    """Meta class providing the `__m_super` and `__super` attributes."""

    def _m_mangled_attr_name (cls, name) :
        return _m_mangled_attr_name (name, cls.__name__)
    # end def _m_mangled_attr_name

    def __init__ (cls, name, bases, dict) :
        super (M_Object, cls).__init__          (name, bases, dict)
        _super_name  = cls._m_mangled_attr_name ("super")
        _super_value = super (cls)
        setattr (cls, _super_name, _super_value)
    # end def __init__

# end class M_Object

class _Object_ (object, metaclass = M_Object) :
    """Base class for all CoAP objects"""

# end class _Object_

class _Field_ (_Object_) :
    """Base class for field of a telegram"""

    def __init__ ( self, name, offset, shift, mask
                 , default   = 0
                 , important = False
                 , ** kw
                 ) :
        self.name      = name
        self.offset    = offset
        self.shift     = shift
        self.mask      = mask
        self.default   = default
        self.important = important
        assert not kw, kw
    # end def __init__

# end class _Field_

class Int_Field (_Field_) :
    """Simple integer field"""

    def __init__ (self, * args, ** kw) :
        self.__super.__init__ (* args, ** kw)
        mask = self.mask << self.shift
        if   mask < 0x100 :
            self.code = "!B"
        elif mask < 0x10000 :
            self.code = "!H"
        else :
            self.code = "!I"
        self.size     = struct.calcsize (self.code)
    # end def __init__

    def as_bytes (self, result, value) :
        off = self.offset
        raw = struct.pack (self.code, (value & self.mask) << self.shift)
        new = []
        for o in range (self.size) :
            new.append (bytes ([int (result [off + o]) | int (raw [o])]))
        return result [:off] + b"".join (new) + result [off + self.size:]
    # end def as_bytes

    def from_bytes (self, bytes, kw) :
        raw = bytes [self.offset : self.offset + self.size]
        return (struct.unpack (self.code, raw) [0] >> self.shift) & self.mask
    # end def from_bytes

    def as_string (self, telegram) :
        return getattr (telegram, self.name)
    # end def as_string

    def from_string (self, telegram, value) :
        return eval (value)
    # end def from_string

# end class Int_Field

class Mapping_Field (Int_Field) :
    """Convert the value into a string"""

    def __init__ (self, * args, ** kw) :
        self.mapping = kw.pop ("mapping")
        self.gnippam = dict ((v, str (k)) for (k, v) in self.mapping.items ())
        self.__super.__init__ (* args, ** kw)
    # end def __init__

    def as_string (self, telegram) :
        return self.mapping [getattr (telegram, self.name)]
    # end def as_string

    def from_string (self, telegram, value) :
        return self.__super.from_string \
            (telegram, self.gnippam.get (value, value))
    # end def from_string

# end class Mapping_Field

class _Address_ (_Field_) :
    """The KNX address"""

    class Value (_Object_) :

        physical  = ( ("area",    0xF000, 12)
                    , ("line",    0x0F00,  8)
                    , ("address", 0x00FF,  0)
                    )
        group     = ( ("main",    0x7800, 11)
                    , ("middle",  0x0700,  8)
                    , ("sub",     0x00FF,  0)
                    )
        seperator = "./"

        def __init__ (self, value, group) :
            self._value   = value
            self.fields   = self.group if group else self.physical
            self.seperator = self.seperator [group]
            for name, mask, shift in self.fields :
                setattr (self, name, (value & mask) >> shift)
        # end def __init__

        def __str__ (self) :
            result = []
            for name, _, _ in self.fields :
                result.append (str (getattr (self, name)))
            return self.seperator.join (result)
        # end def __str__

        def __int__ (self) :
            return self._value
        # end def __int__

        @classmethod
        def From_String (cls, value) :
            if "." in value :
                sep  = "."
                spec = cls.physical
            else :
                sep  = "/"
                spec = cls.group
            parts    = value.split (sep)
            result   = 0
            for v, (n, m, s) in zip (parts, spec) :
                result |= int (v) << s
            return cls (result, sep == "/")
        # end def From_String

    # end class Value

    def __init__ (self, name, offset) :
        self.__super.__init__ (name, offset, 0, 0xFFFF, important = True)
    # end def __init__

    def as_bytes (self, result, value) :
        off = self.offset
        raw = struct.pack ("!H", (value._value & self.mask) << self.shift)
        new = []
        for o in range (2) :
            new.append (bytes ([int (result [off + o]) | int (raw [o])]))
        return result [:off] + b"".join (new) + result [off + 2:]
    # end def as_bytes

    def from_bytes (self, bytes, kw) :
        value = struct.unpack (b"!H", bytes [self.offset:self.offset + 2]) [0]
        return self.Value (value, self.kind (bytes))
    # end def from_bytes

    def as_string (self, telegram) :
        return getattr (telegram, self.name)
    # end def as_string

    def from_string (self, telegram, value) :
        return self.Value.From_String (value)
    # end def from_string

# end class _Address_

class Source_Address (_Address_) :
    """The KNX source address"""

    def kind (self, bytes) :
        return False
    # end def kind

# end class Source_Address

class Destination_Address (_Address_) :
    """The KNX destination address"""

    def kind (self, bytes) :
        return (bytes [5] & 0x80) == 0x80
    # end def kind

# end class Destination_Address

class Boolean_Field (Mapping_Field) :
    """A mapping for a boolean field"""

    def __init__ (self, * args, ** kw) :
        kw ["mapping"] = {0 : "n", 1 : "y"}
        self.__super.__init__ (* args, ** kw)
    # end def __init__

# end class Boolean_Field

class M_Sub_Type (M_Object) :
    """Meta class for sub type classes."""

    def __init__ (cls, name, bases, dct) :
        super (M_Sub_Type, cls).__init__ (name, bases, dct)
        Fields = list (getattr (cls, "Fields", ()))
        if not name.startswith ("_") :
            cls.__bases__ [0].Sub_Types [cls.Sub_Type_Id] = cls
            off, shift, mask = bases [0].Match
            mask           >>= shift
            Fields.append \
                (Int_Field (name.lower (), off, shift, mask, cls.Sub_Type_Id))
        cls.fields = {}
        for b in reversed (bases) :
            cls.fields.update (getattr (b, "fields", {}))
        for f in Fields :
            cls.fields [f.name] = f
        for n, f in cls.fields.items () :
            if n not in cls.Defaults :
                cls.Defaults [n] = f.default
    # end def __init__

    def Find_Class (cls, bytes) :
        result = cls
        #import pdb; pdb.set_trace ()
        while "Sub_Types" in result.__dict__ :
            off, shift, mask = result.Match
            if mask < 0xFF :
                type_id      = (bytes [off] >> shift) & mask
            else :
                type_id      = ((bytes [off] << 8) + bytes [off + 1]) & mask
                type_id    >>= shift
            result           = result.Sub_Types [type_id]
        return result
    # end def Find_Class

    def From_Raw (cls, bytes) :
        tcls = cls.Find_Class (bytes)
        kw   = dict ()
        for f in tcls.sorted_fields :
            kw [f.name] = f.from_bytes (bytes, kw)
        return tcls (** kw)
    # end def From_Raw

    @property
    def sorted_fields (cls) :
        return sorted ( cls.fields.values ()
                      , key = lambda f : (f.offset, f.shift)
                      )
    # end def sorted_fields

# end class M_Sub_Type

class _Telegram_ (_Object_, metaclass = M_Sub_Type) :
    """Base class for a EIB telegram"""

    Match     = 0, 6, 0x3
    Sub_Types = {}
    Defaults  = {}

    def __init__ (self, ** kw) :
        for n, f in self.fields.items () :
            v = kw.pop (n, self.Defaults [f.name])
            if isinstance (v, str) :
                v = f.from_string (self, v)
            setattr (self, n, v)
        assert not kw
    # end def __init__

    @property
    def attributes (self) :
        result = {}
        for f in self.__class__.sorted_fields :
            result [f.name] = getattr (self, f.name)
        return result
    # end def attributes

    @property
    def bytes (self) :
        result = bytes ([0x00] * 23)
        for f in self.__class__.sorted_fields :
            result = f.as_bytes (result, getattr (self, f.name))
        size = 7 + self.length
        csum = 0xFF
        for b in result [:size] :
            csum ^= b
        return result [:size] + bytes ((csum, ))
    # end def bytes

    def test_case_entry (self, variable = 0, stepFunction = "NULL") :
        bytes  = ["0x%02X" % b for b in self.bytes [:-1]]
        result = [ "%-15s" % "TEL_RX"
                 , "%d" % len (bytes)
                 , "%2d" % (variable, )
                 , "(StepFunction *) %-17s" % stepFunction
                 , "{%s}" % (", ".join (bytes))
                 ]
        return ", {%s}" % (", ".join (result), )
    # end def test_case_entry

    def __str__ (self) :
        result = []
        for f in self.__class__.sorted_fields :
            if f.important :
                result.append ("%s=%s" % (f.name, f.as_string (self)))
        return "%s (%s)" % (self.__class__.__name__, ",".join (result))
    # end def __str__

# end class _Telegram_

class Data_Request (_Telegram_) :
    """A normal data request telegram"""

    Sub_Type_Id   = 0b10

    Match         = 6, 6, 0x03
    Sub_Types     = {}

    Fields        = \
        ( Int_Field     ( "start_bit",  0, 0, 0x03, 0b00)
        , Mapping_Field ( "priority",   0, 2, 0x03, 0b11, True
                        , mapping = { 0b11 : "l"
                                    , 0b01 : "h"
                                    , 0b10 : "a"
                                    , 0b00 : "s"
                                    }
                        )
        , Int_Field     ( "reserved_0", 0, 4, 0x01,  0b1)
        , Mapping_Field ( "repeat",     0, 5, 0x01,  0b1, True
                        , mapping = { 1 : "n", 0 : "y"}
                        )
        , Int_Field     ( "reserved_1", 0, 6, 0x03, 0b10)

        , Source_Address      ("src", 1)
        , Destination_Address ("dst", 3)
        , Boolean_Field       ("group_address", 5, 7, 0x01, 0b0)
        , Int_Field           ("route",  5, 4, 0x07, 0b110)
        , Int_Field           ("length", 5, 0, 0x0F, 0b0000)
        )

# end class Data_Request

class Extended_Data_Request (_Telegram_) :
    """An extended data request"""
    Sub_Type_Id   = 0b00
# end class Extended_Data_Request

class Poll_Data_Request (_Telegram_) :
    """A poll data request"""
    Sub_Type_Id   = 0b11
# end class Poll_Data_Request

class Unnumbered_Data_Packet (Data_Request) :
    Sub_Type_Id = 0b00

    Match       = 6, 6, 0x03C0
    Sub_Types   = {}

# end class Unnumbered_Data_Packet

class Physical_Address_Set (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0011
# end class Physical_Address_Set

class Physical_Address (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0100
# end class Physical_Address

class Physical_Address_Value (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0101
# end class Physical_Address_Value

class Unnumbered_Data_Packet_4_F (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b1111

    Match       = 7, 0, 0x3F
    Sub_Types   = {}
# end class Unnumbered_Data_Packet_4_F

class Physical_Address_Serial (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b011100
# end class Physical_Address_Serial

class Physical_Address_Serial_Value (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b011110
# end class Physical_Address_Serial_Value

class App_Status (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b011111
# end class App_Status

class System_ID_Set (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b100000
# end class System_ID_Set

class System_ID (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b100001
# end class System_ID

class System_ID_Response (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b100010
# end class System_ID_Response

class System_ID_2 (Unnumbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b100001
# end class System_ID_2

class Get_Value (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0000
# end class Get_Value

class Get_Value_Response (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0001
# end class Get_Value_Response

class Send_Value (Unnumbered_Data_Packet) :
    Sub_Type_Id = 0b0010
    Fields      = \
        ( Int_Field ("value", 7, 0, 0x3F, important = True)
        ,
        )
    Defaults = dict (group_address = 1)
# end class Send_Value

class Unnumbered_Control_Packet (Data_Request) :
    Sub_Type_Id = 0b10
    Match         = 6, 0, 0x03
    Sub_Types   = {}
# end class Unnumbered_Control_Packetet

class Numbered_Data_Packet (Data_Request) :
    Sub_Type_Id = 0b01
    Match       = 6, 6, 0x3C0
    Sub_Types   = {}

    Fields      = (Int_Field ("pno", 6, 2, 0xF, 0b0000, important = True), )
# end class Numbered_Data_Packet

class Numbered_Control_Packet (Data_Request) :
    Sub_Type_Id = 0b11
    Match         = 6, 0, 0x03
    Sub_Types   = {}

    Fields      = (Int_Field ("pno", 6, 2, 0xF, 0b0000, important = True), )
# end class Numbered_Control_Packet

class Connect (Unnumbered_Control_Packet) :
    Sub_Type_Id = 0b00
# end class Connect

class Disconnect (Unnumbered_Control_Packet) :
    Sub_Type_Id = 0b01
# end class Disconnect

class ACK (Numbered_Control_Packet) :
    Sub_Type_Id = 0b10
# end class ACK

class NACK (Numbered_Control_Packet) :
    Sub_Type_Id = 0b11
# end class NACK

class Numbered_Data_Packet_4_F (Numbered_Data_Packet) :
    """First 4 bits of the type are F"""
    Sub_Type_Id = 0b1111

    Match       = 7, 0, 0x3F
    Sub_Types   = {}
# end class Numbered_Data_Packet_4_F

class Property_Read (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010101
# end class Property_Read

class Property_Read_Response (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010110
# end class Property_Read_Response

class Property_Read_Value (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010111
# end class Property_Read_Value

class Property_Desc (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b011000
# end class Property_Desc

class Property_Desc_Response (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b011001
# end class Property_Desc_Response

class Numbered_Data_Packet_4_B (Numbered_Data_Packet) :
    """First 4 bits of the type are B"""
    Sub_Type_Id = 0b1011

    Match       = 7, 0, 0x3F
    Sub_Types   = {}
# end class Numbered_Data_Packet_4_B

class Memory_Read (Numbered_Data_Packet_4_B) :
    Sub_Type_Id = 0b000000
    Fields      = \
        ( Int_Field ("count",   7, 0, 0xF,    important = True)
        , Int_Field ("address", 8, 0, 0xFFFF, important = True)
        )
# end class Memory_Read

class Memory_Read_Response (Numbered_Data_Packet_4_B):
    Sub_Type_Id = 0b000001
# end class Memory_Read_Response

class Memory_Read_Value (Numbered_Data_Packet_4_B) :
    Sub_Type_Id = 0b000010
# end class Memory_Read_Value

class Memory_Read_Value_Bit (Numbered_Data_Packet_4_B) :
    Sub_Type_Id = 0b000011
# end class Memory_Read_Value_Bit

class Manufacturer_Info (Numbered_Data_Packet_4_B) :
    Sub_Type_Id = 0b000100
# end class Manufacturer_Info

class Manufacturer_Info_Response (Numbered_Data_Packet_4_B) :
    Sub_Type_Id = 0b000001
# end class Manufacturer_Info_Response

class ADC_Value (Numbered_Data_Packet) :
    Sub_Type_Id = 0b0110
# end class ADC_Value

class ADC_Value_Response (Numbered_Data_Packet):
    Sub_Type_Id = 0b0111
# end class ADC_Value_Response

class Memory_CC_Read (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1000
    Fields      = \
        ( Int_Field ("count", 7, 0, 0xF, important = True)
        , Int_Field ("address", 8, 0, 0xFFFF, important = True)
        )
# end class Memory_CC_Read

class Memory_CC_Read_Response (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1001
# end class Memory_CC_Read_Response

class Memory_CC_Read_Value (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1010
# end class Memory_CC_Read_Value

class Mask_Read (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1100
# end class Mask_Read

class Mask_Read_Response (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1101
# end class Mask_Read_Response

class Reset (Numbered_Data_Packet) :
    Sub_Type_Id = 0b1110
# end class Reset

class Memory_CC_Read_Value_Bit (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010000
# end class Memory_CC_Read_Value_Bit

class Access (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010001
# end class Access

class Access_Response (Numbered_Data_Packet_4_F):
    Sub_Type_Id = 0b010010
# end class Access_Response

class Access_Set_Key (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010011
# end class Access_Set_Key

class Access_Response (Numbered_Data_Packet_4_F) :
    Sub_Type_Id = 0b010100
# end class Access_Response

if __name__ == "__main__" :
    import sys
    from pprint import pprint

    def info(type, value, tb):
       if hasattr(sys, 'ps1') or not sys.stderr.isatty():
          # we are in interactive mode or we don't have a tty-like
          # device, so we call the default hook
          sys.__excepthook__(type, value, tb)
       else:
          import traceback, pdb
          # we are NOT in interactive mode, print the exception...
          traceback.print_exception(type, value, tb)
          print
          # ...then start the debugger in post-mortem mode.
          pdb.pm()
    sys.excepthook = info

    for raw in ( bytes ((0xBC, 0x01, 0x03, 0x00, 0x01, 0xE1, 0x00, 0x80, 0x21))
               , bytes ((0xBC, 0x01, 0x03, 0x00, 0x01, 0xE1, 0x00, 0x81, 0x20))
               ) :
        t   = _Telegram_.From_Raw (raw)
        print (t)
        print (raw == t.bytes)
        print (", ".join ("0x%02X" % b for b in t.bytes))
        print ("-"*79)
    tc = Send_Value (src = "0.1.3", dst = "0/0/1", value = 1, repeat = "n", length=1)
    print (tc)
    #pprint (t.attributes)
    #pprint (tc.attributes)
    #import pdb; pdb.set_trace ()
    print (", ".join ("0x%02X" % b for b in tc.bytes))
### __END__ Telegram
