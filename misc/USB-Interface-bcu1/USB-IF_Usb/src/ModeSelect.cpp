/*
 *  ModeSelect.cpp - Manual select of the interface mode
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "chip.h"
#include "busdevice_if.h"
#include "ModeSelect.h"
#include "nxp/libeeprom-v3/libeeprom.h"

#define MODELED_HPRD 25

ModeSelect modeSelect;

typedef struct {
  int port;
  int pin;
} T_PinDef;

const T_PinDef LedPins[MODENUM] =
{
    {1, 16},
    {1, 22},
    {1, 15},
    {0, 17}
};

const T_PinDef ButtonPin = {1, 25};

int32_t ledCount()
{
    return static_cast<int32_t>(sizeof(LedPins)/sizeof(LedPins[0]));
}

ModeSelect::ModeSelect()
{
    mode_new = -1;
    mode_act = -1;
    Chip_IOCON_PinMuxSet(LPC_IOCON, ButtonPin.port, ButtonPin.pin, IOCON_FUNC1 | IOCON_MODE_PULLUP);
    for (int32_t i = 0; i < ledCount(); i++)
    {
        Chip_GPIO_SetPinDIROutput(LPC_GPIO, LedPins[i].port, LedPins[i].pin);
        SetSingleLed(i, false);
    }
}

void ModeSelect::SetSingleLed(int mode, bool OnOff)
{
    Chip_GPIO_SetPinState(LPC_GPIO, LedPins[mode].port, LedPins[mode].pin, !OnOff);
}

bool ModeSelect::ReadButton(void)
{
   return Chip_GPIO_GetPinState(LPC_GPIO, ButtonPin.port, ButtonPin.pin) == false;
}

void ModeSelect::SetLeds(void)
{
    for (int32_t i = 0; i < ledCount(); i++)
    {
        SetSingleLed(i, i == mode_new);
    }
}
}

TCdcDeviceMode ModeSelect::DeviceMode(void)
{
    switch (mode_act)
    {
        case 0:
          return TCdcDeviceMode::HidOnly;
        case 1:
          return TCdcDeviceMode::BusMon;
        case 2:
          return TCdcDeviceMode::UsbMon;
        case 3:
          return TCdcDeviceMode::ProgUserChip;
    }
    return TCdcDeviceMode::HidOnly;
}

void ModeSelect::StartModeSelect(void)
{
  mode_new = 0;
  mode_act = 0;
  blinkcnt = 0;
  timecnt = 0;

  EELIB_Command command;
  EELIB_Result result;
  uint8_t buf8;


  button_state = ReadButton();
  button_debounce = 0;

  command[0] = EELIB_IAP_COMMAND_EEPROM_READ;
  command[1] = 0; // Eeprom address
  command[2] = (uint32_t) &buf8;
  command[3] = 1; // Read length
  command[4] = SystemCoreClock / 1000;
  EELIB_entry(command, result);

  if (result[0] != EELIB_IAP_STATUS_CMD_SUCCESS) {
    buf8 = 0;
  }

  if (buf8 >= ledCount())
    buf8 = 0;
  mode_new = buf8;
  mode_act = buf8;
  SetLeds();
}

// alle 10 ms aufrufen
bool ModeSelect::DoModeSelect(void)
{
  if (ButtonDebounce())
  {
    mode_new = mode_new+1;
    if (mode_new >= ledCount())
      mode_new = 0;
    blinkcnt = 7;
    timecnt = MODELED_HPRD;
    SetLeds();
  } else if (blinkcnt != 0)
  {
    if (timecnt > 0)
      timecnt--;
    if (timecnt == 0)
    {
      blinkcnt--;
      if (blinkcnt > 0)
      {
        timecnt = MODELED_HPRD;
        SetSingleLed(mode_new, (blinkcnt & 1) != 0);
      } else {
        if (mode_act != mode_new)
        {
          mode_act = mode_new;
          SetSingleLed(mode_new, true);
          // Neuen Modus noch abspeichern.
          uint32_t command[5], result[4];
          uint8_t buf8 = mode_act;
          command[0] = EELIB_IAP_COMMAND_EEPROM_WRITE;
          command[1] = 0; // Eeprom address
          command[2] = (uint32_t) &buf8;
          command[3] = 1; // Write length
          command[4] = SystemCoreClock / 1000;
          EELIB_entry(command, result);
          if (result[0] != EELIB_IAP_STATUS_CMD_SUCCESS) {
            // doof das...
          }

          return true;
        }
      }
    } else {
      // nichts zu tun
    }
  } else {
    // nichts zu tun
  }
  return false;
}

// Gehen wir mal von Aufrufen alle 10ms aus
#define BUTTONDOWNDELAY 5 // 50ms
#define BUTTONUPDELAY 5 // 50ms

bool ModeSelect::ButtonDebounce(void)
{
  bool act_button = ReadButton();
  bool btndnevent = false;
  if (button_state)
  {
    if (act_button)
    {
      button_debounce = 0;
    } else {
      if (button_debounce >= BUTTONUPDELAY)
      {
        // Beim Loslassen der Taste keine weiteren Aktionen
        button_state = false;
        button_debounce = 0;
      } else {
        button_debounce++;
      }
    }
  } else {
    if (act_button)
    {
      if (button_debounce >= BUTTONDOWNDELAY)
      {
        // Flag setzen
        btndnevent = true;
        button_state = true;
        button_debounce = 0;
      } else {
        button_debounce++;
      }
    } else {
      button_debounce = 0;
    }
  }
  return btndnevent;
}

