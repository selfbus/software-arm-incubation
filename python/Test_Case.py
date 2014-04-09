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
#    Test_Case_Entry
#
# Purpose
#    Generate the init data for a test case entry
#
# Revision Dates
#    24-Feb-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals
import   sys
import   io
import   re
import   datetime

class Test_Case_Entry (object) :
    """An entry in the test case list"""

    def __init__ ( self, type
                 , length   = 0
                 , variable = 0
                 , step     = None
                 , telegram = None
                 , comment  = None
                 , new_line = False
                 ) :
        self.type     = type
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
    # end def __init__

    def init_code (self, head, number) :
        result = ( "%s{%-15s, %2d, %2d, (StepFunction *) %-20s, %s} // %3d\n"
                 % (head, self.type, self.length, self.variable
                   , self.step or "NULL", self.telegram, number
                   )
                 )
        if self.comment :
            comment = self.comment
            if not isinstance (comment, (tuple, list)) :
                comment = (comment, )
            comment = "\n  // ".join (comment)
            result = "  // %s\n%s" % (comment, result)
        if self.new_line :
            result = "\n" + result
        return result
    # end def as_init

# end class Test_Case_Entry

class Test_Case (object) :
    """A test case"""

    def __init__ (self, name
                 , eeprom       = None
                 , setup        = None
                 , state        = None
                 , description  = None
                 , tags         = ()
                 , * telegrams
                 ) :
        self.name         = name
        self.eeprom       = eeprom
        self.setup        = setup
        self.state        = state
        self.description  = description
        self.telegrams    = list (telegrams)
        if tags :
            tags = "[%s]" % ("][".join (tags))
        self.tags         = tags
    # end def __init__

    def add (self, * telegrams) :
        self.telegrams.extend (telegrams)
    # end def add

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
        if self.eeprom :
            ee_init  = "%s_eepromSetup" % (self.name, )
            file.write ("static void %s(void)\n" % (ee_init, ))
            file.write ("{\n")
            self.eeprom.test_code_init (file = file)
            file.write ("}\n\n")
            man = self.eeprom.manufacturer
            dev = self.eeprom.deviceType
            ver = self.eeprom.version
        if self.telegrams :
            file.write ("static Telegram tel_%s[] =\n" % (self.name, ))
            file.write ("{\n")
            head = "  "
            for i, t in enumerate (self.telegrams) :
                file.write (t.init_code (head, i + 1))
                head = ", "
            file.write (", {END}\n")
            file.write ("};\n")

        file.write ("static Test_Case %s_tc = \n" % (self.name, ))
        file.write ("{\n")
        file.write ('  "%s"\n' % (self.description or self.name, ))
        file.write (", 0x%04x, 0x%04x, %02x\n" % (man, dev, ver))
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
    # end def create_code

# end class Test_Case

### __END__ Test_Case_Entry
