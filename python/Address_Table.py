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
#    Address_Table
#
# Purpose
#    The address table inside the EEPROM
#
# Revision Dates
#    04-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
from    _Object_  import _Object_
from    _Table_   import _Table_
import   Telegram
import   struct

class Address (Telegram._Address_.Value) :
    """Model a group/physical address"""

    sort_key = 0

    def __init__ (self, * args, ** kw) :
        self.__super.__init__ (* args, ** kw)
        self.ref_count = 0
    # end def __init__

    def add_address (self, eep, offset) :
        for b in struct.pack (">H", self._value) :
            eep [offset] |= b
            offset += 1
        return offset
    # end def add_address

    @classmethod
    def Convert (cls, value) :
        if isinstance (value, str) :
            return cls.From_String (value)
        elif not isinstance (value, cls) :
            return cls (int (value))
        return value
    # end def Convert

    def __hash__ (self) :
        return self._value
    # end def __hash__

    def __eq__ (self, rhs) :
        return self._value == rhs._value
    # end def __eq__

    def __lt__ (self, rhs) :
        return self._value < rhs._value
    # end def __lt__

# end class Address

class Address_Table_Dict (dict) :

    def __getitem__ (self, key) :
        key = Address.Convert (key)
        return super (Address_Table_Dict, self).__getitem__ (key)
    # end def __getitem__

# end class Address_Table_Dict

class Address_Table (_Table_) :
    """The group address table"""

    BCU1 = dict \
        ( size  = 0x116
        , table = 0x117
        )
    _own = None

    def __init__ (self, eeprom) :
        self.__super.__init__ (eeprom)
        self.addresses = Address_Table_Dict ()
    # end def __init__

    def add (self, value) :
        result = Address.Convert (value)
        if result not in self.addresses :
            self.addresses [result] = result
        else :
            result                  = self.addresses [result]
        result.ref_count += 1
        return result
    # end def add

    def remove (self, value) :
        result = Address.Convert (value)
        if result in self.addresses :
            result = self.addresses [result]
            result.ref_count -= 1
            if result.ref_count <= 0:
                del self.addresses [result]
    # end def remove

    @property
    def own (self) :
        return self._own
    # end def own

    @own.setter
    def own (self, value) :
        self._own = Address.Convert (value)
    # end def own

    def update_eeprom (self, offset) :
        eep                      = self.eeprom
        offset                   = self.spec ["table"]
        eep [self.spec ["size"]] = len (self.addresses)
        offset                   = self.own.add_address (eep, offset)
        for i, a in enumerate ( sorted ( self.addresses
                                       , key = lambda a : (a.sort_key, int (a))
                                       )
                              ) :
            offset = a.add_address (eep, offset)
            a.idx  = i + 1
        return offset
    # end def update_eeprom

    def as_c_comment (self, bytes = None) :
        return self._as_string (bytes, "    // ")
    # end def as_c_comment

    def _as_string (self, bytes = None, head = "", sep = "\n") :
        if bytes is None :
            bytes = self.eeprom.bytes
        start  = self.spec ["size"] - 0x100
        result = \
            [ "Address Table (0x1%02X):" % (start, )
            , "  (%2d) %7s @ 0x1%02X" % (0, self.own, start + 1)
            ]
        start += 1
        for a in sorted (self.addresses, key = lambda a : a.idx) :
            start += 2
            result.append ("  (%2d) %7s @ 0x1%02X" % (a.idx, a, start))
        return sep.join ("%s%s" % (head, l) for l in result)
    # end def _as_string

    def __str__ (self) :
        return self._as_string ()
    # end def __str__

# end class Address_Table

### __END__ Address_Table
