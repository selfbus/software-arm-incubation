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
#    FPC_AM
#
# Purpose
#    Handling of the FPC AM fingerprint module
#
# Revision Dates
#    30-Jul-2014 (MG) Creation
#    ««revision-date»»···
#--

from   __future__ import division, print_function
from   __future__ import absolute_import, unicode_literals

import serial
import struct
from   enum import Enum
import PIL.Image

class Response_Code (Enum) :

    Failure              = 0x00
    Success              = 0x01
    No_Finger_Present    = 0x02
    Finger_Present       = 0x03
    Verification_Ok      = 0x04
    Verification_Fail    = 0x05
    Enrol_OK             = 0x06
    Enrol_Fail           = 0x07
    Hw_Test_Ok           = 0x08
    Hw_Test_Fail         = 0x09
    Crc_Fail             = 0x0A
    Payload_Too_Long     = 0x0B
    Payload_Too_Short    = 0x0C
    Unknown_Command      = 0x0D
    No_Template_Present  = 0x0E
    Identify_Ok          = 0x0F
    Identify_Fail        = 0x10
    Invalid_Slot_Nr      = 0x11
    Cancel_Success       = 0x12
    FW_Crc_Fail          = 0x14

    Invalid_Response     = 0xFF

# end class Response_Code

class Commands (Enum) :

    Capture_Image        = 0x80
    Enrol                = 0x81
    Verify               = 0x82
    Identify             = 0x84
    Upload_Image         = 0x90
    Download_Image       = 0x91
    Version              = 0xC0
    Download_Template    = 0xA1
    Store_Template       = 0xA2
    Upload_Template      = 0xA3
    Delete_Slot          = 0xA5
    Delete_All           = 0xA6
    Baud_Rate_Ram        = 0xD0
    Baud_Rate_Static     = 0xD1
    Hardware_Test        = 0xD2

# end class Commands

class FPC_AM (object) :
    """Handling of the FPC AM module"""

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

    class Enrolment_Error (Exception) :

        def __init__ (self, code) :
            super (Exception, self).__init__ \
                ("Error during enrolment [%s]" % (code, ))
        # end def __init__

    # end class Enrolment_Error

    def __init__ (self, connection) :
        self.connection = connection
        ### read bytes from module to ensure clean connection
        try :
            self._read (2048, timeout = 0.5)
        except self.Read_Timeout :
            pass
        if self.hardware_test () != Response_Code.Hw_Test_Ok :
            raise TypeError ("Hardware test failed")
    # end def __init__

    def _cmd ( self, code, idx = 0, data = "", crc = None
             , Break = False
             , echo  = False
             ) :
        code = getattr (code, "value", code)
        cmd = struct.pack ("<BHBH", self.START_BYTE, idx, code, len (data))
        if data :
            cmd += data
            if not crc :
                raise ValueError ("Don't know how to calculate the CRC")
            cmd += crc
        self.connection.write (cmd)
        start  = self._read   (1)
        result = Response_Code.Invalid_Response
        crc    = None
        data   = None
        if Break :
            import pdb; pdb.set_trace ()
        if ord (start) == self.START_BYTE :
            code, length = struct.unpack ("<BH", self._read (3))
            result = Response_Code       (code)
            if length :
                data = self._read (length, echo = echo)
                crc  = self._read (4)
        return result, data, crc
    # end def _cmd

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

    ### commands
    def change_baud_rate (self, baudrate, static = False) :
        cmd  = Commands.Baud_Rate_Static if static else Commands.Baud_Rate_Ram
        code = self.Baud_Rates [baudrate]
        result, _, _ = self._cmd (cmd, idx = code)
        if result == Response_Code.Success :
            self.connection.baudrate = baudrate
        return result
    # end def change_baud_rate

    def delete_all (self) :
        result, _, _ = self._cmd (Commands.Delete_All)
        return  result
    # end def delete_all

    def download (self, file, slot) :
        if isinstance (file, str) :
            file = open (file, "rb")
        size         = struct.unpack ("I", file.read (4)) [0]
        data         = file.read (size)
        crc          = file.read (4)
        result, _, _ = self._cmd \
            (Commands.Download_Template, data = data, crc = crc)
        if result == Response_Code.Success :
          result, _, _ = self._cmd (Commands.Store_Template, idx = slot)
        return result
    # end def download

    def enrol (self, slot, verification = 3) :
        self.progress ("Place finger on sensor to enrol")
        result = Response_Code.Failure
        while result != Response_Code.Enrol_OK :
            result, _, _ = self._cmd (Commands.Enrol)
            if result == Response_Code.Enrol_Fail :
                raise self.Enrolment_Error (result)
        self.ensure_no_finger ("Please remove finger from sensor")
        while verification :
            verification -= 1
            self.progress \
                ( "Place finger on sensor for verification [%d]"
                % (verification, )
                )
            result, _, _  = self._cmd (Commands.Verify)
            if result != Response_Code.Verification_Ok :
                raise self.Enrolment_Error (result)
            self.ensure_no_finger ("Please remove finger from sensor")
        self.progress             ("Store finger in slot %d" % (slot, ))
        result, _, _ = self._cmd  (Commands.Store_Template, idx = slot)
        return result
    # end def enrol

    def ensure_no_finger (self, progress = None) :
        if progress :
            self.progress (progress)
        result = Response_Code.Finger_Present
        while result != Response_Code.No_Finger_Present :
            result, _, _ = self._cmd (Commands.Capture_Image)
    # end def ensure_no_finger

    def hardware_test (self) :
        result, _, _ = self._cmd (Commands.Hardware_Test)
        return  result
    # end def hardware_test

    def identify (self, tries = 3) :
        self.progress ("Place finger on sensor")
        result = Response_Code.Identify_Fail
        while (result != Response_Code.Identify_Ok) and tries :
            tries -= 1
            result, data, crc = self._cmd (Commands.Identify)
            if result == Response_Code.Identify_Ok :
                ###self.ensure_no_finger ("Please remove finger from sensor")
                return struct.unpack  ("<H", data) [0]
        return -1
    # end def identify

    def progress (self, text) :
        print (text)
    # end def progress

    def upload (self, file, slot) :
        if isinstance (file, str) :
            file = open (file, "wb")
        result, data, crc = self._cmd (Commands.Upload_Template)
        if result == Response_Code.Success :
            file.write (struct.pack ("I", len (data)))
            file.write (data)
            file.write (crc)
            file.close ()
        return result
    # end def upload

    def upload_image (self, filename) :
        self.progress ("Place finger on sensor to capture image")
        result = Response_Code.No_Finger_Present
        while result != Response_Code.Finger_Present :
            result, _, _ = finger._cmd (Commands.Capture_Image)
        self.progress ("Uploading image, this could take a while")
        result, data, crc = finger._cmd (Commands.Upload_Image)
        img               = None
        if result == Response_Code.Success :
            img = PIL.Image.new ("L", self.Image_Size)
            img.putdata         (data)
            img.save            (filename)
            self.progress       ("Image %s stored" % (filename, ))
        return img
    # end def upload_image

    def version (self) :
        r, v, _ = finger._cmd (Commands.Version)
        if r == Response_Code.Success :
            return v.decode ()
    # end def version

