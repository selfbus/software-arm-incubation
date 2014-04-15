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
    DATA6   = 11,  6
    DOUBLE  = 12,  8
    DATA10  = 13, 10
    MAXDATA = 14, 14
    VARDATA = 15, 15

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
        assert not kw, kw
    # end def __init__

    @property
    def group_address (self) :
        return self._grp_addr
    # end def snd_address

    @group_address.setter
    def group_address (self, value) :
        if self._grp_addr :
            self.eeprom.address_table.remove           (self._grp_addr)
        self._grp_addr = self.eeprom.address_table.add (value)
    # end def snd_address

    def add_to_group (self, * grp_addresses) :
        for g in grp_addresses :
            g = self.eeprom.address_table.add (g)
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
    transmit     = Bit_Mask_Flag (0b01000000)
    value_in_eep = Bit_Mask_Flag (0b00100000)
    write        = Bit_Mask_Flag (0b00010000)
    read         = Bit_Mask_Flag (0b00001000)
    comm         = Bit_Mask_Flag (0b00000100)

    @property
    def priority (self) :
        return self._a_priority
    # end def priority

    @priority.setter
    def priority (self, value) :
        self._a_priority = value & 0x03
    # end def setter

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

    def __init__ (self, eeprom) :
        self.__super.__init__ (eeprom)
        self._objects = []
    # end def __init__

    def add (self, obj, * args, ** kw) :
        if not isinstance (obj, Com_Object) :
            obj    = Com_Object (obj, * args, ** kw)
        obj.eeprom = self.eeprom
        obj.number = len (self._objects)
        self._objects.append (obj)
        return obj
    # end def add

    def update_eeprom (self, offset) :
        offset       = self.__super.update_eeprom (offset)
        eep          = self.eeprom
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
            bytes = self.eeprom.bytes
        result = \
            [ "Com Object table (0x1%02X):"
            % (bytes [self.spec ["ptr"] - 0x100], )
            ]
        for obj in self._objects :
            result.append \
                ( "  (%2d) %-20s <%02X, %02X, %02X> @ 0x%03X"
                % (obj.number
                  , obj.name
                  , obj.ram_off
                  , obj.flags
                  , obj.type_code
                  , obj.eep_off
                  )
                )
        return sep.join ("%s%s" % (head, l) for l in result)
    # end def _as_string

    def __str__ (self) :
        return self._as_string ()
    # end def __str__

# end class Com_Object_Table

### __END__ Com_Table»
