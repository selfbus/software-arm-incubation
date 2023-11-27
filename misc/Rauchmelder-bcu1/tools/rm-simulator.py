#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This program simulates a smoke alarm detector.

Start with "python rm-simulator.py /dev/ttyUSB0"

WARNING: most functionality is not yet implemented

Original written in perl:
 Copyright (c) 2013-2014 Stefan Haller
 Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 
Ported to python:
 Copyright (c) 2023 Darthyson <darth@maptrack.de>
"""

import sys
import time

import keyboard
import serial

NUL = 0x00
STX = 0x02
ETX = 0x03
ACK = 0x06
NAK = 0x15

# The default serial port to use
#SERIAL_PORT_DEFAULT = '/dev/ttyUSB0'
SERIAL_PORT_DEFAULT = 'COM6'
RX_TIMEOUT = 0.2

def convert_to_readable_format(s):
    result = ''
    for x in s:
        if x == NUL:
            result += "<NUL>"
        elif x == STX:
            result += "<STX>"
        elif x == ETX:
            result += "<ETX>"
        elif x == ACK:
            result += "<ACK>"
        elif x == NAK:
            result += "<NAK>"
        elif x >= 0x20:
            result += chr(x)
        else:
            result += ' 0x{0:02x}'.format(x)
    return result


def print_help():
    print("Keys: 1=smoke, 2=temp, 3=wired, 4=wireless, 5=test, 6=wired-test, 7=wireless-test, b=battery-low, m=mounting-fault, h=help, q=quit\n")


def decode_hex_string(hex_str):
    return [int(hex_str[i:i + 2], 16) for i in range(0, len(hex_str), 2)]


def hide_status():
    clear = ' ' * 80
    print("{}\r".format(clear), end='', flush=True)


class RMSimulator(object):
    def __init__(self):
        self.num_temp_alarms = 0
        self.num_smoke_alarms = 0
        self.num_test_alarms = 0
        self.num_wired_alarms = 0
        self.num_wireless_alarms = 0
        self.num_wired_test_alarms = 0
        self.num_wireless_test_alarms = 0

        self.status_battery_low = False
        self.status_mounting_fault = False
        self.status_temp_alarm = False
        self.status_smoke_alarm = False
        self.status_test_alarm = False
        self.status_wired_alarm = False
        self.status_wireless_alarm = False
        self.status_wired_test_alarm = False
        self.status_wireless_test_alarm = False

        self.start_time = time.time()
        self.bat_volt = 8.47
        self.pollution = 7
        self.temp1 = 22.5
        self.temp2 = 32.5
        self.smokebox_value_hex = "005C"
        self.serial_hex = "BADCAB1E"

        self.serial_port: serial.Serial = None
        self.read_active = False
        self.read_buf = ''

        self.debug = False

    def execute(self):
        print_help()
        keyboard.on_press(self.on_key_press, True)

        port_name = sys.argv[1] if len(sys.argv) > 1 else SERIAL_PORT_DEFAULT
        self.open_port(port_name)

        while True:
            bytes_to_read = self.serial_port.in_waiting
            if bytes_to_read > 0:
                serial_bytes = bytearray(self.serial_port.read(bytes_to_read))
                self.process_serial_bytes(serial_bytes)


    def on_key_press(self, e):
        if e.event_type != keyboard.KEY_DOWN:
            return True

        key = e.name
        if key == 'h':  # print help
            print_help()
        elif key == '1':  # toggle smoke alarm
            self.status_smoke_alarm = not self.status_smoke_alarm
            self.send_status_message(True)
        elif key ==  '2':  # toggle temp alarm
            self.status_temp_alarm = not self.status_temp_alarm
            self.send_status_message(True)
        elif key ==  '3':  # toggle wired alarm
            self.status_wired_alarm = not self.status_wired_alarm
            self.send_status_message(True)
        elif key ==  '4':  # toggle wireless alarm
            self.status_wireless_alarm = not self.status_wireless_alarm
            self.send_status_message(True)
        elif key ==  '5':  # toggle test alarm
            self.status_test_alarm = not self.status_test_alarm
            self.send_status_message(True)
        elif key ==  '6':  # toggle wired test alarm
            self.status_wired_test_alarm = not self.status_wired_test_alarm
            self.send_status_message(True)
        elif key ==  '7':  # toggle wireless test alarm
            self.status_wireless_test_alarm = not self.status_wireless_test_alarm
            self.send_status_message(True)
        elif key ==  'b':  # toggle battery low
            self.status_battery_low = not self.status_battery_low
            self.send_status_message(True)
        elif key ==  'm':  # toggle mounting fault
            self.status_mounting_fault = not self.status_mounting_fault
            self.send_status_message(True)
        elif key ==  'q':  # quit
            self.cleanup()
            exit(0)
        else:
            return True

        hide_status()
        self.show_status()
        return False


    def process_serial_bytes(self, msg):
        self.log_msg("RECV {0:s}".format(convert_to_readable_format(msg)), self.debug)
        for ch in msg:
            if ch == STX:
                #log_msg("RECV: <STX>", read_active)
                self.read_buf = ''
                self.read_active = True
            elif ch == ETX:
                self.read_active = False
                self.process_message(self.read_buf)
            elif ch == ACK:
                #log_msg("RECV: <ACK>")
                pass
            elif ch == NAK:
                #log_msg("RECV: <NAK>")
                pass
            elif ch == NUL:
                #log_msg("RECV: <NUL>")
                pass
            elif self.read_active:
                self.read_buf += chr(ch)
            else:
                self.log_msg("RECV: 0x{:02x} (out of bounds - ignored)".format(ch))

    @staticmethod
    def temperature_to_int(temp):
        return int((temp + 20.0)/0.50)

    def process_message(self, msg):
        self.log_msg("RECV: <STX>{}<ETX>".format(msg))
        if not self.validate_checksum(msg):
            return

        if msg[:-2] == "00TEST": # todo implement test command
            self.log_msg("Received test message (00TEST) -> NOT IMPLEMENTED")
            self.send_ack()
            return

        bytes_list = decode_hex_string(msg[:-2])

        cmd = bytes_list[0]
        if cmd == 0x02:  # query status
            self.log_msg("Received status query (0x02)")
            self.send_ack()
            self.send_status_message(False)
        elif cmd == 0x03: # control command # todo implement control command
            self.log_msg("Received control command (0x03) -> NOT IMPLEMENTED")
            self.send_ack()
        elif cmd == 0x04:  # query serial number
            self.log_msg("Received serial number query (0x04)")
            self.send_ack()
            self.send_hex_string('C4' + self.serial_hex)
        elif cmd == 0x09:  # query operating time
            self.log_msg("Received operating time query (0x09)")
            self.send_ack()
            self.send_hex_string('C9{:08X}'.format(int((time.time() - self.start_time) * 4)))
        elif cmd == 0x0B:  # query smokebox values
            self.log_msg("Received smokebox values query (0x0B)")
            self.send_ack()
            self.send_hex_string('CB{}{:02X}{:02X}'.format(self.smokebox_value_hex, self.num_smoke_alarms, self.pollution))
        elif cmd == 0x0C:  # query battery voltage and temperatures
            self.log_msg("Received battery voltage and temperatures query (0x0C)")
            self.send_ack()
            bat_encoded = int(self.bat_volt / 0.018369)
            self.send_hex_string('CC{:04X}{:02X}{:02X}'.format(bat_encoded, self.temperature_to_int(self.temp1), self.temperature_to_int(self.temp2)))
        elif cmd == 0x0D:  # query number of alarms
            self.log_msg("Received number of alarms query (0x0D)")
            self.send_ack()
            self.send_hex_string('CD{:02X}{:02X}{:02X}{:02X}'.format(self.num_temp_alarms, self.num_test_alarms,
                                                                  self.num_wired_alarms, self.num_wireless_alarms))
        elif cmd == 0x0E:  # query number of remote test alarms
            self.log_msg("Received number of remote test alarms query (0x0E)")
            self.send_ack()
            self.send_hex_string('CE{:02X}{:02X}'.format(self.num_wired_test_alarms, self.num_wireless_test_alarms))
        else:
            self.log_msg("Received unknown command {:02X}".format(cmd))
            self.send_nak()

    def send_status_message(self, automatic):
        response_byte = 0xC2
        if automatic:
            response_byte = 0x82

        status_byte_1 = 0x00
        status_byte_2 = 0x00
        status_byte_3 = 0x00 # TODO dig into status_byte_3, seen values 0x00 and 0x80
        status_byte_4 = 0x00

        if self.status_temp_alarm:
            #status_byte_1 |= 0x10
            status_byte_1 |= 0x02

        if self.status_battery_low:
            status_byte_2 |= 0x01
        if self.status_mounting_fault:
            status_byte_2 |= 0x02
        if self.status_smoke_alarm:
            status_byte_2 |= 0x04
        if self.status_wired_alarm:
            status_byte_2 |= 0x08
        if self.status_wireless_alarm:
            status_byte_2 |= 0x10
        if self.status_test_alarm:
            status_byte_2 |= 0x20
        if self.status_wired_test_alarm:
            status_byte_2 |= 0x40
        if self.status_wireless_test_alarm:
            status_byte_2 |= 0x80

        # TODO remove on release
        # Just for testing special status values
        #if self.status_battery_low:
        #    status_byte_1 = 0x02
        #    status_byte_2 = 0x00
        #    status_byte_3 = 0x00
        #    status_byte_4 = 0x04
        #else:
        #    status_byte_1 = 0x00
        #    status_byte_2 = 0x00
        #    status_byte_3 = 0x00
        #    status_byte_4 = 0x00

        self.send_hex_string("{:02X}{:02X}{:02X}{:02X}{:02X}".format(response_byte, status_byte_1, status_byte_2, status_byte_3, status_byte_4))

    def validate_checksum(self, hex_str):
        checksum = sum(map(ord, hex_str[:-2])) & 0xFF
        expected_checksum = int(hex_str[-2:], 16)
        if checksum != expected_checksum:
            self.log_msg("Checksum error: received {:02X}, expected {:02X}".format(expected_checksum, checksum))
            self.send_nak()
            return False
        return True


    def send_bytes(self, *bytes_list):
        hex_str = ''.join('{:02X}'.format(byte & 0xFF) for byte in bytes_list)
        self.send_hex_string(hex_str)


    def send_hex_string(self, hex_str):
        checksum = sum(map(ord, hex_str)) & 0xFF
        hex_str += '{:02X}'.format(checksum)
        self.log_msg("SEND: <NUL><STX>{}<ETX>".format(hex_str))
        self.send_byte(NUL)
        self.send_byte(STX)
        self.serial_port.write(hex_str.encode())
        self.send_byte(ETX)

    def send_byte(self, to_send):
        self.serial_port.write(chr(to_send).encode())

    def send_ack(self):
        self.log_msg("SEND: <ACK>")
        self.send_byte(ACK)


    def send_nak(self):
        self.log_msg("SEND: <NAK>")
        self.send_byte(NAK)


    def send_nul(self):
        self.log_msg("SEND: <NUL>")
        self.send_byte(NUL)


    def open_port(self, port_name):
        self.serial_port = serial.Serial(port_name, baudrate=9600, parity=serial.PARITY_NONE,
                      stopbits=serial.STOPBITS_ONE,
                      bytesize=serial.EIGHTBITS, timeout=RX_TIMEOUT, rtscts=False)
        self.serial_port.rts = False # set RTS to high
        self.log_msg("Listening on {}".format(self.serial_port.name))

    def log_msg(self, msg, is_debug=False):
        if is_debug and not self.debug:
            return
        time = datetime.datetime.now().strftime('%H:%M:%S.%f')
        log = "{0:s} {1:s}".format(time, msg)
        hide_status()
        print(log)
        self.show_status()


    def show_status(self):
        alarms = ''

        if self.status_smoke_alarm:
            alarms += 'SMOKE '
        if self.status_temp_alarm:
            alarms += 'TEMP '
        if self.status_wired_alarm:
            alarms += 'WIRED '
        if self.status_wireless_alarm:
            alarms += 'WIRELESS '
        if self.status_test_alarm:
            alarms += 'TEST '
        if self.status_wired_test_alarm:
            alarms += 'WIRED-TEST '
        if self.status_wireless_test_alarm:
            alarms += 'WIRELESS-TEST '
        if self.status_mounting_fault:
            alarms += 'MOUNTING-FAULT '
        if self.status_battery_low:
            alarms += 'BATTERY-LOW '

        if not alarms:
            alarms = '(none)'

        print("Alarms: {}\r".format(alarms), end='', flush=True)


    def cleanup(self):
        if self.serial_port.is_open:
            self.serial_port.close()


if __name__ == "__main__":
    simulator = RMSimulator()
    simulator.execute()
