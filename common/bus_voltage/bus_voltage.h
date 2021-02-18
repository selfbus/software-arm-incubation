/*
 *  bus_voltage.h - simple ADC ISR monitoring of the bus voltage on the vref-pin of the LPCxx
 *
 *  Copyright (c) 2020 Darthyson <darth@maptrack.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  Don't create instances of class BusVoltage, use variable busVoltageMonitor instead
 *
 *  Notes:
 *      in the KNX-specification a bus fail is defined as a bus voltage drop below 20V with a hysteresis of 1V.
 *      for 4TE-ARM & TS_ARM the voltage divider is R3 (91K0) and R12 ((10K0)
 *      use resistors with 1% tolerance better
 *
 *  Usage:
 *      implement:
 *      -------------------------------------------------------------------------------------------------------------------------------------*
 *      void BusVoltageFail()        // will be called by busVoltageMonitor.checkPeriodic() in case the ADC-Isr detected a bus voltage fail
 *      void BusVoltageReturn()      // will be called by busVoltageMonitor.checkPeriodic() in case the ADC-Isr detects a bus voltage return
 *      int convertADmV(int valueAD) // converts measured AD value into milliVoltage
 *      int convertmVAD(int valuemV) // converts milliVoltage into AD value
 *      -------------------------------------------------------------------------------------------------------------------------------------
 *      e.g. in your app_main.cpp:
 *
 *        void setup()
 *        {
 *            .... your code ....
 *            // enable bus voltage monitoring
 *            if (busVoltageMonitor.setupMonitoring(PIN_VBUS, AD7, 10000,
 *                                                  20000, 21000,
 *                                                  20, 1500,
 *                                                  &timer32_0, 0))
 *            {
 *                busVoltageMonitor.enableMonitoring();
 *            }
 *
 *            .... your code ....
 *        }
 *
 *        void loop()
 *        {
 *            .... your code ....
 *            // the bus voltage check
 *            busVoltageMonitor.checkPeriodic();
 *            if (bus.idle())
 *            {
 *                waitForInterrupt();
 *            }
 *        }
 *
 *        void loop_noapp()
 *        {
 *            .... your code ....
 *            // check the bus voltage
 *            busVoltageMonitor.checkPeriodic();
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

#include <sblib/timer.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include <sblib/timer.h>

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
    enum State : unsigned int {FAILED = 0x00, OK = 0x01, FALLING = 0x10, RISING = 0x11, UNKNOWN = 0xFF};
    BusVoltage() : _ADPin(PIN_VBUS)
                 , _ADChannel(AD7)
                 , _ADSampleFrequency(10000)
                 , _thresholdVoltageFailed(20000)
                 , _thresholdVoltageReturn(21000)
                 , _busVoltageFailTimeMs(20)
                 , _busVoltageReturnTimeMs(1500)
                 , _timerMatchChannel(0)
                 {};
    unsigned int setupMonitoring(unsigned int ADPin, unsigned int ADChannel, unsigned int ADSampleFrequency,
                                 unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn,
                                 unsigned int busVoltageFailTimeMs, unsigned int busVoltageReturnTimeMs,
                                 Timer *adTimer, unsigned int timerMatchChannel);
    void enableMonitoring();
    void disableMonitoring();
    void checkPeriodic();
    bool failed();
    int valuemV(); // returns measured bus voltage in mV (-1 if measurement is invalid)

    void ADCInterruptHandler(); // This method is called by the ADC interrupt handler. Consider it to be a private method and do not call it.

    unsigned int getBusVoltageFailTimeMs() {return _busVoltageFailTimeMs;}
    unsigned int getBusVoltageReturnTimeMs() {return _busVoltageReturnTimeMs;}
    unsigned int getADChannel() {return _ADChannel;}
    unsigned int _ADCR;

protected:

private:
    void adcTimerSetup(void);
    void analogSetup();
    void isrSetup();

    unsigned int _ADPin;
    unsigned int _ADChannel;
    unsigned int _ADSampleFrequency;
    unsigned int _thresholdVoltageFailed;
    unsigned int _thresholdVoltageReturn;
    unsigned int _busVoltageFailTimeMs;
    unsigned int _busVoltageReturnTimeMs;

    Timer *_adTimer; // a timer is needed to periodically start the ADC (16 or 32 bit).
    unsigned int _timerMatchChannel;
};

#define ADC_INTERRUPT_HANDLER(handler, busVoltageMonitorObj) \
    extern "C" void handler() { busVoltageMonitor.ADCInterruptHandler(); }

extern BusVoltage busVoltageMonitor; // declared in bus_voltage.cpp, use only this instance for access

#endif /* BUS_VOLTAGE_H_ */
