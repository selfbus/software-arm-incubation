# -*- coding: utf-8 -*-
# Copyright (C) 2014 Martin Glueck All rights reserved
# Langstrasse 4, A--2244 Spannberg, Austria. martin@mangari.org
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
#    Device
#
# Purpose
#    Base class for the definition of a EIB device
#
# Revision Dates
#    29-Jun-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

from    _Object_        import  M_Object, _Object_
from     Address_Table  import  Address_Table, Address
from     Assoc_Table    import  Assoc_Table
from     Com_Table      import  Com_Object_Table
import   struct
import   configparser
import   re
import   datetime

class Field (_Object_) :
    """Base class for config fields"""

    default        = 0
    bit_field_kind = None
    offset         = 0
    inc            = 0

    def __init__ ( self
                 , address = None
                 , mask    = None
                 , default = None
                 , offset  = None
                 , inc     = None
                 , ** kw
                 ) :
        if address is not None : kw ["address"] = address
        if offset  is not None : kw ["offset"]  = offset
        if mask    is not None : kw ["mask"]    = mask
        if inc     is not None : kw ["inc"]     = inc
        if default is not None : kw ["default"] = default
        self.__dict__.update (kw)
    # end def __init__

    def set_name (self, name) :
        self.name      = name
        self.attr_name = "_%s" % (name, )
    # end def set_name

    def from_string (self, value, obj) :
        return value
    # end def from_string

    def _set_value (self, obj, value) :
        setattr (obj, self.attr_name, value)
    # end def _set_value

    def __get__ (self, obj, cls) :
        if obj is None :
            return self
        return getattr (obj, self.attr_name, self.default)
    # end def __get__

    def __set__ (self, obj, value) :
        if isinstance (value, str) :
            value = self.from_string (value, obj)
        self._set_value (obj, value)
    # end def __set__

    def __delete__ (self, obj) :
        delattr (obj, self.attr_name)
    # end def __delete__

    @classmethod
    def New (cls, address, bit_field_kind = None, ** kw) :
        for name in "address", "offset", "mask", "inc" :
            if kw.get (name, -1) is None :
                del kw [name]
        return cls.__class__ \
            ( cls.__name__, (cls, )
            , dict (kw, address = address, bit_field_kind = bit_field_kind)
            )
    # end def New

    def update_eeprom (self, value, eep,i = 0) :
        vars          = self._setup_mask_and_shift (i)
        value         = (value << self.shift) & self.mask
        address       = self.address
        offset        = self.offset
        if isinstance (offset, str) :
            offset    = eval (offset, {}, vars)
        address      += offset + self.inc
        eep.set_bits (address, struct.pack (self.code, value), self.mask, self)
    # end def update_eeprom

    def _setup_mask_and_shift (self, i) :
        if self.bit_field_kind :
            vars = self._setup_mask_and_offset (self.bit_field_kind, i)
        else :
            vars = dict (i = i, no = i + 1)
        mask = self.mask
        if   mask < 0x100 :
            self.code = "<B"
        elif mask < 0x10000 :
            self.code = "<H"
        else :
            self.code = "<I"
        self.size     = struct.calcsize (self.code)
        shift         = 0
        while not mask & 0x01 :
            mask    >>= 1
            shift    += 1
        self.shift    = shift
        return vars
    # end def _setup_mask_and_shift

    def _setup_mask_and_offset (self, kind, i) :
        no   = i + 1
        if kind   == "bit" :
            self.mask = 1 << i
        elif kind == "duo" :
            self.inc    = i // 2
            self.mask   = 0x0F << (4 * (i % 2))
        elif kind == "iduo" :
            self.inc    = no // 2
            self.mask   = 0x0F << (4 * (no % 2))
        elif kind == "quad" :
            self.inc    = i // 4
            self.mask   = 0x03 << (2 * (i % 4))
    # end def _setup_mask_and_offset

# end class Field

class Int_Field (Field) :
    """Base class for an integer field"""

    limits = None

    def _set_value (self, obj, value) :
        if self.limits :
            if not (self.limits [0] <= value <= self.limits [1]) :
                raise ValueError ("%s not in %s" % (value, self.limits))
        Field._set_value  (self, obj, value)
    # end def _set_value

    def from_string (self, value, obj) :
        return eval (value)
    # end def from_string

