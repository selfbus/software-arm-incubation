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
#    Test_Case
#
# Purpose
#    Classes for test case generation
#
# Revision Dates
#    27-Jun-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

import io
import re
import datetime
import Test_Case_Store
import Device
import os
from   Com_Table import Com_Object

class Section :
    """A simple comment in the output"""

    def __init__ (self, * lines, ** kw) :
        self.lines    = lines
        new_line      = kw.pop ("new_line", None)
        if new_line is None :
            new_line = 1 if Test_Case_Store.Test_Case.steps else 0
        self.new_line = new_line
        Test_Case_Store.Test_Case.steps.append (self)
    # end def __init__

    def init_code (self, head, number) :
        code = "// %s\n" % ("\n// ".join (self.lines), )
        nl   = self.new_line
        while nl :
            code = "\n%s" % (code, )
            nl  -= 1
        return number, code
    # end der init_code

# end class Section

class Comment (Section) :
    """Add a comment line to the source file"""

    def __init__ (self, * args, ** kw) :
        if "new_line" not in kw :
            kw ["new_line"] = 0
        super (Comment, self).__init__ (* args, ** kw)
    # end def __init__

# end class Comment

class Include :
    """Include a set of test steps from an external file"""

    def __init__ (self, file_name) :
        tc        = Test_Case_Store.Test_Case
        file_name = os.path.join (tc.directory, file_name)
        with open (file_name) as f :
            code      = compile (f.read (), file_name, "exec")
            exec (code, globals (), tc.vars)
    # end def __init__

# end class Include

class Test_Step :
    """Base class for all kinds fo test steps."""

    tick_length = 0

    def __init__ ( self
                 , kind
                 , step     = "NULL"
                 , length   = 0
                 , variable = 0
                 , telegram = None
                 , comment  = None
                 , new_line = 0
                 ) :
        self.kind     = kind
        self.length   = length
        self.variable = variable
        self.step     = step
        self.new_line = new_line
        if telegram :
            bytes     = ["0x%02X" % b for b in telegram.bytes [:-1]]
            self.length = len (bytes)
            telegram  = "{%s}" % (", ".join (bytes))
        else :
            telegram  = "{}"
        self.telegram = telegram
        self.comment  = comment
        Test_Case_Store.Test_Case.steps.append (self)
        self.Tick     = self.Ticks
        Test_Step.Ticks += self.tick_length
    # end def __init__

    def init_code (self, head, number) :
        result = ( "/* %3d */ %s{%-15s, %4d, %2d, (StepFunction *) %-20s, %s}\n"
                 % ( number
                   , head, self.kind, self.length, self.variable
                   , self.step or "NULL", self.telegram
                   )
                 )
        if self.comment :
            comment = self.comment
            if not isinstance (comment, (tuple, list)) :
                comment = (comment, )
            comment = "\n  // ".join (comment)
            result = "          // %s\n%s" % (comment, result)
        if self.new_line :
            result = "\n" + result
        return number + 1, result
    # end def as_init

    def __add__ (self, rhs) :
        return self.Tick + rhs - self.Ticks
    # end def __add__

# end class Test_Step

class App_Loop (Test_Step) :
    """Run the application loop and simulate time passing"""

    def __init__ (self, step = "_loop", ticks = 0, ** kw) :
        self.tick_length = ticks
        super (App_Loop, self).__init__ \
            ( kind   = "TIMER_TICK"
            , step   = step
            , length = ticks
            , ** kw
            )
    # end def __init__

# end class App_Loop

class Progress_Time (App_Loop) :
    """Just simulate some passing time"""

    def __init__ (self, ticks, comment = None, ** kw) :
        super (Progress_Time, self).__init__ \
            ( ticks   = ticks
            , comment = comment
            , ** kw
            )
    # end def __init__

# end class Progress_Time

class Send_Telegram (Test_Step) :
    """Check if the prepared send telegram in the queue of the device
       matches this telegram
    """

    def __init__ (self, telegram, value = None, ** kw) :
        if isinstance (telegram, Com_Object) :
            telegram = telegram.send_telegram (value)
        super (Send_Telegram, self).__init__ \
            ( kind     = "TEL_TX"
            , step     = kw.pop ("step", "_loop")
            , telegram = telegram
            , ** kw
            )
    # end def __init__

# end class Send_Telegram

class Receive_Telegram (Test_Step) :
    """Put the telegram into the receive buffer of the device"""

    def __init__ (self, telegram, value = None, ** kw) :
        if isinstance (telegram, Com_Object) :
            telegram = telegram.receive_telegram (value)
        super (Receive_Telegram, self).__init__ \
            ( kind     = "TEL_RX"
            , step     = kw.pop ("step", "_loop")
            , telegram = telegram
            , ** kw
            )
    # end def __init__

# end class Receive_Telegram

