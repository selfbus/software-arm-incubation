/*
 *  ModeSelect.h - Manual select of the interface mode
 *
 *  Copyright (C) 2018 Florian Voelzke <fvoelzke@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef MODESELECT_H_
#define MODESELECT_H_

#define MODENUM 4 //!> Number of operation modes/leds

class ModeSelect;

extern ModeSelect modeSelect;

class ModeSelect {
public:
    ModeSelect();
    void StartModeSelect(void); // Liest den Tastenzustand ein, liest Modus aus EEPROM, aktiviert LED, liefert Modus zurück

    /**
     * Handles mode selection and updates system behavior accordingly.
     *
     * @details This function processes button presses to cycle through modes, updates the LEDs
     *          to reflect the selected mode and saves the selected mode to flash.
     *
     * @return true if the mode was changed; otherwise false.
     *
     * @note Must be called in a fixed time interval, e.g. every 10ms
     */
    bool DoModeSelect(void);

    DeviceMode getDeviceMode(void);
    void SetLeds(void); // Setzt die LED-Ausgänge
    void setAllLeds(bool on);

private:
    bool ButtonDebounce(void);
    void SetSingleLed(int mode, bool OnOff);
    void UpdateLeds(int newstates); // Weist den LEDs einen neuen Zustand zu und gibt sie aus bei Änderung
    bool ReadButton(void);
    bool button_state;
    unsigned button_debounce;
    int mode_new;
    int mode_act;
    int blinkcnt;
    int timecnt;
};

#endif /* MANUALCTRL_H_ */
