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
#    EEPROM
#
# Purpose
#
# Revision Dates
#     2-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__       import division, print_function
from   __future__       import absolute_import, unicode_literals
from    _Object_        import _Object_, M_Object
from    _Table_         import _Table_
from     Address_Table  import  Address_Table, Address
from     Assoc_Table    import  Assoc_Table
from     Com_Table      import  Com_Object_Table
import   struct
import   re
import   datetime

class Field (_Object_) :
    """Base class for a field inside the EEPROM"""

    name = "<unnamed>"

    def __init__ (self, offset, mask = 0xFF, default = 0) :
        self.offset   = offset
        self.mask     = mask
        self.default  = 0
        if   mask < 0x100 :
            self.code = "<B"
        elif mask < 0x10000 :
            self.code = "<H"
        else :
            self.code = "<I"
        self.size     = struct.calcsize (self.code)
        ### determine the shift value
        shift         = 0
        while not mask & 0x01 :
            mask    >>= 1
            shift    += 1
        self.shift    = shift
    # end def __init__

    def set_name (self, name) :
        self.name      = name
    # end def set_name

    def __get__ (self, obj, cls) :
        if obj is None :
            return self
        raw    = obj [self.offset : self.offset + self.size]
        result = struct.unpack (self.code, bytes (raw)) [0]
        return (result & self.mask) >> self.shift
    # end def __get__

    def __set__ (self, obj, value) :
        value = (value << self.shift) & self.mask
        for o, b in enumerate (struct.pack (self.code, value)) :
            obj [self.offset + o] |= b
    # end def __set__

    def __repr__ (self) :
        return "<%s:0x%03X:0x%0*X>" % \
            (self.name, self.offset, self.size * 2, self.mask)
    # end def __repr__

# end class Field

class _M_EEPROM_ (M_Object) :
    """Meta class for the definition of a new EEPROM layout"""

    def __init__ (cls, name, bases, dct) :
        super (_M_EEPROM_, cls).__init__ (name, bases, dct)
        cls._fields  = []
        cls.Defaults = dict ()
        for base in reversed (bases) :
            cls._fields .extend (getattr (base, "_fields",  ()))
            cls.Defaults.update (getattr (base, "Defaults", {}))
        cls.Update (dct)
        cls.Name         = name
        cls.Table [name] = cls
    # end def __init__

    def Update (cls, kw) :
        fields   = cls._fields
        Defaults = cls.Defaults
        cls._add_fields (kw)
        for n, v in kw.items () :
            if isinstance (v, Field) :
                fields.append (v)
                v.set_name    (n)
                Defaults [n] = v.default
                setattr (cls, n, v)
            else :
                f = getattr (cls, n, None)
                if isinstance (f, Field) :
                    ### change the default for this field for this class
                    Defaults [n] = v
        fields.sort (key     = lambda f : (f.offset, f.shift))
    # end def Update

# end class _M_EEPROM_

class EEPROM (_Object_, metaclass = _M_EEPROM_) :
    """The EEPROM layout of an EIB application"""

    Table = {}

    def __init__ (self, ** kw) :
        for a in "Address_Table", "Assoc_Table", "Com_Table" :
            cls = getattr (self, a, None)
            if cls :
                setattr (self, a.lower (), cls (self))
        self._bytes = [0x00] * self.Size
        for f in self._fields :
            setattr (self, f.name, kw.pop (f.name, self.Defaults [f.name]))
        assert not kw, kw
        self._create_com_objects ()
    # end def __init__

    def add_com_object (self, * args, ** kw) :
        Kind = kw.pop             ("Kind", None)
        obj  = self.com_table.add (* args, ** kw)
        if Kind :
            l = getattr (self, Kind, [])
            l.append    (obj)
            setattr     (self, Kind, l)
    # end def add_com_object

    @classmethod
    def _add_fields (self, kw) :
        pass
    # end def _add_fields

    def _create_com_objects (self) :
        pass
    # end def _create_com_objects

    @property
    def bytes (self) :
        offset = 0
        for a in "address_table", "com_table", "assoc_table" :
            table = getattr (self, a, None)
            if table :
                offset = table.update_eeprom (offset)
        return self._bytes
    # end def bytes

    @property
    def own_address () :
        return self.address_table.own
    # end def own_address

    @own_address.setter
    def own_address (self, value) :
        self.address_table.own = value
    # end def own_address

    def test_code_init (self, file_name = None, limit = None) :
        result = [ "    // >>> EEPROM INIT"
                 , "    // Date: %s" % (datetime.datetime.now (), )
                 ]
        bytes = self.bytes
        result.append (self.assoc_table.  as_c_comment (bytes))
        result.append (self.address_table.as_c_comment (bytes))
        result.append (self.com_table.    as_c_comment (bytes))
        for o, b in enumerate (bytes [:limit]) :
            result.append ("    userEeprom[0x1%02X] = 0x%02X;" % (o, b))
        result.append ("    // <<< EEPROM INIT")
        result = "\n".join (result)
        if not file_name :
            print (result)
        else :
            with open (file_name, "r") as f :
                old = f.read ()
                new = self._replace_in_file (old, "EEPROM INIT", result)
            with open (file_name, "w") as f :
                f.write (new)
                print ("Updated %s" % (file_name))
    # end def test_code_init

    def _replace_in_file (self, content, section, new) :
        pat = re.compile \
            ( "^\s*// >>> %s(.)+// <<< %s" % (section, section)
            , re.MULTILINE | re.DOTALL
            )
        return pat.sub (new, content)
    # end def _replace_in_file

    def _fix_key (self, key) :
        if isinstance (key, slice) :
            return slice (key.start - 0x100, key.stop - 0x100, key.step)
        return key - 0x100
    # end def _fix_key

    def __getitem__ (self, key) :
        return self._bytes [self._fix_key (key)]
    # end def __getitem__

    def __setitem__ (self, key, value) :
        self._bytes [self._fix_key (key)] = value
    # end def __setitem__

# end class EEPROM

class BCU (EEPROM) :
    """Base class for all BCU types"""

    optionReg     = Field (0x100, 0xFF)
    manuData      = Field (0x101, 0xFFFF)
    manufacturer  = Field (0x103, 0xFFFF)
    deviceType    = Field (0x105, 0xFFFF)
    version       = Field (0x107, 0xFF)
    checkLimit    = Field (0x108, 0xFF)
    appPeiType    = Field (0x109, 0xFF)
    syncRate      = Field (0x10A, 0xFF)
    portCDDR      = Field (0x10B, 0xFF)
    portADDR      = Field (0x10C, 0xFF)
    runError      = Field (0x10D, 0xFF)
    routeCnt      = Field (0x10E, 0xFF)
    maxRetransmit = Field (0x10F, 0xFF)
    confDesc      = Field (0x110, 0xFF)
    usrInitPtr    = Field (0x113, 0xFF)
    usrProgPtr    = Field (0x114, 0xFF)

# end class BCU

class BCU1 (BCU) :
    """BCU 1.x versions"""

    Address_Table = Address_Table
    Assoc_Table   = Assoc_Table
    Com_Table     = Com_Object_Table
    Size          = 256
    Kind          = "BCU1"
    usrProgPtr    = Field (0x114, 0xFF)
    usrSavePtr    = Field (0x115, 0xFF)
    addrTabSize   = Field (0x116, 0xFF)

# end class BCU1

### __END__ EEPROM
