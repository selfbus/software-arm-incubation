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
#    _Object_
#
# Purpose
#    Base class
#
# Revision Dates
#    03-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

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

class Once_Property (object) :

    def __init__ (self, fct) :
        self.fct = fct
    # end def __init__

    def __get__ (self, obj, cls) :
        if obj is None :
            return self
        result = self.fct (obj)
        setattr (obj, self.fct.__name__, result)
        return result
    # end def __get__

# end class Once_Property

### __END__ _Object_
