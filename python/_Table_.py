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
#    _Table_
#
# Purpose
#    Base class for EEPROM tables
#
# Revision Dates
#    04-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
from    _Object_  import _Object_

class _Table_ (_Object_) :
    """Base class for all table objects"""

    def __init__ (self, device) :
        self.device = device
        self.spec   = getattr (self, device.Kind)
    # end def __init__

    def update_eeprom (self, offset) :
        eep = self.device._eeprom
        eep [self.spec ["ptr"]] = offset & 0xFF
        return offset
    # end def update_eeprom

# end class _Table_

### __END__ _Table_