# end class Int_Field

class Choice_Field (Field) :
    """A field which has a list of possible values"""

    def __init__ (self, * args, ** kw) :
        Field.__init__ (self, * args, ** kw)
        self.Choices = dict ((k.lower (), v) for k, v in self.Choices.items ())
        self.Seciohc = dict ((v, k) for k, v in self.Choices.items ())
    # end def __init__

    def from_string (self, value, obj) :
        key = value.lower ()
        try :
            return self.Choices [key]
        except KeyError :
            try :
                return self.Choices [key.replace (" ", "_")]
            except KeyError :
                raise KeyError \
                    ( "`%s` not in choices list: `%s`"
                    % (key, "`, `".join (sorted (self.Choices.keys ())))
                    )
    # end def from_string

# end class Choice_Field

class Time_Base_Field (Choice_Field) :
    """Specify the base of a time"""

    Choices = { "130ms"  :  0
              , "260ms"  :  1
              , "520ms"  :  2
              , "1s"     :  3
              , "2,1s"   :  4
              , "4,2s"   :  5
              , "8,4s"   :  6
              , "17s"    :  7
              , "34s"    :  8
              , "1,1min" :  9
              , "2,2min" : 10
              , "4,5min" : 11
              , "9min"   : 12
              , "18min"  : 13
              , "35min"  : 14
              , "1,2h"   : 15
              }

# end class Time_Base_Field

class Channel_Select_Field (Choice_Field) :
    """Special field which changes the config instance"""

    def _set_value (self, obj, new_value) :
        old_value = getattr (obj, self.name)
        if old_value != new_value :
            self.Choice2Cls [old_value].remove_fields (obj)
            self.Choice2Cls [new_value].add_fields    (obj, device = obj.device)
    # end def _set_value

# end class Channel_Select_Field

class Alternative_Select_Field (Choice_Field) :
    """Special field which changes the selected alternative"""

    def _set_value (self, obj, new_value) :
        definition = self.definition
        alt_dict   = definition.alternatives
        old_value  = getattr (obj, self.name)
        alt_dict [old_value].remove_fields (obj)
        alt_dict [new_value].add_fields    (obj, device = obj.device)
        super (Alternative_Select_Field, self)._set_value (obj, new_value)
    # end def _set_value

# end class Alternative_Select_Field

class M_Setup_Fields (_Object_.__class__) :
    """Meta class collecting all fields"""

    def __new__ (mcls, name, bases, dct) :
        dct ["fields"]          = fields = []
        dct ["Channel_Configs"] = cc     = []
        for b in reversed (bases) :
            fields.extend (getattr (b, "fields", ()))
        field_dict = dict ((f.name, f) for f in fields)
        Defaults   = dict ()
        for n, v in list (dct.items ()) :
            if   isinstance (v, Channels_Definition) :
                v.name = n
                cc.append (dct [n])
            elif isinstance (v, Alternative_Definition) :
                field   = v.set_name (n, dct)
                fields.append        (field)
                dct [n] = field
            elif isinstance (v, Field) :
                v.set_name    (n)
                fields.append (v)
            else :
                field = field_dict.get (n)
                if isinstance (field, Field) :
                    Defaults [n] = v
                    dct.pop (n)
        result = super \
            (M_Setup_Fields, mcls).__new__ (mcls, name, bases, dct)
        bDefaults = getattr (result, "Defaults", {})
        bDefaults.update    (Defaults)
        setattr             (result, "Defaults", bDefaults)
        return result
    # end def __new__

# end class M_Setup_Fields

