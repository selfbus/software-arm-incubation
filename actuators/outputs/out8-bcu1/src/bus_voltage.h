/*
 *  bus_voltage.h - simple ADC ISR monitoring of the bus voltage on the vref-pin of the LPCxx
 *
 *  Don't create instances of class BusVoltage, use variable vBus instead
 *
 *  Implement in your apps main:
 *
 *  void BusVoltageFail()
 *  void BusVoltageReturn()
 *
 *  both will be called by the ISR which handles the ADC interrupt for the bus voltage.
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef BUS_VOLTAGE_H_
#define BUS_VOLTAGE_H_

#define WEAK __attribute__ ((weak))

WEAK void BusVoltageFail();
WEAK void BusVoltageReturn();

class BusVoltage
{
public:
    BusVoltage();
    void enableBusVRefMonitoring(int ADPin, int ADChannel, float thresholdVoltage);
    void disableBusVRefMonitoring();
    void checkPeriodic();
    bool failed();
protected:

private:
    unsigned int ADPin_;
    bool pendingRead();
    void startBusVoltageRead();
    void enableADCIsr(void);
};

extern BusVoltage vBus;

#endif /* BUS_VOLTAGE_H_ */
