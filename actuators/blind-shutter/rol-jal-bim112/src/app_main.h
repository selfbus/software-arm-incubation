/*
 *  app_main.h - The application's main header file.
 *
 *  Copyright (c) 2024 Oliver Stefan <o.stefan252@googlemail.com>
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
#    include "channel.h"
#endif



#ifdef BUSFAIL
typedef struct ApplicationData {
    short channelPositions[NO_OF_CHANNELS];
    short channelSlatPositions[NO_OF_CHANNELS];
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
