/*
 *  app_main.cpp - The application's main.
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <sblib/eibBCU1.h>
#include <sblib/io_pin_names.h>
#include <sblib/timeout.h>
#include "config.h"
#include "GenFifo.h"
#include "BufferMgr.h"
#include "UartIf.h"
#include "emi_knx.h"
#include "device_mgnt.h"
#include "device_mgnt_const.h"
#include "prog_uart.h"

BCU1 bcu = BCU1();

APP_VERSION("SBif_knx", "1", "10") // Don't forget to also change the build-variable sw_version

BcuBase* setup()
{
    pinMode(PIN_PROG, OUTPUT);
    digitalWrite(PIN_PROG, false);
    pinMode(PIO1_5, OUTPUT);
    digitalWrite(PIO1_5, false);
    delay(500);
    digitalWrite(PIN_PROG, true);
    digitalWrite(PIO1_5, true);
    bcu.begin(2, 1, 1); // ABB, dummy something device
    emiknxif.reset(); // Set bcu in download mode to disable all KNX bus communication
    uart.Init(C_Dev_Baurate, false, PinSerialTx, PinSerialRx);
    return (&bcu);
}

/*
 * The main processing loop.
 */
void loop()
{
  uart.SerIf_Tasks(); // inter-mcu transmitting
  // emiknxif.SetCdcMonMode(true); //only for debugging to "hard" activate serial busmonitor mode
  emiknxif.EmiIf_Tasks();
  proguart.SerIf_Tasks();
  devicemgnt.DevMgnt_Tasks();
  // Sleep until the next 1 msec timer interrupt occurs (or shorter)
  __WFI();
}

void loop_noapp()
{
  loop();
}
