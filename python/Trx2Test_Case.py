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

    def __init__ (self, bytes, time = None) :
        self.bytes = bytes
        self.time  = time
    # end def __init__

    def test_case_entry (self) :
        return "{%s, %2d, NULL, {%s}}" \
               % ( self.Enum
                 , len (self.bytes)
                 , ", ".join ("0x%02X" % x for x in self.bytes)
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

    Kind     = 0x2E
    Enum     = "TEL_RX"

# end class Send_Record

class Received_Record (_Record_) :
    """A record of a telegram received by the ETS."""

    Kind     = 0x29
    Enum     = "TEL_TX"

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
    # end def __init__

    def as_test_case (self, name) :
        telegrams = []
        for r in self.records :
            telegrams.append (r.test_case_entry ())
        telegrams.append ("{END}")
        result    = [ "// automatically generated from %s"
                    % (os.path.basename (self.file_name), )
                    , ""
                    , '#include "sb_proto_test.h"'
                    , ""
                    , "Telegram %s_t[] =\n{ %s\n};"
                    % (name, "\n, ".join (telegrams))
                    , ""
                    , "Test_Case %s =\n{ 0x00   // sbState" % (name, )
                    , ", NULL   // init function"
                    , ", %s_t   // telegrams" % (name, )
                    , "};"
                    ]
        return "\n".join (result)
    # end def as_test_case

# end class Trx_File

if __name__ == "__main__" :
    import sys
    trx = Trx_File (sys.argv [1])
    print (trx.as_test_case ("prog_pa"))
### __END__ Trx2Test_Case
