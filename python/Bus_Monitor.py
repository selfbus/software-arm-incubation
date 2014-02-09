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
    sys.excepthook = info

    Trace_File (sys.argv [1])
### __END__ Bus_Monitor
