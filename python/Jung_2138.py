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
import   Test_Case
import   math

class Jung_2138 (EEPROM.BCU1) :
    """The Jung 2138 out 8 device"""

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
            kw ["timed_fct_output_%d"  % no] = F (0x1EA, 1 << i)
            kw ["reaction_to_off_%d"   % no] = F (0x1EB, 1 << i)
            kw ["lock_polarity_%d"     % no] = F (0x1F1, 1 << i)
            kw ["output_kind_%d"       % no] = F (0x1F2, 1 << i)
            kw ["feeback_polarity_%d"  % no] = F (0x1F3, 1 << i)
            kw ["bus_return_action_%d" % no] = F (0x1F6 + quad_inc, quad_mask)
            kw ["delay_base_%d"        % no] = F (0x1F9 + iduo_inc, iduo_mask)
            if i < 4 :
                kw ["special_%d_output"% no] = F (0x1D8 + duo_inc,  duo_mask)
                kw ["special_%d_kind"  % no] = F (0x1ED + quad_inc, quad_mask)
                kw ["special_%d_operator"% no] = F (0x1EE + quad_inc, quad_mask)
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

def Simple (file_name) :
    j = Jung_2138 ()
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.feedback [0].group_address = "1/0/20"
    TCE = Test_Case.Test_Case_Entry
    tc  = Test_Case.Test_Case \
        ( "simple"
        , description = "OUT8 - Test 1"
        , eeprom      = j
        , setup       = "initApplication"
        , state       = "_gatherState"
        , tags        = ("APP", "OUT8", "SIMPLE")
        )
    SV      = Telegram.Send_Value
    fb0_off = SV (src = "1.1.1", dst = "1/0/20", value = 0, length = 1)
    fb0_on  = SV (src = "1.1.1", dst = "1/0/20", value = 1, length = 1)
    tel1    = SV (src = "1.1.1", dst = "1/0/30", value = 1, length = 1)
    tel2    = SV (src = "1.1.1", dst = "1/0/31", value = 0, length = 1)
    tc.add \
        ( TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb0_off
              , comment =
                  "check after the init of the application that the "
                  "feedback object will be sent"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ("CHECK_TX_BUFFER"
              , comment =
                 "only for one feedback channel a group address is store "
                 "-> only one telegram should be sent"
              )

        , TCE ( "TEL_RX", telegram = tel1
              , comment = 'receive a "ON" telegram for output 1 and output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output12Set"
              , comment =
                 ( "process the received telegram inside the app"
                 , "since there is a switch from OFF to ON, the PWM "
                   "timer should be started"
                 )
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 "another app loop is required for the feedback "
                 "telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb0_on
              , comment = "as a reaction to the change a feedback "
                          "telegram should be sent"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment =
                 "only for one feedback channel a group address is store -> "
                 "only one telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")

        , TCE ( "TEL_RX", telegram = tel2
              , comment = 'receive a "OFF" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Clear"
              , comment = "process the received telegram inside the app"
              )
        )
    tc.create_code (file_name)
# end def Simple

def Simple_Inverted (file_name) :
    j = Jung_2138 \
        ( output_kind_1 = 0
        , output_kind_2 = 1
        )
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.feedback [0].group_address = "1/0/20"
    j.feedback [1].group_address = "1/0/21"
    TCE = Test_Case.Test_Case_Entry
    tc  = Test_Case.Test_Case \
        ( "simple_i"
        , description = "OUT8 - Test Inverted"
        , eeprom      = j
        , setup       = "initApplication"
        , state       = "_gatherState"
        , tags        = ("APP", "OUT8", "SIMPLE_I")
        )
    SV      = Telegram.Send_Value
    fb1_off = SV (src = "1.1.1", dst = "1/0/20", value = 0, length = 1)
    fb1_on  = SV (src = "1.1.1", dst = "1/0/20", value = 1, length = 1)
    fb2_off = SV (src = "1.1.1", dst = "1/0/21", value = 0, length = 1)
    fb2_on  = SV (src = "1.1.1", dst = "1/0/21", value = 1, length = 1)
    o1_on   = SV (src = "1.1.1", dst = "1/0/30", value = 1, length = 1)
    o1_off  = SV (src = "1.1.1", dst = "1/0/30", value = 0, length = 1)
    o2_on   = SV (src = "1.1.1", dst = "1/0/31", value = 1, length = 1)
    o2_off  = SV (src = "1.1.1", dst = "1/0/31", value = 0, length = 1)
    tc.add \
        ( TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "feedback output 1"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb2_off
              , comment = "feedback output 2"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only two feedback telegrams"
              )

        , TCE ( "TEL_RX", telegram = o1_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment =
                 "process the received telegram inside the app"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 "another loop is required for the feedback telegram creation"
              )

        , TCE ( "TEL_TX", telegram = fb1_on
              , comment = "feedback telegram ON for output 1"
              )

        , TCE ( "TEL_RX", telegram = o2_on
              , comment = 'receive a "ON" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Clear"
              , comment =
                 "process the received telegram inside the app"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 "another loop is required for the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb2_on
              , comment = "feedback telegram ON for output 1"
              )

        , TCE ( "TEL_RX", telegram = o2_off
              , comment = 'receive a "OFF" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Set"
              , comment =
                 "process the received telegram inside the app"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 "another loop is required for the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb2_off
              , comment = "feedback telegram OFF for output 2"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment =
                 "only for one feedback channel a group address is store -> "
                 "only one telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")
        )
    tc.create_code (file_name)
