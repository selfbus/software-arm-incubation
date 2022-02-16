#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# start with "python raincenterlogger.py"

# Selfbus Raincenter Logger by Darthyson
# created: August 16, 2020

from time import sleep

import keyboard
import serial

from rc_protocol import *

SERIAL_PORT = "COM1"
CMD_READ_PARAMETER = b'p'
CMD_READ_DISPLAY = 'w'
CMD_SET_A = 'a'
CMD_SET_B = 'b'

PAUSE_LONG = 2.0
PAUSE = 0.1


# def TimerTimeOut():
#   print('TimerTimeOut')
#   if len(telegram) > 0:
#       print('removed following bytes from in queue' +telegram.hex())
#   telegram = []

class RaincenterLogger(object):
    def __init__(self):
        self.ser = serial.Serial(SERIAL_PORT, baudrate=2400, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                                 bytesize=serial.EIGHTBITS, timeout=1)
        print("Selfbus Raincenter Logger using COM-Port: {0:s}".format(self.ser.name))
        print("")
        print("Keys:")
        print("c = switch display info")
        print("w = read display data")
        print("p = read parameters including water height in cm")
        print("a = switch to manual tap water refill")
        print("b = switch to reservoir")
        print("s = set parameters (not implemented)")
        print("q = quit")
        print("")

    def send_display_command(self):
        self.send_command(RCDisplayMessage.msgIdentifier)
        # Timer(1, self.SendDisplayCommand(), ()).start()
        # Timer(1, self.SendDisplayCommand(), self).start()

    def send_parameter_command(self):
        self.send_command(RCParameterMessage.msgIdentifier)

    def send_command(self, command):
        self.ser.write(command)
        # Timer(5, TimerTimeOut, telegram).start()
        # self.ser.write(0x00)
        # print(command)
        # sleep(PAUSE_LONG)

    def execute(self):
        telegram = bytearray()
        self.send_display_command()
        # Timer(1, self.SendDisplayCommand(), self).start()
        while self.ser.is_open:
            bytes_to_read = self.ser.inWaiting()
            if bytes_to_read > 0:
                serial_bytes = bytearray(self.ser.read(bytes_to_read))
                telegram += serial_bytes
                msg = get_msg_from_telegram(telegram)
                if msg:
                    msg.decode()
                    print(msg.to_information_text(True, True))
                    # print (len(telegram))
                    telegram = []

            else:
                sleep(0.01)
            # Check if a key was pressed
            if keyboard.is_pressed('c'):
                print('c Key was pressed, switching display')
                self.send_command(RCSwitchDisplayMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('w'):
                print('w Key was pressed, requesting Display-Data')
                self.send_display_command()
                sleep(0.1)
            elif keyboard.is_pressed('p'):
                print('p Key was pressed, requesting Parameter-Data')
                self.send_parameter_command()
                sleep(0.1)
            elif keyboard.is_pressed('a'):
                print('a Key was pressed, switching to port A')
                self.send_command(RCSwitchToAMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('b'):
                print('b Key was pressed, switching to port B')
                self.send_command(RCSwitchToBMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('s'):
                print('s Key was pressed, Setting Parameters for Raincenter...NOT YET IMPLEMENTED')
                print(RCSetParameterMessage.msgTeleExample)
                # self.send_command(RCqSetParameterMessage.msgTeleExample)
                sleep(0.1)
            elif keyboard.is_pressed('q'):
                print('q Key was pressed, exiting...')
                exit()
            else:
                sleep(0.01)


if __name__ == '__main__':
    test = RaincenterLogger()
    test.execute()
