#!/usr/bin/env python3
# -*- coding: iso-8859-1 -*-

# start with "python raincentersimulator.py"

# Selfbus Raincenter Simulator by Darthyson
# created: September 25, 2020

from time import sleep

import keyboard
import serial

from rc_protocol import *

SERIAL_PORT = "COM9"
CMD_READ_PARAMETER = b'p'
CMD_READ_DISPLAY = 'w'
CMD_SET_A = 'a'
CMD_SET_B = 'b'

PAUSE_LONG = 2.0
PAUSE = 0.1


class RaincenterSimulator(object):
    def __init__(self):
        self.ser = serial.Serial(SERIAL_PORT, baudrate=2400, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
                                 bytesize=serial.EIGHTBITS, timeout=1)
        self.ParamMsg = get_msg_from_telegram(RCParameterMessage.msgTeleExample)
        self.ParamMsg.decode()
        print(self.ParamMsg.to_information_text(True, True))
        self.DisplayMsg = get_msg_from_telegram(RCDisplayMessage.msgTeleExample)
        self.DisplayMsg.decode()
        print(self.DisplayMsg.to_information_text(True, True))
        print("Selfbus Raincenter Simulator using COM-Port: {0:s}".format(self.ser.name))
        print("")
        print("Keys:")
        print("q = quit")
        print("+ = increase LevelMeasured")
        print("- = decrease LevelMeasured")
        print("a = AlarmBuzzerActive")
        print("p = PumpActive")
        print("n = ManualSwitchedToTapWater")
        print("m = AutomaticallySwitchedToTapWater")
        print("w = WaterExchangeActive")
        print("")

    def send_display_command(self):
        self.send_command(RCDisplayMessage.msgIdentifier)

    def send_parameter_command(self):
        self.send_command(RCParameterMessage.msgIdentifier)

    def send_command(self, command):
        self.ser.write(command)

    def send_msg_answer(self, telegram_bytes):
        self.ser.write(telegram_bytes)

    def execute(self):

        while self.ser.is_open:
            bytes_to_read = self.ser.inWaiting()
            if bytes_to_read > 0:
                serial_bytes = bytearray(self.ser.read(bytes_to_read))
                # print(serial_bytes[0])
                # print(rcParameterMessage.msgIdentifier)
                if serial_bytes[0] == RCParameterMessage.msgIdentifier[0]:
                    print("rcParameterMessage")
                    self.ParamMsg.encode()
                    self.send_msg_answer(self.ParamMsg.telegram_bytes)
                elif serial_bytes[0] == RCDisplayMessage.msgIdentifier[0]:
                    self.DisplayMsg.encode()
                    self.send_msg_answer(self.DisplayMsg.telegram_bytes)
                    print("rcDisplayMessage")
            else:
                sleep(0.01)
            # Check if a key was pressed
            if keyboard.is_pressed('q'):
                print('q Key was pressed, exiting...')
                exit()
            elif keyboard.is_pressed('+'):
                self.ParamMsg.LevelMeasured += 1
                if self.ParamMsg.LevelMeasured > self.ParamMsg.FillingLevelMax_cm:
                    self.ParamMsg.LevelMeasured = 0
                print('+ Key was pressed new LevelMeasured {0:03d}'.format(self.ParamMsg.LevelMeasured))
                sleep(0.1)
            elif keyboard.is_pressed('-'):
                self.ParamMsg.LevelMeasured -= 1
                if self.ParamMsg.LevelMeasured < 0:
                    self.ParamMsg.LevelMeasured = self.ParamMsg.FillingLevelMax_cm
                print('- Key was pressed new LevelMeasured {0:03d}'.format(self.ParamMsg.LevelMeasured))
                sleep(0.1)
            elif keyboard.is_pressed('a'):
                if self.DisplayMsg.AlarmBuzzerActive:
                    self.DisplayMsg.AlarmBuzzerActive = 0
                else:
                    self.DisplayMsg.AlarmBuzzerActive = 1
                print('a Key was pressed new AlarmBuzzerActive {0:01d}'.format(self.DisplayMsg.AlarmBuzzerActive))
                sleep(0.1)
            elif keyboard.is_pressed('p'):
                if self.DisplayMsg.PumpActive:
                    self.DisplayMsg.PumpActive = 0
                else:
                    self.DisplayMsg.PumpActive = 1
                print('p Key was pressed new PumpActive {0:01d}'.format(self.DisplayMsg.PumpActive))
                sleep(0.1)
            elif keyboard.is_pressed('n'):
                if self.DisplayMsg.ManualSwitchedToTapWater:
                    self.DisplayMsg.ManualSwitchedToTapWater = 0
                else:
                    self.DisplayMsg.ManualSwitchedToTapWater = 1
                print('n Key was pressed new ManualSwitchedToTapWater {0:01d}'.format(
                    self.DisplayMsg.ManualSwitchedToTapWater))
                sleep(0.1)
            elif keyboard.is_pressed('w'):
                if self.DisplayMsg.WaterExchangeActive:
                    self.DisplayMsg.WaterExchangeActive = 0
                else:
                    self.DisplayMsg.WaterExchangeActive = 1
                print('w Key was pressed new WaterExchangeActive {0:01d}'.format(self.DisplayMsg.WaterExchangeActive))
                sleep(0.1)
            elif keyboard.is_pressed('m'):
                if self.DisplayMsg.AutomaticallySwitchedToTapWater:
                    self.DisplayMsg.AutomaticallySwitchedToTapWater = 0
                else:
                    self.DisplayMsg.AutomaticallySwitchedToTapWater = 1
                print('m Key was pressed new AutomaticallySwitchedToTapWater {0:01d}'.format(
                    self.DisplayMsg.AutomaticallySwitchedToTapWater))
                sleep(0.1)
            else:
                sleep(0.01)


if __name__ == '__main__':
    test = RaincenterSimulator()
    test.execute()