# end def Simple

def Simple_Timeouts (file_name) :
    j = Jung_2138 \
        ( timed_fct_output_1 = False
        , timed_fct_output_2 = False
        , on_delay_factor_1  = 1
        , off_delay_factor_1 = 2
        , on_delay_factor_2  = 3
        , off_delay_factor_2 = 4
        , delay_base_1       = int (math.log(130/130, 2))
        , delay_base_2       = int (math.log(260/130, 2))
        )
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.feedback [0].group_address = "1/0/20"
    j.feedback [1].group_address = "1/0/21"
    TCE = Test_Case.Test_Case_Entry
    tc  = Test_Case.Test_Case \
        ( "simple_timeout"
        , description = "OUT8 - Timeout Test 1"
        , eeprom      = j
        , setup       = "initApplication"
        , state       = "_gatherState"
        , tags        = ("APP", "OUT8", "DELAY")
        )
    SV      = Telegram.Send_Value
    fb1_off = SV (src = "1.1.1", dst = "1/0/20", value = 0, length = 1)
    fb1_on  = SV (src = "1.1.1", dst = "1/0/20", value = 1, length = 1)
    fb2_off = SV (src = "1.1.1", dst = "1/0/21", value = 0, length = 1)
    fb2_on  = SV (src = "1.1.1", dst = "1/0/21", value = 1, length = 1)
    o1_on   = SV (src = "1.1.1", dst = "1/0/30", value = 1, length = 1)
    o1_off  = SV (src = "1.1.1", dst = "1/0/30", value = 0, length = 1)
    o2_on   = SV (src = "1.1.1", dst = "1/0/31", value = 1, length = 1)
    o2_off  = SV (src = "1.1.1", dst = "1/0/31", value = 0, length = 1)
    tc.add \
        ( TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "init feedback telegram for output 1"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb2_off
              , comment = "and out 2"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only two feedback channel telegrams"
              )
        , TCE ( "TIMER_TICK", length = 20, step = "_enablePWM"
              , comment = "After the power up the PWM will be enabled"
              )
        ### 8
        , TCE ( "TEL_RX", telegram = o1_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*1)*1 on delay -> the output should be "
                   "switch on in 130ms"
                 )
              )
        , TCE ( "TIMER_TICK", length =  60, step = "_loop")
        ### 11
        , TCE ( "TEL_RX", telegram = o2_on
              , comment = 'receive a "ON" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*2)*3 on delay -> the output should be "
                   "switch on in 780ms"
                 )
              )
        , TCE ( "TIMER_TICK", length = 69, step = "_loop")
        ### 130 ms passed since the ON telegram was received
        , TCE ( "TIMER_TICK", length = 1, step = "_output1Set")
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment =
                  "another loop is required for the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_on
              , comment = "feedback telegram for output 1 ON"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only for one feedback channel a group address is store -> only one telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")

        , TCE ("TIMER_TICK", length = 699, step = "_loop")
        ### 130 ms passed since the telegram was received
        , TCE ( "TIMER_TICK", length = 1,   step = "_output2Set")
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment =
                  "another loop is required for the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb2_on
              , comment = "feedback telegram for output 2 ON"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only for one feedback channel a group address is store -> only one telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")

        ### 25
        , TCE ( "TEL_RX", telegram = o1_off
              , comment = 'receive a "OFF" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*1)*2 on delay -> the output should be "
                   "switch off in 260ms"
                 )
              )
        , TCE ( "TIMER_TICK", length =  260, step = "_loop")
        , TCE ( "TIMER_TICK", length =  1,   step = "_output1Clear")
        , TCE ( "TIMER_TICK", length = 1,    step = "_loop"
              , comment =
                  "another loop is required for the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "feedback telegram for output 1 OFF"
              )
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only for one feedback channel a group address is store -> only one telegram should be sent"
              )
        , TCE ( "TIMER_TICK", length = 1,    step = "_loop")
        ### XXX test off delays
        )
    tc.create_code (file_name)
