/*
 *  bus_voltage.h - simple ADC ISR monitoring of the bus voltage on the vref-pin of the LPCxx
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  Don't create instances of class BusVoltage, use variable vBus instead
 *
 *  Notes:
 *      in the KNX-specification a bus fail is defined as a bus voltage drop below 20V with a hysteresis of 1V.
 *      4TE-ARM the voltage divider is R3/R12 (91K0 & 10K0)
 *      TS_ARM  the voltage divider is R3/R12 (91K0 & 10K0)
 *
 *  Usage:
 *      implement:
 *      --------------------------------------------------------------------------------------------------------------*
 *      void BusVoltageFail()        // will be called by the ISR which handles the ADC interrupt for the bus voltage.
 *      void BusVoltageReturn()      // will be called by the ISR which handles the ADC interrupt for the bus voltage.
 *      int convertADmV(int valueAD) // converts measured AD value into milliVoltage
 *      --------------------------------------------------------------------------------------------------------------
 *      e.g. in your app_main.cpp:
 *
 *        void setup()
 *        {
 *            .... your code ....
 *            // enable bus voltage monitoring
 *            vBus.enableBusVRefMonitoring(PIN_VBUS, AD7, 20000, 21000);
 *            .... your code ....
 *        }
 *
 *        void loop()
 *        {
 *            .... your code ....
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
 *
 *        int convertADmV(int valueAD)
 *        {
 *            // converts measured AD value into milliVoltage
 *
 *            // e.g. for a 4TE-ARM controller:
 *            // good approximation between 17 & 30V
 *            if (valueAD > 2150)
 *                return 30000;
 *            else if (valueAD < 1546)
 *                return 0;
 *            else
 *                return 0.0198*valueAD*valueAD - 52.104*valueAD + 50375;
 *
 *            4TE ARM-Controller coefficients found with following measurements:
 *            ---------------------
 *            | Bus mV  ADC-Value |
 *            ---------------------
 *            | 30284   2158      |
 *            | 30006   2150      |
 *            | 29421   2132      |
 *            | 27397   2073      |
 *            | 26270   2035      |
 *            | 25210   1996      |
 *            | 24094   1953      |
 *            | 22924   1903      |
 *            | 21081   1811      |
 *            | 20003   1751      |
 *            | 18954   1683      |
 *            | 17987   1619      |
 *            | 17007   1546      |
 *            ---------------------
 *        }
 */

#ifndef BUS_VOLTAGE_H_
#define BUS_VOLTAGE_H_

#define WEAK __attribute__ ((weak))

WEAK void BusVoltageFail();
WEAK void BusVoltageReturn();
WEAK int convertADmV(int valueAD);

class BusVoltage
{
public:
    BusVoltage();
    void enableBusVRefMonitoring(int ADPin, int ADChannel, unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn);
    void disableBusVRefMonitoring();
    void checkPeriodic();
    bool failed();
    int valuemV(); // returns measured bus voltage in mV (-1 if measurement is invalid)
protected:

private:
    unsigned int ADPin_;
    bool pendingRead();
    void startBusVoltageRead();
    void enableADCIsr(void);
    void resetIsrData();
};

extern BusVoltage vBus; // declared in bus_voltage.cpp, use only this instance for access

#endif /* BUS_VOLTAGE_H_ */
