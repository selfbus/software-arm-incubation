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
#    Assoc_Table
#
# Purpose
#    The association table of the EEPROM
#
# Revision Dates
#    04-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
from    _Table_   import _Table_

class Assoc_Table (_Table_) :
    """The association table"""

    BCU1 = dict \
        ( ptr   = 0x111
        )

    def __init__ (self, eeprom) :
        self.__super.__init__ (eeprom)
        self._assocs = []
    # end def __init__

    def _build_assocs (self, com_table, addr_table) :
        for obj in com_table :
            if obj.group_address :
                self._assocs.append ((obj.group_address, obj))
        for obj in com_table :
            for grp in obj.rcv_addresses :
                self._assocs.append ((grp,               obj))
    # end def _build_assocs

    def update_eeprom (self, offset) :
        offset       = self.__super.update_eeprom (offset)
        eep          = self.eeprom
        self._build_assocs (eep.com_table, eep.address_table)
        eep [offset] = len (self._assocs)
        offset      += 1
        for grp, com in self._assocs :
            eep [offset    ] = grp.idx
            eep [offset + 1] = com.number
            offset          += 2
        return offset
    # end def update_eeprom

    def as_c_comment (self, bytes = None) :
        return self._as_string (bytes, "    // ")
    # end def as_c_comment

    def _as_string (self, bytes = None, head = "", sep = "\n") :
        if bytes is None :
            bytes = self.eeprom.bytes
        start  = bytes [self.spec ["ptr"] - 0x100]
        result = \
            [ "Assoc Table (0x1%02X):" % (start, )
            ]
        start += 1
        for grp, obj in self._assocs :
            result.append \
                ( "  %2d (%7s) <-> %2d (%-20s) @ 0x1%02X"
                % (grp.idx, grp, obj.number, obj.name, start)
                )
            start += 2
        return sep.join ("%s%s" % (head, l) for l in result)
    # end def _as_string

    def __str__ (self) :
        return self._as_string ()
    # end def __str__

# end class Assoc_Table

### __END__ Assoc_Table
