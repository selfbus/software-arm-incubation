# -*- coding: utf-8 -*-
# Copyright (C) 2014-2015 Martin Glueck All rights reserved
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
#    Bus_Monitor
#
# Purpose
#    Interface to the busmonitor selfbus tool to display the EIB telegrams
#
# Revision Dates
#    30-Jan-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
import   serial

from Telegram import _Object_, _Telegram_

class Trace_File (_Object_) :
    """Load the protocol byte stream from a file captured by a terminal
       programm
    """

    def __init__ (self, filename) :
        with open (filename) as f :
            for l in f :
                raw = bytes ([int (p, 16) for p in l.split (" ")])
                #import pdb; pdb.set_trace ()
                t = _Telegram_.From_Raw (raw)
                print (t)
    # end def __init__

# end class Trace_File

class Bus_Monitor (_Object_) :
    """Listen for telegrams received by the embedded node and sent over the
       serial connection.
    """

    def __init__ (self, port) :
        self.port = serial.Serial (port, 115200)
    # end def __init__

    def run (self, file = None) :
        try :
            telegram  = b""
            data      = b""
            skip_line = False
            while True :
                byte = self.port.read ()
                #print (byte, data, skip_line)
                if byte == b" " :
                    if not skip_line :
                        try :
                            telegram += bytes ((int (data, 16), ))
                            data = b""
                        except :
                            skip_line = True
                    if skip_line :
                        data += byte
                elif byte == b"\n" or byte == b"\r" :
                    if data and not skip_line :
                        telegram += bytes ((int (data, 16), ))
                        data      = b""
                    elif data :
                        telegram  = b""
                        skip_line = False
                        print (data.decode ("latin1"))
                        data = b""
                    if telegram :
                        if len (telegram) == 1 and telegram [0] == 0xCC :
                            continue ### ignore this type of telegram for now
                        #print (telegram)
                        try :
                            t = _Telegram_.From_Raw (telegram)
                        except :
                            t = ""
                        print (t or telegram)
                        if file :
                            for b in telegram :
                                file.write ("%02X " % (b, ))
                            file.write ("\n    %s\n" % str (t))
                    telegram = b""
                else :
                    data += byte
                ##print (byte, data, telegram)
        finally :
            if file :
                file.close ()
    # end def run

# end class Bus_Monitor

if __name__ == "__main__" :
    import sys

    def info(type, value, tb):
       if hasattr(sys, 'ps1') or not sys.stderr.isatty():
          # we are in interactive mode or we don't have a tty-like
          # device, so we call the default hook
          sys.__excepthook__(type, value, tb)
       else:
          import traceback, pdb
          # we are NOT in interactive mode, print the exception...
          traceback.print_exception(type, value, tb)
          print
          # ...then start the debugger in post-mortem mode.
          pdb.pm()
    #sys.excepthook = info

    #Trace_File (sys.argv [1])
    bm = Bus_Monitor (sys.argv [1])
    file = None
    if len (sys.argv) > 2 :
        file = open (sys.argv [2], "w")
    bm.run           (file)
### __END__ Bus_Monitor
