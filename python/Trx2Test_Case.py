# -*- coding: iso-8859-15 -*-
# Copyright (C) 2013 Martin Glueck All rights reserved
# Langstrasse 4, A--2244 Spannberg, Austria. martin@mangari.org
# #*** <License> ************************************************************#
# This script is part of the library selfbus.
#
# This script is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this script. If not, see <http://www.gnu.org/licenses/>.
# #*** </License> ***********************************************************#
#
#++
# Name
#    Trx2Test_Case
#
# Purpose
#    Convert the traces of the ETS into a test cases for the selfbus protocol
#    implementation
#
# Revision Dates
#    19-Dec-2013 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

import   datetime
import   os

class _Record_ (object) :
    """Base class for a record of a trx file"""

    response_count = 0
    check_function = "NULL"

    def __init__ (self, bytes, time = None) :
        self.bytes = bytes
        self.time  = time
        #print (self)
    # end def __init__

    def test_case_entry (self, number = 0) :
        return "{%s, %2d, %d, %-20s, {%s}} // %3d" \
               % ( self.Enum
                 , len (self.bytes)
                 , self.response_count
                 , self.check_function
                 , ", ".join ("0x%02X" % x for x in self.bytes)
                 , number
                 )
    # end def test_case_entry

    @classmethod
    def From_Trx (cls, line) :
        time, bytes = line.split ("\t")
        kind, bytes = bytes.split (" ", 1)
        kind        = int (kind, 16)
        bytes       = [int (n, 16) for n in bytes.split (" ")]
        for cls in Send_Record, Received_Record :
            if kind == cls.Kind :
                return cls (bytes, time)
    # end def From_Trx

    def __repr__ (self) :
        return "%s (%s, %s)" % \
            ( self.__class__.__name__
            , " ".join ("%02X" % x for x in self.bytes)
            , self.time
            )
    # end def __repr__

# end class _Record_

class Send_Record (_Record_) :
    """A record for a telegram sent from the ETS to the device(s)"""

    Kind          = 0x2E
    Enum          = "TEL_RX"
    Read_Requests = dict ()

    def __init__ (self, * args, ** kw) :
        super (Send_Record, self).__init__ (* args, ** kw)
        self.response_count = 0
        bytes = self.bytes
        if bytes [1] == 0 and bytes [2] == 0 :
            ### convert a sender address of 0.0.0 into 0.0.1
            self.bytes [2] = 0x01
        tpci = bytes [6] & 0xC3
        if tpci == 0x42 : ### memory operations
            apci = bytes [7] & 0xF0
            if (apci == 0x00) or (apci == 0x80): ###    a memory read request
                                                 ### or a memroy write request
                count   = bytes  [7] & 0x0F
                address = (bytes [8] << 8) | bytes [9]
                value   = None if apci == 0x00 else "WF"
                for addr in range (address, address + count) :
                    if addr not in self.Read_Requests :
                        self.Read_Requests [addr] = value

    # end def __init__

    @property
    def check_function (self) :
        if self.bytes [5] == 0x60 and self.bytes [6] == 0x80 :
            return "check_connect"
        if self.bytes [5] == 0x60 and self.bytes [6] == 0x81 :
            return "check_disconnect"
        return "NULL"
    # end def check_function

# end class Send_Record

class Received_Record (_Record_) :
    """A record of a telegram received by the ETS."""

    Kind     = 0x29
    Enum     = "TEL_TX"

    def __init__ (self, * args, ** kw) :
        super (Received_Record, self).__init__ (* args, ** kw)
        self.response_count = 0
        bytes = self.bytes
        if bytes [3] == 0 and bytes [4] == 0 :
            ### convert a destination address of 0.0.0 into 0.0.1
            self.bytes [4] = 0x01
        tpci = bytes [6] & 0xC3
        if tpci == 0x42 : ### memory operations
            apci = bytes [7] & 0xF0
            if apci == 0x40 : ### a memory read response
                RR      = Send_Record.Read_Requests
                count   = bytes  [7] & 0x0F
                address = (bytes [8] << 8) | bytes [9]
                for i, addr in enumerate (range (address, address + count)) :
                    value = RR.get (addr, "<undef>")
                    if value is None :
                        ### this address has been accessed to first time
                        ### we need to initialize it to make the test case
                        ### succeed
                        RR [addr] = bytes [10 + i]
                    if value == "<undef>" :
                        ### this address is write to before a read ahs been
                        ### made -> we need to mark this so that we DO NOT
                        ### initialze this address
                        RR [addr] = "WF"
    # end def __init__

# end class Received_Record

class Trx_File (object) :
    """A recorded session of the ETS"""

    def __init__ (self, file_name) :
        self.file_name = file_name
        self.records   = []
        #import pdb;  pdb.set_trace ()
        with open (file_name) as file :
            for line in file :
                self.records.append (_Record_.From_Trx (line.strip ()))
        records  = self.records [:] ### make a copy
        last_snd = None
        tx_count = 0
        while records :
            r = records.pop (0)
            if isinstance (r, Send_Record) :
                if last_snd :
                    last_snd.response_count = tx_count
                last_snd  = r
                tx_count  = 0
            else :
                tx_count += 1
        if last_snd :
            last_snd.response_count = tx_count
    # end def __init__

    def as_test_case (self, name) :
        var_name = name.lower ().replace (" ", "_")
        telegrams = []
        for n, r in enumerate (self.records) :
            telegrams.append (r.test_case_entry (1 + n))
        telegrams.append ("{END}")
        result    = [ "// automatically generated from %s"
                    % (os.path.basename (self.file_name), )
                    , ""
                    , '#include "CUnit/Basic.h"'
                    , ""
                    , '#include "sb_proto_test.h"'
                    , '#include "sb_bus.h"'
                    , '#include "sb_proto.h"'
                    , '#include "sb_const.h"'
                    , '#include "sb_eep_emu.h"'
                    , ""
                    ]
        self._add_default_check_functions (result)
        result.extend \
            ( ( ""
              , "Telegram %s_t[] =\n{ %s\n};"
              % (var_name, "\n, ".join (telegrams))
              , ""
              , "Test_Case %s =" % (var_name, )
              , '{ "%s"'         % (name, )
              , ", 0x00   // sbState"
              , ", tc_setup // init function"
              , ", %s_t   // telegrams" % (var_name, )
              , "};"
              )
            )
        return "\n".join (result)
    # end def as_test_case

    def _add_default_check_functions (self, result) :
        result.extend \
            ( ( "static void check_connect(void)"
              , "{"
              , '  CU_ASSERT(sbConnectedAddr == 0x0001);'
              , "}"
              , ""
              , "static void check_disconnect(void)"
              , "{"
              , '  CU_ASSERT(sbConnectedAddr == 0x0000);'
              , "}"
              , ""
              , "static void tc_setup(void)"
              , "{"
              , '  sbOwnPhysicalAddr = 0x1112; // set own address to 1.1.18'
              )
            )
        for addr, value in sorted (Send_Record.Read_Requests.items ()) :
            if isinstance (value, int) :
                if addr & 0xFF00 :
                    result.append \
                        ("  eep [0x%02X] = 0x%02X;" % (addr & 0xFF, value))
        result.append ("}")
    # end def _add_default_check_functions

# end class Trx_File

if __name__ == "__main__" :
    import sys
    trx = Trx_File (sys.argv [1])
    print (trx.as_test_case (sys.argv [2]))
### __END__ Trx2Test_Case
