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
#    FPC_AM_Simulator
#
# Purpose
#    Simulate a real FPC-AM module
#
# Revision Dates
#    18-Aug-2014 (MG) Creation
#    ««revision-date»»···
#--

from   FPC_AM import Commands, Response_Code
import serial
import struct
import time

class FPC_AM_Simulator :

    START_BYTE = 0x02
    Image_Size = (152, 200) ### Cols, Rows
    Baud_Rates = \
        {   9600 : 0x10
        ,  14400 : 0x20
        ,  19200 : 0x30
        ,  28800 : 0x40
        ,  38400 : 0x50
        ,  57600 : 0x60
        ,  76800 : 0x70
        , 115200 : 0x80
        }

    class Read_Timeout (Exception) : pass

    def __init__ (self, connection) :
        self.connection = connection
        ### read bytes from module to ensure clean connection
        try :
            self._read (2048, timeout = 0.5)
        except self.Read_Timeout :
            pass
    # end def __init__

    def _read (self, length, timeout = False, echo = False) :
        data = []
        con  = self.connection
        if timeout :
            old_timeout = con.timeout
            con.timeout = timeout
        try :
            count = 0
            while length :
                part = self.connection.read (length)
                if part :
                    data.append   (part)
                    length -= len (part)
                    count += len  (part)
                    if echo :
                        print ("%d/%d/%d" % (count, len (part), length))
                elif timeout :
                    raise self.Read_Timeout \
                        ("Could not receive enough bytes from module")
        finally :
            if timeout :
                con.timeout = old_timeout
        return b"".join (data)
    # end def _read

    def _send (self, code, data, crc = b"    ") :
        code = getattr (code, "value", code)
        cmd = struct.pack ("<BBH", self.START_BYTE, code, len (data))
        if data :
            cmd += data
            if not crc :
                raise ValueError ("Don't know how to calculate the CRC")
            cmd += crc
        self.connection.write (cmd)
        print (cmd)
    # end def _send

    def __call__(self) :
        while True :
            cmd = self._read (6)
            start, idx, cmd, length = struct.unpack ("<BHBH", cmd)
            if start != self.START_BYTE :
                raise ValueError ("Out of sequence byte received")
            cmd = Commands (cmd)
            print (cmd)
            if cmd == Commands.Version :
                self._send (Response_Code.Success, b"FPC_AM-SIM")
            elif cmd == Commands.Identify :
                time.sleep (1)
                import pdb;pdb.set_trace ()
                self._send (Response_Code.Identify_Ok, bytes((1, 0)))
    # end def __call__

# end class FPC_AM_Simulator

if __name__ == "__main__" :
    import argparse
    parser = argparse.ArgumentParser (description = "Fingerprint sensor")
    parser.add_argument \
        ( "-c", "--com", dest = "com"
        , action  = "store"
        , default = "COM24"
        , type    = str
        , help    = "COM port"
        )
    parser.add_argument \
        ( "-b", "--baudrate", dest = "baudrate"
        , action  = "store"
        , default = 115200
        , type    = int
        , help    = "Baudrate"
        )
    parser.add_argument \
        ( "--interactive", dest = "interactive"
        , action = "store_true"
        , help   = "Launch interactive shell"
        )

    args = parser.parse_args ()
    con = serial.Serial \
        ( args.com
        , baudrate     = args.baudrate
        , writeTimeout = 0
        , timeout      = 1
        , parity       = serial.PARITY_ODD
        )
    sim = FPC_AM_Simulator (con)
    sim ()