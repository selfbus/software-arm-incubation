/*
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *                2021 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef HAND_ACTUATION_H_
#define HAND_ACTUATION_H_

#include <sblib/timeout.h>

class HandActuation
{
public:
    enum ButtonState {NO_ACTUATION = -1, BUTTON_PRESSED = 0x100, BUTTON_RELEASED = 0x200};
    HandActuation() = delete;
    HandActuation(const unsigned int* pins, const unsigned int pinCount, const unsigned int readbackPin, const unsigned int blinkTimeMs);
    ~HandActuation() = default;

    /**
     * Checks whether a button is pressed or was released
     *
     * @param btnNumber Button which is pressed or was release
     * @param btnState  Button state (pressed or released)
     * @return True in case a button is pressed or was released, otherwise false
     */
    bool getButtonAndState(int& btnNumber, HandActuation::ButtonState& btnState);
    bool ledState(unsigned int led);
    bool blinkState(unsigned int led);
    void setLedState(unsigned int led, bool state, bool blinking = false);
    void setallLedState(bool state);

    unsigned int getDelayBetweenButtonsMs();
    unsigned int getDelayAtEndMs();
    void setDelayBetweenButtonsMs(unsigned int newDelayBetweenButtonsMs);
    void setDelayAtEndMs(unsigned int newDelayAtEndMs);

    /**
     * @brief Simple test of the hand actuation pins.
     *        This test toggles the pins, waits blinkTimeMs and toggles the pins again.
     *
     * @param testPins     Array of pins to test
     * @param pinCount     Number of pins in testPins
     * @param blinkTimeMs  Time in milliseconds the test should take
     */
    static void testIO(const unsigned int* testPins, const unsigned int pinCount, const unsigned int blinkTimeMs);

protected:
    int check(void);
    unsigned int getHandPinCount();
    static Timeout blinkTimer;
    static bool blinkOnOffState;
    unsigned int pinCount_;
    unsigned int readbackPin_;
    unsigned int blinkTimeMs_;
    unsigned int  number_;
    unsigned char mask_;
    unsigned char buttonState_;
    unsigned char ledState_;
    unsigned char blinkState_;
    unsigned int delayBetweenButtonsMs_;
    unsigned int delayAtEndMs_;
    Timeout handDelay_;
    unsigned int* handPins_;
private:

};

#endif /* HAND_ACTUATION_H_ */
