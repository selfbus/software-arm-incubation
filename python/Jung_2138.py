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
#    Jung_2138
#
# Purpose
#    The EEPROM layout for the Jung 2138 out8 device
#
# Revision Dates
#    03-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
import   EEPROM
import   Com_Table as CT
import   Telegram

class Jung_2138 (EEPROM.BCU1) :
    """The Junbf 2138 out 8 device"""

    ### default values for fields
    manufacturer     = 0x0004
    deviceType       = 0x2060
    version          = 0x01

    ### new fields
    special_1_lock_on_action  = EEPROM.Field (0x1EF, 0x03)
    special_1_lock_off_action = EEPROM.Field (0x1EF, 0x0C)
    special_2_lock_on_action  = EEPROM.Field (0x1EF, 0x30)
    special_2_lock_off_action = EEPROM.Field (0x1EF, 0xC0)
    special_3_lock_on_action  = EEPROM.Field (0x1F0, 0x03)
    special_3_lock_off_action = EEPROM.Field (0x1F0, 0x0C)
    special_4_lock_on_action  = EEPROM.Field (0x1F0, 0x30)
    special_4_lock_off_action = EEPROM.Field (0x1F0, 0xC0)

    @classmethod
    def _add_fields (cls, kw) :
        F = EEPROM.Field
        for i in range (8) :
            no          = i + 1
            mask        = 1 << i
            duo_inc     = i // 2
            duo_mask    = 0x0F << (4 * (i % 2))
            quad_inc    = i // 4
            quad_mask   = 0x03 << (2 * (i % 4))

            iduo_inc    = no // 2
            iduo_mask   = 0x0F << (4 * (no % 2))

            kw ["on_delay_factor_%d"   % no] = F (0x1DA + i, 0xFF)
            kw ["off_delay_factor_%d"  % no] = F (0x1E2 + i, 0xFF)
            kw ["timed_fct_output_%d"  % no] = F (0x1DA, 1 << i)
            kw ["lock_polarity_%d"     % no] = F (0x1F1, 1 << i)
            kw ["output_kind_%d"       % no] = F (0x1F2, 1 << i)
            kw ["feeback_polarity_%d"  % no] = F (0x1F3, 1 << i)
            kw ["bus_return_action_%d" % no] = F (0x1F6 + quad_inc, quad_mask)
            kw ["delay_base_%d"        % no] = F (0x1F9 + iduo_inc, iduo_mask)

            kw ["special_%d_output"    % no] = F (0x1D8 + duo_inc,  duo_mask)
            kw ["special_%d_kind"      % no] = F (0x1ED + quad_inc, quad_mask)
            kw ["special_%d_operator"  % no] = F (0x1EE + quad_inc, quad_mask)
    # end def _add_fields

    def _create_com_objects (self) :
        add = self.add_com_object
        for i in range (8) : ### 0..7
            add (CT.CO_WO_UINT1 ("output %d"   % (i+1)), Kind = "output")
        for i in range (4) : ### 8..B
            add (CT.CO_UINT1    ("special %d"  % (i+1)), Kind = "special")
        for i in range (8) : ### C..13
            add (CT.CO_RO_UINT1 ("feedback %d" % (i+1)), Kind = "feedback")
    # end def _create_com_objects

# end class Jung_2138

def Telegrams (file_name = None) :
    result = []
    SV     = Telegram.Send_Value
    a      = result.append
    a ( SV (src = "1.1.1", dst = "1/0/30", value = 1, length = 1
           ).test_case_entry ()
      )
    a ( SV (src = "1.1.1", dst = "1/0/31", value = 0, length = 1
           ).test_case_entry ()
      )
    if not file_name :
        for e in result :
            print (e)
# end def Telegrams

if __name__ == "__main__" :
    import sys
    j = Jung_2138 ()
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.feedback [0].group_address = "1/0/20"
    if 1 :
        Telegrams ()
    elif 1 :
        j.test_code_init (sys.argv [1])
    else :
        j.bytes
        print (j.address_table.as_c_comment ())
### __END__ Jung_2138
