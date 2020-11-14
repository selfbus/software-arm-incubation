/*
 *  bus_voltage.h - simple ADC ISR monitoring of the bus voltage on the vref-pin of the LPCxx
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *  Don't create instances of class BusVoltage, use variable vBus instead
 *
 *  Usage:
 *      implement & define:
 *      --------------------------------------------------------------------------------------------------------------*
 *
 *      #define VBUS_AD_CHANNEL AD7 // ARM-ADC Channel for measuring the bus voltage
 *      #define VBUS_THRESHOLD 1.94 // voltage threshold below which a bus failure should be reported
 *      void BusVoltageFail()       //  will be called by the ISR which handles the ADC interrupt for the bus voltage.
 *      void BusVoltageReturn()     //  will be called by the ISR which handles the ADC interrupt for the bus voltage.
 *
 *      --------------------------------------------------------------------------------------------------------------
 *      e.g. in your app_main.cpp:
 *
 *        #define VBUS_AD_CHANNEL AD7
 *        #define VBUS_THRESHOLD 1.94
 *
 *        void setup()
 *        {
 *            .... your code ....
 *
 *            // enable bus voltage monitoring on PIO1_11 & AD7 with 1.94V threshold
 *            vBus.enableBusVRefMonitoring(PIN_VBUS, VBUS_AD_CHANNEL, VBUS_THRESHOLD);
 *
 *            .... your code ....
 *        }
 *
 *        void loop()
 *        {
 *            .... your code ....
 *
 *            // the bus voltage check, should be done before waitForInterrupt()
 *            vBus.checkPeriodic();
 *            if (bus.idle())
 *            {
 *                waitForInterrupt();
 *            }
 *        }
 *
 *        void BusVoltageFail()       //  will be called by the ISR in case of bus voltage failure.
 *        {
 *            .... your code for a bus failure ....
 *        }
 *
 *        void BusVoltageReturn()     //  will be called by the ISR in case of bus voltage return.
 *        {
 *            .... your code for a bus return ....
 *        }
 *      --------------------------------
 *
 *  1.94V @ ADC-Pin of the LPC11xx is just selected for fast testing, needs further investigation
 *  depend's on used controller e.g.
 *  4TE-ARM the voltage divider is R3/R12 (91K0 & 10K0)
 *  TS_ARM  the voltage divider is R3/R12 (91K0 & 10K0)
 *
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
    int valuemV(); // returns measured bus voltage in mV (-1 if measurement isnt startet)
protected:

private:
    unsigned int ADPin_;
    bool pendingRead();
    void startBusVoltageRead();
    void enableADCIsr(void);
    void resetIsrData();
};

extern BusVoltage vBus; // declared in bus_voltage.cpp, use only this instance for acccess

#endif /* BUS_VOLTAGE_H_ */
