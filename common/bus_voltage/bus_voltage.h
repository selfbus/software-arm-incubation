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
 *        void loop_noapp()
 *        {
 *            .... your code ....
 *            // check the bus voltage, should be done before waitForInterrupt()
 *            vBus.checkPeriodic();
 *            waitForInterrupt();
 *        }
 *
 *        void BusVoltageFail()       //  will be called by the ISR in case of bus voltage failure.
 *        {
 *            .... your code for a bus failure ....
 *            bcu.end();
 *        }
 *
 *        void BusVoltageReturn()     //  will be called by the ISR in case of bus voltage return.
 *        {
 *            .... your code for a bus return ....
 *            bcu.begin(...);
 *        }
 *
 *        int convertADmV(int valueAD)
 *        {
 *            // converts measured AD value into milliVoltage
 *
 *            // e.g. for a 4TE-ARM controller:
 *            // good approximation between 17 & 30V
 *            if (valueAD > 2158)
 *                return 30000;
 *            else if (valueAD < 1546)
 *                return 0;
 *            else
 *                return 0.019812094*sq(valueAD) - // a*x^2
 *                       52.1039160138*valueAD +   // b*x
 *                       50375.4168671156;         // c
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
 *
 *        int convertmVAD(int valuemV)
 *        {
 *            // good approximation between 17 & 30V for the 4TE-ARM controller
 *            if (valuemV >= 30284)
 *                return 2158;
 *            else if (valuemV < 17007)
 *                return 0;
 *            else
 *                return -0.0000018353*sq(valuemV) + // a*x^2
 *                        0.132020974*valuemV -      // b*x
 *                        161.7265204893;            // c
 *        }
 */

#ifndef BUS_VOLTAGE_H_
#define BUS_VOLTAGE_H_

/*
 * implement in your code
 * This function is called when the bus voltage drops below the thresholdVoltageFailed
 */
void BusVoltageFail(); // implement in your app, will be called should be overwritten in the apps main

/*
 * implement in your code
 * This function is called when the bus voltage raises over the thresholdVoltageReturn
 */
void BusVoltageReturn();

/*
 * implement in your code
 * This function is called to convert AD-values to millivolts
 */
int convertADmV(int valueAD);

/*
 * implement in your code
 * This function is called to convert millivolts to AD-values
 */
int convertmVAD(int valuemV);

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