# end def Simple_Timeouts

def Timed_Functions (file_name) :
    j = Jung_2138 \
        ( timed_fct_output_1 = True
        , timed_fct_output_2 = True
        , on_delay_factor_1  = 0
        , off_delay_factor_1 = 1
        , on_delay_factor_2  = 2
        , off_delay_factor_2 = 3
        , delay_base_1       = int (math.log(130/130, 2))
        , delay_base_2       = int (math.log(260/130, 2))
        , reaction_to_off_1  = True
        , reaction_to_off_2  = False
        )
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.feedback [0].group_address = "1/0/20"
    j.feedback [1].group_address = "1/0/21"
    TCE = Test_Case.Test_Case_Entry
    tc  = Test_Case.Test_Case \
        ( "timed_function"
        , description = "OUT8 - Timed Test 1"
        , eeprom      = j
        , setup       = "initApplication"
        , state       = "_gatherState"
        , tags        = ("APP", "OUT8", "TIMED")
        )
    SV      = Telegram.Send_Value
    fb1_off = SV (src = "1.1.1", dst = "1/0/20", value = 0, length = 1)
    fb1_on  = SV (src = "1.1.1", dst = "1/0/20", value = 1, length = 1)
    fb2_off = SV (src = "1.1.1", dst = "1/0/21", value = 0, length = 1)
    fb2_on  = SV (src = "1.1.1", dst = "1/0/21", value = 1, length = 1)
    o1_on   = SV (src = "1.1.1", dst = "1/0/30", value = 1, length = 1)
    o1_off  = SV (src = "1.1.1", dst = "1/0/30", value = 0, length = 1)
    o2_on   = SV (src = "1.1.1", dst = "1/0/31", value = 1, length = 1)
    o2_off  = SV (src = "1.1.1", dst = "1/0/31", value = 0, length = 1)
    tc.add \
        ( TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "init feedback for out 1"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TEL_TX", telegram = fb2_off
              , comment = "and out 2"
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ("CHECK_TX_BUFFER"
              , comment = "only two feedback channel telegrams"
              )
        , TCE ( "TIMER_TICK", length = 20, step = "_enablePWM"
              , comment = "After the power up the PWM will be enabled"
              )
        ### 8
        , TCE ( "TEL_RX", telegram = o1_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*1)*1 on delay -> the output should be "
                   "switch on in 130ms"
                 )
              )
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment = "another app loop is required for the the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_on
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")

        , TCE ( "TIMER_TICK", length = 119, step = "_loop")
        , TCE ( "TIMER_TICK", length = 1, step = "_output1Clear")
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment = "another app loop is required for the the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )

        ### 28
        , TCE ( "TEL_RX", telegram = o2_on
              , comment = 'receive a "ON" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*2)*2 on delay -> the output should be "
                   "switch on in 520ms"
                 )
              )
        , TCE ( "TIMER_TICK", length = 520, step = "_loop"
              , comment = "wait for 520ms before the output will be switched on"
              )
        , TCE ( "TIMER_TICK", length = 1, step = "_output2Set"
              , comment =
                ( "520ms have gone by -> output will be switched on"
                , "and a off delay off (130*2)*3=780ms will be started"
                )
              )
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment = "another app loop is required for the the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb2_on
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")

        , TCE ( "TEL_RX", telegram = o1_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment =
                 ( "process the received telegram inside the app"
                 , "we have a (130*1)*1 on delay -> the output should be "
                   "switch on in 130ms"
                 )
              )
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment = "another app loop is required for the the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_on
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )
        , TCE ("TIMER_TICK", length = 10, step = "_enablePWM")
        ### 29
        , TCE ( "TEL_RX", telegram = o1_off
              , comment = 'receive a "OFF" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "An off telegram has beed received for output 1"
                 )
              )
        , TCE ( "TEL_RX", telegram = o2_off
              , comment = 'receive a "OFF" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Clear"
              , comment =
                 ( "An off telegram has beed received for output 2"
                 )
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment =
                 ( "Another tick to create the feedback telegram"
                 )
              )
        , TCE ( "TEL_TX", telegram = fb2_off
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )
        ### 33
        , TCE ( "TIMER_TICK", length = 116, step = "_loop")
        , TCE ( "TIMER_TICK", length = 1, step = "_output1Clear")
        , TCE ( "TIMER_TICK", length = 1, step = "_loop"
              , comment = "another app loop is required for the the feedback telegram creation"
              )
        , TCE ( "TEL_TX", telegram = fb1_off
              , comment = "as a reaction to the change a feedback telegram should be sent"
              )
        )
    tc.create_code (file_name)
