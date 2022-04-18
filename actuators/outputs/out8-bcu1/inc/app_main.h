/*
 *  app_main.h - The application's main header file.
 *
 *  Copyright (c) 2021 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INC_APP_MAIN_H_
#define INC_APP_MAIN_H_

#ifdef BUSFAIL
#    include "bus_voltage.h"
#    include "app_nov_settings.h"
#endif



#ifdef BUSFAIL
typedef struct ApplicationData {
    unsigned char relaisstate;         // current relays state
} ApplicationData;

class AppCallback: public BusVoltageCallback {
public:
    virtual void BusVoltageFail();
    virtual void BusVoltageReturn();
    virtual int convertADmV(int valueAD);
    virtual int convertmVAD(int valuemV);
};
#endif






#endif /* INC_APP_MAIN_H_ */