class Alternative_Definition (_Object_) :
    """A set of fields is available depending of a value of another field."""

    def __init__ ( self
                 , address        = None
                 , bit_field_kind = None
                 , offset         = None
                 , mask           = None
                 , inc            = None
                 , default        = None
                 , ** kw
                 ) :
        self.field_cls = Alternative_Select_Field.New \
            ( address        = address
            , mask           = mask
            , default        = default
            , bit_field_kind = bit_field_kind
            , inc            = inc
            , offset         = offset
            )
        self.alternatives    = dict (kw)
    # end def __init__

    def set_name (self, name, cls_dct) :
        self.name = name
        Choices   = dict \
            (  (n, getattr (cls, name))
            for (n, cls) in self.alternatives.items ()
            )
        for n, v in Choices.items () :
            self.alternatives [v] = self.alternatives [n]
        result = self.field_cls (Choices = Choices, definition = self)
        result.set_name         (name)
        fields = cls_dct ["fields"]
        for f in self.alternatives [result.default].fields :
            cls_dct [f.name] = f
            fields.append (f)
        return result
    # end def set_name

# end class Alternative_Definition

class Channels_Definition (_Object_) :
    """Object for configuring the channel types"""

    def __init__ ( self, field_name, no_of_channels
                 , address        = None
                 , bit_field_kind = None
                 , offset         = None
                 , mask           = None
                 , inc            = None
                 , default        = None
                 , ** kw
                 ) :
        self.no_of_channels      = no_of_channels
        Choices                  = dict ()
        Choice2Cls               = dict ()
        for n, cls in kw.items () :
            value                = getattr (cls, field_name)
            Choices    [n]       = value
            Choice2Cls [value]   = cls
        channel_field_cls        = Channel_Select_Field.New \
            ( address            = address
            , bit_field_kind     = bit_field_kind
            , offset             = offset
            , mask               = mask
            , inc                = inc
            , default            = default
            , Choices            = Choices
            , Choice2Cls         = Choice2Cls
            , channel_field_name = field_name
            )
        self.channel_field_cls = channel_field_cls
        for n, cls in kw.items () :
            channel_field = channel_field_cls (default = Choices [n])
            channel_field.set_name (field_name)
            setattr                (cls, field_name, channel_field)
            cls.fields.append      (channel_field)
    # end def __init__

    def initialize (self, parent, device = None) :
        result = Channels (self, parent, device or parent)
        setattr           (parent, self.name, result)
        return result
    # end def initialize

# end class Channels_Definition

class Channels (_Object_) :
    """Channel object"""

    def __init__ (self, definition, parent, device) :
        self.parent           = parent
        self.device           = device
        self.definition       = definition
        self.name             = definition.name
        self._channel_objects = []
        channel_field_cls     = definition.channel_field_cls
        for i in range (definition.no_of_channels) :
            ck = Channel_Kind.New (self.name, i) (definition, parent, device, i)
            ck.activate_kind (channel_field_cls.default)
            self._channel_objects.append (ck)
    # end def __init__

    def __getitem__ (self, key) :
        if key == 0 :
            raise ValueError ("0 not allowed")
        elif key > 0 :
            key -= 1
        return self._channel_objects [key]
    # end def __getitem__

    def update_eeprom (self, value, eep) :
        for co in self._channel_objects :
            co.update_eeprom (eep)
    # end def update_eeprom

# end class Channels

class _Has_Field_Mixin_ (_Object_) :

    def ms (self, time_base_field_name) :
        field = getattr (self.__class__, time_base_field_name)
        tb    = getattr (self,           time_base_field_name)
        tf    = getattr (self,           field.factor)
        return 132 * (2 ** tb) * tf
    # end def ms

# end class _Has_Field_Mixin_

