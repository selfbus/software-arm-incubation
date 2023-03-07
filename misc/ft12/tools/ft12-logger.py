#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# start with "python ft12-logger.py"

# Selfbus RM (smoke detector) logger

from time import sleep
from datetime import datetime

import keyboard
import serial

SERIAL_PORT_FT12 = "COM4"
SERIAL_PORT_KNXD = "COM6"

PAUSE = 0.001


def convert_to_readable_format(s):
    result = ''
    for x in s:
        result += ' 0x{0:02x}'.format(x)
    return result


class Ft12Logger(object):
    def __init__(self):
        self.ser_ft12 = serial.Serial(SERIAL_PORT_FT12, baudrate=19200, parity=serial.PARITY_EVEN,
                                      stopbits=serial.STOPBITS_ONE,
                                      bytesize=serial.EIGHTBITS, timeout=.001)
        self.ser_knxd = serial.Serial(SERIAL_PORT_KNXD, baudrate=19200, parity=serial.PARITY_EVEN,
                                      stopbits=serial.STOPBITS_ONE,
                                      bytesize=serial.EIGHTBITS, timeout=.001)

        print("Selfbus ft12<->knxd logger using")
        print(" ft12 COM-Port: {0:s}".format(self.ser_ft12.name))
        print(" knxd COM-Port: {0:s}".format(self.ser_knxd.name))
        print("")
        print("Keys:")
        print("q = quit")
        print("")

    def execute(self):
        while self.ser_ft12.is_open and self.ser_knxd.is_open:
            bytes_to_read = self.ser_ft12.inWaiting()
            if bytes_to_read > 0:
                msg_ft12 = bytearray(self.ser_ft12.read(bytes_to_read))
                time = datetime.now().strftime('%H:%M:%S.%f')
                print("{0:s} ft12 {1:s}".format(time, convert_to_readable_format(msg_ft12)))

            bytes_to_read = self.ser_knxd.inWaiting()
            if bytes_to_read > 0:
                msg_knxd = bytearray(self.ser_knxd.read(bytes_to_read))
                time = datetime.now().strftime('%H:%M:%S.%f')
                # print("{0:s} knxd {1:s}".format(time, convert_to_readable_format(msg_knxd)))

            # Check if a key was pressed
            if keyboard.is_pressed('q'):
                print('q Key was pressed, exiting...')
                exit()
            else:
                sleep(PAUSE)


if __name__ == '__main__':
    test = Ft12Logger()
    test.execute()