# end class FPC_AM

if __name__ == "__main__" :
    import argparse
    parser = argparse.ArgumentParser (description = "Fingerprint sensor")
    #parser.add_argument \
    #    ( 'integers', metavar='N', type=int, nargs='+'
    #    , help='an integer for the accumulator'
    #    )
    parser.add_argument \
        ( "-s", "--slot", dest = "slot"
        , action = "store"
        , type   = int
        , help   = "Slot number inside the fingerprint module"
        )
    parser.add_argument \
        ( "--delete", dest = "delete"
        , action = "store_true"
        , help   = "Delete all stored templates"
        )
    parser.add_argument \
        ( "-e", "--enrol", dest = "enrol"
        , action = "store_true"
        , help   = "Enrol a new finter to the specified slot"
        )
    parser.add_argument \
        ( "-u", "--upload", dest = "upload"
        , action = "store"
        , type   = argparse.FileType ("wb")
        , help   = "Upload the stored finger"
        )
    parser.add_argument \
        ( "-U", "--upload_image", dest = "upload_image"
        , action = "store"
        , type   = str
        , help   = "Catpure and upload an image of an finger"
        )
    parser.add_argument \
        ( "-d", "--download", dest = "download"
        , action = "store"
        , type   = argparse.FileType ("rb")
        , help   = "Download the finger to the module"
        )
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
    finger = FPC_AM (con)
    print (finger.version ())

    if args.enrol or args.upload or args.download :
        if args.slot is None :
            raise ValueError ("Need a slot number")
        slot = args.slot
        if args.enrol :
            finger.enrol (slot)
        if args.upload :
            finger.upload   (args.upload,   args.slot)
        if args.download :
            finger.download (args.download, args.slot)
    elif args.delete :
        finger.delete_all ()
    elif args.upload_image :
        img = finger.upload_image (args.upload_image)
        if img :
            img.show ()
    elif args.interactive :
        pass
    else :
        while True :
            slot = finger.identify ()
            print (slot)
### __END__ FPC_AM