class Channel_Kind (_Has_Field_Mixin_, metaclass = M_Setup_Fields) :
    """Base class for the different kind of a channel"""

    def __init__ (self, definition, parent, device, i, ** kw) :
        self.i          = i
        self.definition = definition
        self.parent     = parent
        self.device     = device
        self.Channels   = \
            [cc.initialize (self, device) for cc in self.Channel_Configs]
    # end def __init__

    def activate_kind (self, new_value) :
        cfc       = self.definition.channel_field_cls
        new_cls   = cfc.Choice2Cls [new_value]
        new_cls.add_fields        (self, device = self.device)
    # end def activate_kind

    @classmethod
    def update_com_objects (cls, i, device) :
        ### called when this cannel kind is activated
        pass
    # end def update_com_objects

    @classmethod
    def add_fields (cls, obj, ** kw) :
        ocls = obj.__class__
        for f in cls.fields :
            setattr                    (ocls,   f.name, f)
            ocls.fields.append         (f)
            default = cls.Defaults.get (f.name, f.default)
            setattr                    (obj,    f.name, default)
        cls.update_com_objects         (obj.i,  ** kw)
    # end def add_fields

    @classmethod
    def remove_fields (cls, obj) :
        ocls = obj.__class__
        for f in cls.fields :
            try :
                delattr (obj, f.name)
            except AttributeError :
                pass ### in case the attribute has not yet been set
            delattr                 (ocls, f.name)
            idx = ocls.fields.index (f)
            del ocls.fields [idx]
    # end def remove_fields

    @classmethod
    def New (cls, name, idx) :
        return type (cls) \
            ("%s_%s_%s" % (cls.__name__, name, idx), (cls, ), dict (i = idx))
    # end def New

    def update_eeprom (self, eep) :
        for f in self.fields :
            value = getattr (self, f.name)
            f.update_eeprom (value, eep, self.i)
        for cc in self.Channels :
            value = getattr  (self, cc.name)
            cc.update_eeprom (value, eep)
    # end def update_eeprom

# end class Channel_Kind

class EEPROM :
    """The EEPROM of an device."""

    def __init__ (self, size, offset = 0x100) :
        self._bytes       = [0x00] * size
        self._set         = []
        for i in range (size) :
            mask = dict ()
            for b in range (8) :
                mask [1 << b] = None
            self._set.append (mask)
        self.offset       = offset
    # end def __init__

    def set_bits (self, address, value, mask, field = True) :
        for o, b in enumerate (value) :
            off       = address + o - self.offset
            m         = mask & 0xFF
            for bit in range (8) :
                bm = 1 << bit
                if bm & m :
                    set_field = self._set [off] [bm]
                    if set_field :
                        name = getattr (set_field, "name", "<unknown>")
                        raise ValueError \
                            ( "A value for %03X:%02X field is already set by "
                              "%s and now try to set by %s"
                            % (off, m, name, getattr (field, "name", "<unknown>"))
                            )
                    else :
                        self._set [off] [bm] = field
            self._bytes [off] |= b
            mask >>= 8
    # end def set_bits

    def _fix_key (self, key) :
        if isinstance (key, slice) :
            return slice (key.start - 0x100, key.stop - 0x100, key.step)
        return key - 0x100
    # end def _fix_key

    def __getitem__ (self, key) :
        return self._bytes [self._fix_key (key)]
    # end def __getitem__

    def __setitem__ (self, key, value) :
        ckey = self._fix_key (key)
        if isinstance (ckey, slice) :
            raise TypeError ("Slice assignment not supported !")
        for b in range (8) :
            bm        = 1 << b
            set_field = self._set [ckey] [bm]
            if set_field :
                name = getattr (set_field, "name", "<unknown>")
                raise ValueError \
                    ( "A value for %03X:%02X field is already set by "
                      "%s and now try to set by %s"
                    % (off, m, name, "<unknown>")
                    )
            else :
                self._set [ckey] [bm] = True
        self._bytes [ckey] = value
    # end def __setitem__

    def __str__ (self) :
        bytes    = self._bytes
        result   = ["   : %s" % (" ".join ("%02X" % i for i in range (16)), )]
        address  = 0x100
        while bytes :
            line = ["%03X:" % (address, )]
            for i in range (16) :
                line.append ("%02X" % (bytes.pop (0), ))
                address += 1
            result.append (" ".join (line))
        return "\n".join (result)
    # end def __str__

# end class EEPROM

