/*
 *  outputsBoStable.cpp - Handle the update of the port pins in accordance to the
 *                need of the application
 *
 *  Copyright (C) 2014-2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef OUTPUTSBISTABLE_H_
#define OUTPUTSBISTABLE_H_

#include "outputs.h"

#define ON_DELAY 15 // Hongfa HFE20-1 24-1HSD-L2(359) datasheet
                    // says "Pulse Duration min. 50ms"
                    // tested on 12/02/2020 ON_DELAY set to:
                    //  4ms, 0 of 8 relays work
                    //  5ms, 2 of 8 relays work
                    //  6ms, 6 of 8 relays work
                    //  7ms, 7 of 8 relays work
                    //  8ms, 8 of 8 relays work
                    //  9ms, 8 of 8 relays work
                    // 10ms, 8 of 8 relays work
                    // 15ms, 8 of 8 relays work
/*
 * order code Hongfa HFE20-1 24-1HSD-L2(359)
 *    1:      5mm pin
 *   24:     24VDC
 *   1H:     1 Form A
 *    S:      Plastic sealed
 *    D:      W+AgSnO2 Contact
 *   L2:     Double coils latching
 *    -:    Positive polarity
* (359):  lamp load
 */


class OutputsBiStable : public Outputs
{
public:
	OutputsBiStable() : Outputs() {};

    virtual void updateOutputs(void);
    virtual void checkPWM(void);
};

#ifdef BI_STABLE
extern OutputsBiStable relays;
#endif

#endif /* OUTPUTSBISTABLE_H_ */
