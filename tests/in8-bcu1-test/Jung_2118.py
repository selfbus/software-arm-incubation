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
#    Jung_2118
#
# Purpose
#    Definition of the 8in jung 2118 device
#
# Revision Dates
#    29-Jun-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

import   Device    as     D
from     Com_Table import Com_Object

class Edge_Command_Config (D.Choice_Field) :

    offset         = "i * 4"
    Choices        = { "No Reaction" : 0
                     , "ON"          : 1
                     , "OFF"         : 2
                     , "TOGGLE"      : 3
                     }

# end class Edge_Command_Config

class Dimm_By_Field (D.Choice_Field) :

    offset        = "i * 4"
    Choices       = { "100%"        : 1
                    , "50%"         : 2
                    , "25%"         : 3
                    , "12,5%"       : 4
                    , "6%"          : 5
                    , "3%"          : 6
                    , "1,5%"        : 7
                    }
# end class Dimm_By_Field

class Device (D.BCU1) :
    """Jung 2118 (8in) device"""

    ### default values for standard fields
    manufacturer      = 0x0004
    deviceType        = 0x7054
    version           = 0x02

    bus_return_delay_base         = D.Time_Base_Field \
        (0x1Fe, 0xF0, 3, factor = "bus_return_delay_factor")
    bus_return_delay_factor       = D.Int_Field       (0x1D4, 0x7F, 17)
    debounce_time                 = D.Int_Field       (0x1D2, 0xFF, 10)
    telegram_rate_limiting_enable = D.Int_Field       (0x110, 0x08)
    telegram_rate_limit           = D.Int_Field       (0x1D3, 0xFF, 30)

    class Disabled (D.Channel_Kind) :

        type               = 0

        @classmethod
        def update_com_objects (cls, i, device) :
            no = i + 1
            ct = device.com_table
            ct [i +  0].specialize ("generic %d.1"    % no, "1.001")
            ct [i +  8].specialize ("generic %d.2"    % no, "1.001")
            ct [i + 16].specialize ("generic lock %d" % no, "1.001")
        # end def update_com_objects

    # end class Disabled

    class Switch_Config (D.Channel_Kind) :

        type                = 1

        command_raising_edge_obj_1  = Edge_Command_Config \
            ( address = 0x1D5 + 2, mask = 0x0C)
        command_falling_edge_obj_1  = Edge_Command_Config \
            ( address = 0x1D5 + 2, mask = 0x03)
        command_raising_edge_obj_2  = Edge_Command_Config \
            ( address = 0x1D5 + 2, mask = 0xC0)
        command_falling_edge_obj_2  = Edge_Command_Config \
            ( address = 0x1D5 + 2, mask = 0x30)
        bus_return_action           = D.Choice_Field \
            ( address = 0x1D5,     mask = 0xC0, offset = "i * 4"
            , Choices = { "No Reaction"         : 0
                        , "send current state"  : 1
                        , "send ON"             : 2
                        , "send OFF"            : 3
                        }
            )
        cyclic_send                 = D.Choice_Field \
            ( address = 0x1D5,     mask = 0x0C, offset = "i * 4"
            , Choices = { "No"                   : 0
                        , "repeat on ON"         : 1
                        , "repeat on OFF"        : 2
                        , "repeat on ON and OFF" : 3
                        }
            )
        cyclic_send_time_base_obj_1 = D.Time_Base_Field \
            ( address  = 0x1F6, bit_field_kind = "iduo"
            , default  = 3
            , factor   = "cyclic_send_time_factor"
            )
        cyclic_send_time_base_obj_2 = D.Time_Base_Field \
            ( address  = 0x1FA, bit_field_kind = "iduo"
            , default  = 3
            , factor   = "cyclic_send_time_factor"
            )
        cyclic_send_time_factor     = D.Int_Field \
            ( address  = 0x1D5 + 1, mask = 0x7F, offset = "i * 4"
            , default  = 17
            , limits   = (3, 127)
            )

        locking_polarity            = D.Choice_Field \
            ( address  = 0x1D5 + 0, mask = 0x03, offset = "i * 4"
            , Choices = { "disabled" : 0, "lock=1" : 1, "lock=0" : 2}
            )
        locking_action_at_start     = D.Choice_Field \
            ( address  = 0x1D5 + 0, mask = 0x30, offset = "i * 4"
            , Choices = { "No Reaction" : 0
                        , "send On"     : 1
                        , "send Off"    : 2
                        , "send Toggle" : 3
                        }
            )
        locking_action_at_end       = D.Choice_Field \
            ( address  = 0x1D5 + 3, mask = 0x03, offset = "i * 4"
            , Choices = { "No Reaction"        : 0
                        , "send On"            : 1
                        , "send Off"           : 2
                        , "send current state" : 3
                        }
            )

        @classmethod
        def update_com_objects (cls, i, device) :
            no = i + 1
            ct = device.com_table
            ct [i +  0].specialize ("switch state %d.1" % no, "1.001")
            ct [i +  8].specialize ("switch state %d.2" % no, "1.001")
            ct [i + 16].specialize ("switch lock %d" % no,    "1.001")
        # end def update_com_objects

    # end class Switch_Config

    class Dimmer_Config (D.Channel_Kind) :

        type               = 2

        class _Dimm_Config_Base_ (D.Channel_Kind) :
            time_between_dim_switch_base   = D.Time_Base_Field \
                ( 0x1F6, bit_field_kind = "iduo", default = 0
                , factor = "time_between_dim_switch_factor"
                )
            time_between_dim_switch_factor = D.Int_Field \
                ( 0x1D5 + 2, 0x7F, offset = "i * 4", default = 4
                , limits = (4, 127)
                )
            bus_return_send_on             = D.Int_Field \
                ( 0x1D5 + 2, 0x80, offset = "i * 4")
            bus_return_send_off            = D.Int_Field \
                ( 0x1D5 + 3, 0x80, offset = "i * 4")
            send_stop_telegram             = D.Choice_Field \
                ( 0x1D5 + 0, 0x08, offset = "i * 4"
                , Choices = { "Yes" : 1, "No" : 0}
                )
            telegram_repitition            = D.Choice_Field \
                ( 0x1D5 + 0, 0x04, offset = "i * 4"
                , Choices = {"No" : 0, "Yes" : 1}
                )
            repitition_time_base           = D.Time_Base_Field \
                ( 0x1FA, bit_field_kind = "iduo", default = 6
                , factor = "repitition_time_factor"
                )
            repitition_time_factor         = D.Int_Field \
                ( 0x1D5 + 3, 0x7F, offset = "i * 4", default = 10
                , limits = (3, 127)
                )

            locking_polarity            = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x03, offset = "i * 4"
                , Choices = { "disabled" : 0, "lock=1" : 1, "lock=0" : 2}
                )
            locking_action_at_start     = D.Choice_Field \
                ( address  = 0x1D5 + 1, mask = 0xC0, offset = "i * 4"
                , Choices = { "No Reaction" : 0
                            , "send On"     : 1
                            , "send Off"    : 2
                            , "send Toggle" : 3
                            }
                )
            locking_action_at_end       = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x80, offset = "i * 4"
                , Choices = { "No Reaction"        : 0
                            , "send Off"           : 1
                            }
                )

            @classmethod
            def update_com_objects (cls, i, device) :
                no = i + 1
                ct = device.com_table
                ct [i +  0].specialize ("dim switch %d.1" % no, "1.001")
                ct [i +  8].specialize ("dim %d.2"        % no, "3.007")
                ct [i + 16].specialize ("dim lock %d"     % no, "1.001")
            # end def update_com_objects

        # end class _Dimm_Config_Base_

        class One_Switch (_Dimm_Config_Base_) :
            method                         = 0
            lighten_by                     = Dimm_By_Field (0x1D5 + 1, 0x38)
            darker_by                      = Dimm_By_Field (0x1D5 + 1, 0x03)
        # end class One_Switch

        class Two_Switch_Lighter_On (_Dimm_Config_Base_) :
            method                         = 1
            lighten_by                     = Dimm_By_Field (0x1D5 + 1, 0x38)
        # end class Two_Switch_Lighter_On

        class Two_Switch_Darker_Off (_Dimm_Config_Base_) :
            method                         = 2
            darker_by                      = Dimm_By_Field (0x1D5 + 1, 0x03)
        # end class Two_Switch_Darker_Off

        class Two_Switch_Lighter_Toggle (_Dimm_Config_Base_) :
            method                         = 3
            lighten_by                     = Dimm_By_Field (0x1D5 + 1, 0x38)
        # end class Two_Switch_Lighter_Toggle

        class Two_Switch_Darker_Toggle (_Dimm_Config_Base_) :
            method                         = 4
            darker_by                      = Dimm_By_Field (0x1D5 + 1, 0x03)
        # end class Two_Switch_Darker_Toggle

        method = D.Alternative_Definition \
            ( address = 0x1D5, mask = 0x70, offset = "i * 4", default = 1
            , One              = One_Switch
            , Two_Light_On     = Two_Switch_Lighter_On
            , Two_Dark_Off     = Two_Switch_Darker_Off
            , Two_Light_Toggle = Two_Switch_Lighter_Toggle
            , Two_Dark_Toggle  = Two_Switch_Darker_Toggle
            )
    # end class Dimmer_Config

    class Jalo_Config (D.Channel_Kind) :

        type              = 3

        class _Jalo_Base_ (D.Channel_Kind) :
            command_on_raising_edge  = D.Choice_Field \
                ( address = 0x1D8, mask = 0xF0, offset = "i * 4"
                , Choices =
                    { "no function" : 0
                    , "Up"          : 1
                    , "Down"        : 2
                    , "Toggle"      : 3
                    }
                )
            bus_return_action           = D.Choice_Field \
                ( address = 0x1D5,     mask = 0xC0, offset = "i * 4"
                , Choices = { "No Reaction"         : 0
                            , "Up"                  : 2
                            , "Down"                : 1
                            }
                )
            blade_change_time_base    = D.Time_Base_Field \
                ( 0x1FA, bit_field_kind = "iduo", default = 0
                , factor = "blade_change_time_factor"
                )
            blade_change_time_factor  = D.Int_Field \
                ( 0x1D5 + 2, 0x7F, offset = "i * 4", default = 20
                , limits = (4, 127)
                )

            locking_polarity            = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x03, offset = "i * 4"
                , Choices = { "disabled" : 0, "lock=1" : 1, "lock=0" : 2}
                )
            locking_action_at_start     = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x30, offset = "i * 4"
                , Choices = { "No Reaction" : 0
                            , "Down"        : 1
                            , "Up"          : 2
                            , "Toggle"      : 3
                            }
                )
            locking_action_at_end       = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x0C, offset = "i * 4"
                , Choices = { "No Reaction" : 0
                            , "Down"        : 1
                            , "Up"          : 2
                            , "Toggle"      : 3
                            }
                )

            @classmethod
            def update_com_objects (cls, i, device) :
                no = i + 1
                ct = device.com_table
                ct [i +  0].specialize ("jal short %d" % no, "1.008")
                ct [i +  8].specialize ("jal long  %d" % no, "1.007")
                ct [i + 16].specialize ("jal lock  %d" % no, "1.001")
            # end def update_com_objects

        # end class _Jalo_Base_

        class Short_Long_Short (_Jalo_Base_) :
            method                           = 0
            time_between_short_and_long_base = D.Time_Base_Field \
                ( 0x1F6, bit_field_kind = "iduo", default = 0
                , factor = "time_between_short_and_long_factor"
                )
            time_between_short_and_long_factor = D.Int_Field \
                ( 0x1D5 + 1, 0x7F, offset = "i * 4", default = 4
                , limits = (4, 127)
                )
        # end class Short_Long_Short

        class Long_Short (_Jalo_Base_) :
            method         = 1
        # end class Long_Short

        method = D.Alternative_Definition \
            ( address = 0x1D8, mask = 0x08, offset = "i * 4", default = 0
            , Long_Short       = Long_Short
            , Short_Long_Short = Short_Long_Short
            )
    # end class Jalo_Config

    class Dim_Encoder_Config (D.Channel_Kind) :

        type              = 4

        class _Dim_Encoder_Base_ (D.Channel_Kind) :
            bus_return_action           = D.Choice_Field \
                ( address = 0x1D6,     mask = 0x0C, offset = "i * 4"
                , Choices = { "No Reaction"          : 0
                            , "Same as rising edge"  : 1
                            , "Same as falling edge" : 2
                            , "current state"        : 3
                            }
                )
            locking_polarity            = D.Choice_Field \
                ( address  = 0x1D5 + 0, mask = 0x03, offset = "i * 4"
                , Choices = { "disabled" : 0, "lock=1" : 1, "lock=0" : 2}
                )
            locking_action_at_start     = D.Choice_Field \
                ( address = 0x1D6,     mask = 0x03, offset = "i * 4"
                , Choices = { "No Reaction"         : 0
                            , "Same as rising edge"  : 1
                            , "Same as falling edge" : 2
                            , "current state"        : 3
                            }
                )
            locking_action_at_end       = D.Choice_Field \
                ( address = 0x1D6,     mask = 0x30, offset = "i * 4"
                , Choices = { "No Reaction"         : 0
                            , "Same as rising edge"  : 1
                            , "Same as falling edge" : 2
                            , "current state"        : 3
                            }
                )
        # end class _Dim_Encoder_Base_

        class Send_At_Rising_Edge  (_Dim_Encoder_Base_) :
            send_at            = 0
            value_rising_edge  = D.Int_Field \
                ( 0x1D7, 0xFF, offset = "i * 4", default = 100
                )
            change_via_long_press      = D.Choice_Field \
                ( 0x1D5 + 0, 0x08, offset = "i * 4"
                , Choices = { "Yes" : 0, "No" : 1}, default = 1
                )
            time_between_two_telegrams_base = D.Time_Base_Field \
                ( 0x1F6, bit_field_kind = "iduo", default = 0
                , factor = "time_between_two_telegrams_factor"
                )
            time_between_two_telegrams_factor = D.Int_Field \
                ( 0x1D8, 0x7F, offset = "i * 4", default = 3
                , limits = (3, 127)
                )
            change_step = D.Int_Field \
                ( 0x1D5, 0xF0, offset = "i * 4", default = 10
                )
        # end class Send_At_Rising_Edge

        class Send_At_Rising_Falling  (_Dim_Encoder_Base_) :
            send_at            = 1
            value_falling_edge = D.Int_Field \
                ( 0x1D7, 0xFF, offset = "i * 4", default = 0
                )

            change_via_long_press      = D.Choice_Field \
                ( 0x1D5 + 0, 0x08, offset = "i * 4"
                , Choices = { "Yes" : 0, "No" : 1}, default = 1
                )
            time_between_two_telegrams_base = D.Time_Base_Field \
                ( 0x1F6, bit_field_kind = "iduo", default = 0
                , factor = "time_between_two_telegrams_factor"
                )
            time_between_two_telegrams_factor = D.Int_Field \
                ( 0x1D8, 0x7F, offset = "i * 4", default = 3
                , limits = (3, 127)
                )
            change_step = D.Int_Field \
                ( 0x1D5, 0xF0, offset = "i * 4", default = 10
                )
        # end class Send_At_Rising_Falling

        class Send_At_Rising_and_Falling_Edge  (_Dim_Encoder_Base_) :
            send_at       = 3
            value_rising_edge  = D.Int_Field \
                ( 0x1D7, 0xFF, offset = "i * 4", default = 100
                )
            value_falling_edge = D.Int_Field \
                ( 0x1D8, 0xFF, offset = "i * 4", default = 0
                )
        # end class Send_At_Rising_and_Falling_Edge

        send_at = D.Alternative_Definition \
            ( address = 0x1D6, mask = 0xC0, offset = "i * 4", default = 0
            , Rising  = Send_At_Rising_Edge
            , Falling = Send_At_Rising_Falling
            , Both    = Send_At_Rising_and_Falling_Edge
            )

        @classmethod
        def update_com_objects (cls, i, device) :
            no = i + 1
            ct = device.com_table
            ct [i +  0].specialize ("dimen        %d" % no, "5.001")
            ct [i + 16].specialize ("dimenc lock  %d" % no, "1.001")
        # end def update_com_objects

    # end class Dim_Encoder_Config

    channel = D.Channels_Definition \
        ( "type", 8
        , address = 0x1CE, bit_field_kind = "duo", default = 1
        , Disabled = Disabled
        , Switch   = Switch_Config
        , Dimm     = Dimmer_Config
        , Jalo     = Jalo_Config
        , DimEnc   = Dim_Encoder_Config
        )

    Com_Objects = [  ( ( "generic %d" % (i + 1, ), )
                     , dict (write = False, transmit = True)
                     )
                   for i in range (16)
                  ] + \
                  [  ( ( "generic %d" % (i + 1, ), )
                     , dict (read = False)
                     )
                   for i in range (16, 24)
                  ]
# end class Device

if __name__ == "__main__" :
    #d = Device.from_device_file (r"tests\Jung_2118_switch_config.device")
    #d.eeprom
    #print (d.address_table)
    #print (d.com_table)
    #print (d.assoc_table)
    d = Device ()
    d.own_address = "1.1.1"
    c = d.channel [1]
    c.type = "dimm"
    d.eeprom
    print (d.com_table)
    c.method = "One"
    d.eeprom
    print (d.com_table)
### __END__ Jung_2118