# end def Timed_Functions

def Special_Function_1 (file_name) :
    j = Jung_2138 \
        ( special_1_output    = 1 ## channel 0
        , special_1_kind      = 0 ## logic
        , special_1_operator  = 1 ## or

        , special_2_output    = 2 ## channel 1
        , special_2_kind      = 0 ## logic
        , special_2_operator  = 2 ## and

        , special_3_output    = 3 ## channel 2
        , special_3_kind      = 0 ## logic
        , special_3_operator  = 3 ## and with feedback

        , special_4_output    = 4 ## channel 3
        , special_4_kind      = 2 ## forced output
        )
    j.own_address = "1.1.1"
    j.output   [0].add_to_group ("1/0/30")
    j.output   [1].add_to_group ("1/0/31")
    j.output   [2].add_to_group ("1/0/32")
    j.output   [3].add_to_group ("1/0/33")
    j.special  [0].add_to_group ("1/0/40")
    j.special  [1].add_to_group ("1/0/41")
    j.special  [2].add_to_group ("1/0/42")
    j.special  [3].add_to_group ("1/0/43")
    TCE = Test_Case.Test_Case_Entry
    tc  = Test_Case.Test_Case \
        ( "special_1"
        , description = "OUT8 - Special Function 1"
        , eeprom      = j
        , setup       = "initApplication"
        , state       = "_gatherState"
        , tags        = ("APP", "OUT8", "SEPCIAL")
        )
    SV      = Telegram.Send_Value
    c0_on   = SV (src = "0.0.1", dst = "1/0/30", value = 1, length = 1)
    c1_on   = SV (src = "0.0.1", dst = "1/0/31", value = 1, length = 1)
    c2_on   = SV (src = "0.0.1", dst = "1/0/32", value = 1, length = 1)
    c3_on   = SV (src = "0.0.1", dst = "1/0/33", value = 1, length = 1)
    c0_off  = SV (src = "0.0.1", dst = "1/0/30", value = 0, length = 1)
    c1_off  = SV (src = "0.0.1", dst = "1/0/31", value = 0, length = 1)
    c2_off  = SV (src = "0.0.1", dst = "1/0/32", value = 0, length = 1)
    c3_off  = SV (src = "0.0.1", dst = "1/0/33", value = 0, length = 1)
    s0_on   = SV (src = "0.0.1", dst = "1/0/40", value = 1, length = 1)
    s1_on   = SV (src = "0.0.1", dst = "1/0/41", value = 1, length = 1)
    s2_on   = SV (src = "0.0.1", dst = "1/0/42", value = 1, length = 1)
    s0_off  = SV (src = "0.0.1", dst = "1/0/40", value = 0, length = 1)
    s1_off  = SV (src = "0.0.1", dst = "1/0/41", value = 0, length = 1)
    s2_off  = SV (src = "0.0.1", dst = "1/0/42", value = 0, length = 1)

    s3_zi0  = SV (src = "0.0.1", dst = "1/0/43", value = 0b00, length = 1)
    s3_zi1  = SV (src = "0.0.1", dst = "1/0/43", value = 0b01, length = 1)
    s3_za0  = SV (src = "0.0.1", dst = "1/0/43", value = 0b10, length = 1)
    s3_za1  = SV (src = "0.0.1", dst = "1/0/43", value = 0b11, length = 1)

    tc.add \
        ( TCE ("TIMER_TICK", length = 1, step = "_loop")
        , TCE ( "TIMER_TICK", length = 20, step = "_enablePWM"
              , comment = "After the power up the PWM will be enabled"
              )
        )
    ### test OR
    if 0 :
     tc.add \
        ( TCE ( "TEL_RX", telegram = c0_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = c0_off
              , comment = 'receive a "OFF" telegram for output 1'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Clear"
              , comment = "process the received telegram"
              )

        , TCE ( "TEL_RX", telegram = s0_on
              , comment = 'receive a "ON" telegram for special 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s0_off
              , comment = 'receive a "FF" telegram for special 1'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Clear"
              , comment = "process the received telegram"
              )

        , TCE ( "TEL_RX", telegram = c0_on
              , comment = 'receive a "ON" telegram for output 1'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Set"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s0_on
              , comment = 'receive a "ON" telegram for special 1'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = c0_off
              , comment = 'receive a "OFF" telegram for output 1'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s0_off
              , comment = 'receive a "FF" telegram for special 1'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output1Clear"
              , comment = "process the received telegram"
              )
        )
    ### test AND
    if 0 :
     tc.add \
        ( TCE ( "TEL_RX", telegram = c1_on
              , comment = 'receive a "ON" telegram for output 2'
              , new_line = True
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s1_on
              , comment = 'receive a "ON" telegram for special 2'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Set"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s1_off
              , comment = 'receive a "OFF" telegram for special 2'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Clear"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s1_on
              , comment = 'receive a "ON" telegram for special 2'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Set"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = c1_off
              , comment = 'receive a "OFF" telegram for output 2'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_output2Clear"
              , comment = "process the received telegram"
              )
        , TCE ( "TEL_RX", telegram = s1_off
              , comment = 'receive a "OFF" telegram for special 2'
              )
        , TCE ("TIMER_TICK", length = 1, step = "_loop"
              , comment = "process the received telegram"
              )
        )
    ### test forced follow
    if 1 :
     tc.add \
       ( TCE ( "TEL_RX", telegram = c3_on
             , comment = ( 'Forced channel 4'
                         , 'receive a "ON" telegram for output 4'
                         )
             , new_line = True
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Set"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_zi0
             , comment = 'receive a 00 telegram for special 4'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_loop"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_zi1
             , comment = 'receive a 01 telegram for special 4'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_loop"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = c3_off
             , comment = 'receive a OFF telegram for output 4'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Clear"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = c3_on
             , comment = 'receive a "ON" telegram for output 4'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Set"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_za0
             , comment = 'receive a 10 telegram for special 4'
             , new_line = True
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Clear"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_za1
             , comment = 'receive a 11 telegram for special 4'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Set"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_za0
             , comment = 'receive a 10 telegram for special 4'
             , new_line = True
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Clear"
             , comment = "process the received telegram"
             )
       , TCE ( "TEL_RX", telegram = s3_zi1
             , comment = 'receive a 10 telegram for special 4 to deactivate '
                         'the forced value'
             )
       , TCE ("TIMER_TICK", length = 1, step = "_output4Set"
             , comment = "The output of channel should be high again"
             )
       )
    tc.create_code (file_name)
# end def Special_Function_1

if __name__ == "__main__" :
    import sys
    file_name = None
    if len (sys.argv) > 1 :
        file_name = sys.argv [1]
    Simple              (file_name)
    Simple_Timeouts     (file_name)
    Timed_Functions     (file_name)
    Simple_Inverted     (file_name)
    Special_Function_1  (file_name)
    if 0 :
        j = Jung_2138 ()
        j.own_address = "1.1.1"
        j.output   [0].add_to_group ("1/0/30")
        j.output   [1].add_to_group ("1/0/30")
        j.output   [1].add_to_group ("1/0/31")
        j.feedback [0].group_address = "1/0/20"
        if   1 :
            Telegrams ()
        elif 0 :
            j.test_code_init (sys.argv [1])
        elif 0 :
            j.bytes
            print (j.address_table.as_c_comment ())
        else :
            print (Jung_2138.Description)
### __END__ Jung_2138