class Test_Case :
    """A test case"""

    device_spec_pat = re.compile ("device_spec\s*=\s*\"([\w.]+)\"")

    def __init__ (self, file_name) :
        self._parse (file_name)
    # end def __init__

    def _parse (self, file_name) :
        self.steps     = []
        self.file_name = os.path.abspath (file_name)
        self.directory = os.path.dirname (file_name)
        self.vars      = vars = dict ()
        with open (file_name) as f :
            content = f.read                      ()
            match   = self.device_spec_pat.search (content)
            if match :
                pwd      = os.getcwd ()
                os.chdir             (os.path.dirname (file_name))
                vars ["device"] = Device._BCU_.from_device_file \
                    (match.group (1))
                os.chdir         (pwd)
                Test_Case_Store.Test_Case = self
                Test_Step.Ticks           = 0
                code                      = compile (content, file_name, "exec")
                exec (code, globals (), vars)
            else :
                raise ValueError ("No device specified")
        self.name           = vars.pop ("name")
        self.description    = vars.pop ("description", None)
        self.setup          = vars.pop ("setup", "NULL")
        self.state          = vars.pop ("state", "_gatherState")
        self.power_on_delay = vars.pop ("power_on_delay", 0)
        self.device         = vars ["device"]
        self.tags           = vars.pop ("tags", ())
        if self.tags :
            self.tags = "[%s]" % ("][".join (self.tags), )
        if isinstance (self.power_on_delay, str) :
            self.power_on_delay = -1# eval \
                #(self.power_on_delay, {}, dict (E = self.eeprom))
    # end def _parse

    def _replace_in_file (self, content, section, new) :
        pat = re.compile \
            ( "^\s*// >>> %s$(.)+// <<< %s$" % (section, section)
            , re.MULTILINE | re.DOTALL
            )
        if pat.search (content) :
            return pat.sub (new.strip (), content)
        return content + new
    # end def _replace_in_file

    def create_code (self, file_name) :
        file = io.StringIO ()
        file.write ("// >>> TC:%s\n" % (self.name, ))
        file.write ("// Date: %s\n" % (datetime.datetime.now (), ))
        file.write ("\n/* Code for test case %s */\n" % (self.name, ))
        ee_init   = "NULL"
        man = dev = ver = 0
        if self.device :
            ee_init  = "%s_eepromSetup" % (self.name, )
            file.write ("static void %s(void)\n" % (ee_init, ))
            file.write ("{\n")
            self.device.test_code_init (file = file)
            file.write ("}\n\n")
            man = self.device.manufacturer
            dev = self.device.deviceType
            ver = self.device.version
        if self.steps :
            file.write ("static Telegram tel_%s[] =\n" % (self.name, ))
            file.write ("{\n")
            head = "  "
            i    = 1
            for t in self.steps :
                i, code = t.init_code (head, i)
                file.write            (code)
                if i > 1 :
                    head = ", "
            file.write (", {END}\n")
            file.write ("};\n")

        file.write ("static Test_Case %s_tc = \n" % (self.name, ))
        file.write ("{\n")
        file.write ('  "%s"\n' % (self.description or self.name, ))
        file.write (", 0x%04x, 0x%04x, %02x\n" % (man, dev, ver))
        file.write (", %d // power-on delay\n" % (self.power_on_delay, ))
        file.write (", %s\n" % (ee_init, ))
        file.write (", %s\n" % (self.setup, ))
        file.write (", (StateFunction *) %s\n" % (self.state or "NULL"))
        file.write (", (TestCaseState *) &_refState\n")
        file.write (", (TestCaseState *) &_stepState\n")
        file.write (", tel_%s\n" % (self.name, ))
        file.write ("};\n\n")
        file.write ('TEST_CASE("%s","%s")\n' % (self.description, self.tags))
        file.write ("{\n")
        file.write ("  executeTest(& %s_tc);\n" % (self.name, ))
        file.write ("}\n")
        file.write ("// <<< TC:%s\n" % (self.name, ))
        if not file_name :
            sys.stdout.write (file.getvalue ())
        else :
            if not os.path.exists (file_name) :
                self._create_file (file_name)
            with open (file_name) as f :
                content = f.read ()
            with open (file_name, "w") as f :
                f.write \
                    ( self._replace_in_file
                        ( content
                        , "TC:%s" % (self.name, )
                        , file.getvalue ()
                        )
                    )
                print ("%s updated" % (file_name, ))
    # end def create_code

    def _create_file (self, file_name) :
        code = """/*
 *  %s -
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "protocol.h"
#include "catch.hpp"
#include "sblib/timer.h"
"""
        with open (file_name, "w") as file :
            file.write (code % os.path.basename (file_name))
    # end def _create_file

# end class Test_Case

if __name__ == "__main__" :
    import sys
    import glob
    file_name = None
    if len (sys.argv) > 2 :
        file_name = sys.argv [2]
    tc_spec = sys.argv [1]
    for tc in glob.glob (tc_spec) :
        Test_Case (tc).create_code (file_name)
### __END__ Test_Case
