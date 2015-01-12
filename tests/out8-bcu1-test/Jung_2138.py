# -*- coding: utf-8 -*-
# Copyright (C) 2015 Martin Glueck All rights reserved
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
#    Jung_2138
#
# Purpose
#    Definition of the 8out jung 2138 device
#
# Revision Dates
#    11-Jan-2015 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

import   Device    as     D
from     Com_Table import Com_Object

TBF = D.Time_Base_Field
TFF = D.Time_Factor_Field
INF = D.Int_Field

class Device (D.BCU1) :
    """Jung 2138 (8out) device"""

    ### default values for standard fields
    manufacturer    = 0x0004
    deviceType      = 0x2060
    version         = 1

    class Output (D.Channel) :
        """Fields for the output configuration"""

        delay_base        = TBF (0x1F9, bit_field_kind = "iduo")
        on_delay_factor   = TFF (0x1DA, 0xFF, offset = "i", base = "delay_base")
        off_delay_factor  = TFF (0x1E2, 0xFF, offset = "i", base = "delay_base")
        timed_fct_active  = INF (0x1EA, bit_field_kind = "bit")
        reaction_to_off   = D.Choice_Field \
            ( 0x1EB, bit_field_kind = "bit"
            , Choices = { "ignore" : 0, "switch off" : 1}
            )
        lock_polarity     = D.Choice_Field \
            ( 0x1F1, bit_field_kind = "bit"
            , Choices = { "lock=1" : 0, "lock=0" : 1}
            )
        output_polarity   = D.Choice_Field \
            ( 0x1F2, bit_field_kind = "bit"
            , Choices = { "normally open" : 0, "normally closed" : 1}
            )
        feedback_polarity = D.Choice_Field \
            ( 0x1F3, bit_field_kind = "bit"
            , Choices = { "normal" : 0, "inverted" : 1}
            )
        bus_return_action = D.Choice_Field \
            ( 0x1F4, bit_field_kind = "quad"
            , Choices = { "no reaction"   : 0
                        , "open contact"  : 1
                        , "close contact" : 2
                        }
            )

    # end class Output

    class Special_Function (D.Channel) :
        """The special function channel"""

        lock_on_action       = D.Choice_Field \
            ( 0x1EF
            , bit_field_kind = "duo_quad_l"
            , Choices        = {"no action" : 0, "close" : 1, "open" : 2}
            )
        lock_off_action      = D.Choice_Field  \
            ( 0x1EF
            , bit_field_kind = "duo_quad_h"
            , Choices        = {"no action" : 0, "close" : 1, "open" : 2}
            )
        output               = D.Choice_Field \
            ( 0x1D8
            , bit_field_kind = "duo"
            , Choices        = { "disabled"  : 0
                               , "channel 1" : 1
                               , "channel 2" : 2
                               , "channel 3" : 3
                               , "channel 4" : 4
                               , "channel 5" : 5
                               , "channel 6" : 6
                               , "channel 7" : 7
                               , "channel 8" : 8
                               }
            )
        kind                 = D.Choice_Field \
            ( 0x1ED
            , bit_field_kind = "quad"
            , Choices        = {"logic" : 0, "lock" : 1, "force" : 2}
            )
        operator             = D.Choice_Field \
            ( 0x1EE
            , bit_field_kind = "quad"
            , Choices        = { "undefined"              : 0
                               , "or"                     : 1
                               , "and"                    : 2
                               , "and with recirculation" : 3
                               }
            )

    # end class Special_Function

    channel = D.Channels_Definition (Output,           8)
    special = D.Channels_Definition (Special_Function, 4)

    Com_Objects = [  ( ( "output %d" % (i + 1, ), )
                     , dict (write = True)
                     )
                   for i in range (8)
                  ] + \
                  [  ( ( "special %d" % (i - 7, ), )
                     , dict ()
                     )
                   for i in range (8, 12)
                  ] + \
                  [  ( ( "feedback %d" % (i - 11, ), )
                     , dict (read = False, transmit = True)
                     )
                   for i in range (12, 20)
                  ]
# end class Device

if __name__ == "__main__" :
    d = Device ()
    d.own_address = "1.1.1"
    d.special [1].lock_on_action  = 3
    d.special [1].lock_off_action = 3
    d.special [2].lock_on_action  = 2
    d.special [2].lock_off_action = 2
    d.special [3].lock_on_action  = 1
    d.special [3].lock_off_action = 1
    d.special [4].lock_on_action = 0
    print (d.eeprom)
### __END__ Jung_2138
