#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# start with "python raincenter.py"

# Selfbus Raincenter Simulator by Darthyson
# created: August 16, 2020

import os, sys, serial, keyboard, time
from time import sleep
from rc_protocol import *
from threading import Timer

CMD_READ_PARAMETER = b'p'
CMD_READ_DISPLAY = 'w'
CMD_SET_A = 'a'
CMD_SET_B = 'b'

PAUSE_LONG = 2.0
PAUSE = 0.1

#def TimerTimeOut():
#   print('TimerTimeOut')
#   if len(telegram) > 0:
#       print('removed following bytes from inque' +telegram.hex())
#   telegram = []

class raincenter(object):
    def __init__(self):
        self.ser = serial.Serial("COM1", baudrate = 2400, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1)
        print("Selfbus Raincenter Simulator using COM-Port: {0:s}".format(self.ser.name))
        print("")
        print("Keys:")
        print("c = switch display info")
        print("w = read display data")
        print("p = read parameters including waterheight in cm")
        print("a = switch to manual tap water refill")
        print("b = switch to reservoir")
        print("s = set parameters (not implemented)")
        print("q = quit")
        print("")
    def SendDisplayCommand(self):
        self.SendCommand(rcDisplayMessage.msgIdentifier)
        #Timer(1, self.SendDisplayCommand(), ()).start()
        #Timer(1, self.SendDisplayCommand(), self).start()
    def SendParameterCommand(self):
        self.SendCommand(rcParameterMessage.msgIdentifier)
        
    def SendCommand(self, command):
        self.ser.write(command)
        #Timer(5, TimerTimeOut, telegram).start()
        #self.ser.write(0x00)
        #print(command)
        #sleep(PAUSE_LONG)

    def Excecute(self):
        telegram = bytearray()
        self.SendDisplayCommand()
        #Timer(1, self.SendDisplayCommand(), self).start()
        while self.ser.is_open:
            bytesToRead = self.ser.inWaiting()
            if bytesToRead > 0:
                serialbytes = bytearray(self.ser.read(bytesToRead))
                telegram += serialbytes
                msg = rcMessage.GetMsgFromTelegram(telegram)
                if (msg):
                    msg.Decode()
                    print(msg.ToInformationText(True, True))
                    #print (len(telegram))
                    telegram = []
                    
                    
            else:
                sleep(0.01)
            # Check if a key was pressed
            if keyboard.is_pressed('c'):
                print('c Key was pressed, switching display')
                self.SendCommand(rcSwitchDisplayMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('w'):
                print('w Key was pressed, requesting Display-Data')
                self.SendDisplayCommand()
                sleep(0.1)
            elif keyboard.is_pressed('p'):
                print('p Key was pressed, requesting Parameter-Data')
                self.SendParameterCommand()
                sleep(0.1)
            elif keyboard.is_pressed('a'):
                print('a Key was pressed, switching to port A')
                self.SendCommand(rcSwitchToAMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('b'):
                print('b Key was pressed, switching to port B')
                self.SendCommand(rcSwitchToBMessage.msgIdentifier)
                sleep(0.1)
            elif keyboard.is_pressed('s'):
                print('s Key was pressed, Setting Parameters for Raincenter...NOT YET IMPLEMENTED')
                print(rcSetParameterMessage.msgTeleExample)
                #self.SendCommand(rcSetParameterMessage.msgTeleExample)
                sleep(0.1)
            elif keyboard.is_pressed('q'):
                print('q Key was pressed, exiting...')
                exit()
            else:
                sleep(0.01)

test = raincenter()
test.Excecute()