class _BCU_ (_Has_Field_Mixin_, metaclass = M_Setup_Fields) :
    """Base class for a device."""

    manuData      = Int_Field (0x101, 0xFFFF)
    manufacturer  = Int_Field (0x103, 0xFFFF)
    deviceType    = Int_Field (0x105, 0xFFFF)
    version       = Int_Field (0x107, 0xFF)

    def __init__ (self, ** kw) :
        for a in "Address_Table", "Assoc_Table", "Com_Table" :
            cls = getattr (self, a, None)
            if cls :
                setattr (self, a.lower (), cls (self))
        self.com_object = self.com_table
        for args, kw in self.Com_Objects :
            self.com_table.add (* args, ** kw)
        self.Channels = [cc.initialize (self) for cc in self.Channel_Configs]
        for f in self.fields :
            default = self.Defaults.get (f.name, f.default)
            setattr                     (self, f.name, kw.pop (f.name, default))
    # end def __init__

    @property
    def eeprom (self) :
        self._eeprom = result = EEPROM (self.Size)
        for f in self.fields :
            value = getattr (self, f.name)
            f.update_eeprom (value, result)
        for cc in self.Channels :
            value = getattr  (self, cc.name)
            cc.update_eeprom (value, result)
        offset = 0
        for a in "address_table", "com_table", "assoc_table" :
            table = getattr (self, a, None)
            if table :
                offset = table.update_eeprom (offset)
        return result
    # end def eeprom

    @property
    def own_address (self) :
        return self.address_table.own
    # end def own_address

    @own_address.setter
    def own_address (self, value) :
        self.address_table.own = value
    # end def own_address

    name_spit_pat = re.compile ("[ .,]+")
    @classmethod
    def from_device_file (cls, file_name) :
        cp     = configparser.RawConfigParser ()
        device = None
        if not cp.read (file_name) :
            raise ValueError ("Could not parse %s" % (file_name, ))
        print ("Test-Case specification %s read" % (file_name, ))
        for s in cp.sections () :
            if s.lower ().replace (" ", "") == "groupaddresses" :
                ct = device.com_table
                for com_no, addresses in cp.items (s) :
                    ct [int (com_no)].add_to_group \
                         (* cls.name_spit_pat.split (addresses))
            else :
                if s.startswith ("Device") :
                    device_type  = s.rsplit ("=", 1) [-1]
                    Device_Class = __import__   (device_type).Device
                    device       = Device_Class ()
                    obj          = device
                else :
                    parts        = cls.name_spit_pat.split (s.lower ())
                    obj          = device
                    while parts :
                        p        = parts.pop (0)
                        if p.isdigit () :
                            obj  = obj [int (p)]
                        else :
                            obj  = getattr (obj, p)
                for opt, value in cp.items (s) :
                    ### print (obj, opt, value)
                    try :
                        setattr (obj, opt, value)
                    except KeyError :
                        raise
                    except Exception as exc :
                        raise ValueError \
                             ( "Cannot set field %s.%s to %s\n[%s]"
                             % (s, opt, value, exc)
                             )
        return device
    # end def from_device_file

    def _replace_in_file (self, content, section, new) :
        pat = re.compile \
            ( "^\s*// >>> %s(.)+// <<< %s" % (section, section)
            , re.MULTILINE | re.DOTALL
            )
        return pat.sub (new, content)
    # end def _replace_in_file

    def test_code_init (self, file_name = None, limit = 1024*1024, file = None) :
        result = [ "    // >>> EEPROM INIT"
                 , "    // Date: %s" % (datetime.datetime.now (), )
                 ]
        eep = self.eeprom
        result.append (self.assoc_table.  as_c_comment (eep))
        result.append (self.address_table.as_c_comment (eep))
        result.append (self.com_table.    as_c_comment (eep))
        for o, b in enumerate (eep [0x100:0x100 + limit]) :
            result.append ("    userEeprom[0x1%02X] = 0x%02X;" % (o, b))
        result.append ("    // <<< EEPROM INIT")
        result = "\n".join (result)
        if not file_name :
            if not file :
                print (result)
            else :
                file.write (result)
                file.write ("\n")
        else :
            with open (file_name, "r") as f :
                old = f.read ()
                new = self._replace_in_file (old, "EEPROM INIT", result)
            with open (file_name, "w") as f :
                f.write (new)
                print ("Updated %s" % (file_name))
    # end def test_code_init

# end class _BCU_

class BCU1 (_BCU_) :
    """Base class for a BCU1 based device"""

    Size          = 256
    Kind          = "BCU1"

    Address_Table = Address_Table
    Assoc_Table   = Assoc_Table
    Com_Table     = Com_Object_Table

# end class BCU1

### __END__ Device_New
