#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# start with "python rm-logger.py"

# Selfbus RM (smoke detector) logger

from time import sleep
from datetime import datetime

import keyboard
import serial

SERIAL_PORT_ARM = "COM6"
SERIAL_PORT_SMOKE = "COM7"

PAUSE_LONG = 2.0
PAUSE = 0.1


def convert_to_readable_format(s):
    result = ''
    for x in s:
        if x == 0x00:
            result += "<NUL>"
        elif x == 0x02:
            result += "<STX>"
        elif x == 0x03:
            result += "<ETX>"
        elif x == 0x06:
            result += "<ACK>"
        elif x == 0x15:
            result += "<NAK>"
        elif x >= 0x20:
            result += chr(x)
        else:
            result += ' 0x{0:02x}'.format(x)
    return result


class RMLogger(object):
    def __init__(self):
        self.ser_arm = serial.Serial(SERIAL_PORT_ARM, baudrate=9600, parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE,
                                     bytesize=serial.EIGHTBITS, timeout=1)
        self.ser_smoke = serial.Serial(SERIAL_PORT_SMOKE, baudrate=9600, parity=serial.PARITY_NONE,
                                       stopbits=serial.STOPBITS_ONE,
                                       bytesize=serial.EIGHTBITS, timeout=1)

        print("Selfbus RM (smoke detector) logger using")
        print(" ARM COM-Port  : {0:s}".format(self.ser_arm.name))
        print(" Detector COM-Port: {0:s}".format(self.ser_smoke.name))
        print("")
        print("Keys:")
        print("q = quit")
        print("")

    def execute(self):
        while self.ser_arm.is_open or self.ser_smoke.is_open:
            if self.ser_arm.is_open:
                bytes_to_read = self.ser_arm.inWaiting()
                if bytes_to_read > 0:
                    msg_arm = bytearray(self.ser_arm.read(bytes_to_read))
                    time = datetime.now().strftime('%H:%M:%S.%f')
                    print("{0:s} ARM   {1:s}".format(time, convert_to_readable_format(msg_arm)))

            if self.ser_smoke.is_open:
                bytes_to_read = self.ser_smoke.inWaiting()
                if bytes_to_read > 0:

                    msg_smoke = bytearray(self.ser_smoke.read(bytes_to_read))
                    time = datetime.now().strftime('%H:%M:%S.%f')
                    print("{0:s} Smoke {1:s}".format(time, convert_to_readable_format(msg_smoke)))

            # Check if a key was pressed
            if keyboard.is_pressed('q'):
                print('q Key was pressed, exiting...')
                exit()
            else:
                sleep(0.01)


if __name__ == '__main__':
    test = RMLogger()
    test.execute()
