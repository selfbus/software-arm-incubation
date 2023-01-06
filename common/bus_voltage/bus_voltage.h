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
 *      for 4TE-ARM & TS_ARM the voltage divider on the AD-Pin is R3 (91K0) and R12 (10K0).
 *      Use resistors with 1% tolerance or better for these.
 *
 *  Usage:
 *
 *      Try to not use functions from sblib/analog_pin.h, because they will not work well together without changes to analog_pin.h
 *      In case you need these functions, a restart of the monitoring with
 *      busVoltageMonitor.enable();
 *      is mandatory, or otherwise monitoring will stop.
 *
 *      implement a sub-class of BusVoltageCallback in your app_main.h or app_main.cpp to get notifications of bus voltage fail/return. e.g.:
 *      -------------------------------------------------------------------------------------------------------------------------------------

        include "bus_voltage.h"

        class MySubBusVoltageCallback public BusVoltageCallback {
        public:
            virtual void BusVoltageFail();
            virtual void BusVoltageReturn();
            virtual int convertADmV(int valueAD);
            virtual int convertmVAD(int valuemV);
        };

        void MySubBusVoltageCallback::BusVoltageFail()
        {
            // return as fast as possible from this method, because its called directly from the ADC interrupt service routine
            // and avoid calling other functions which depend on interrupts
            e.g.
            .... your actions on bus voltage fail ....
            bcu.end();
        }

        void MySubBusVoltageCallback::BusVoltageReturn()
        {
            // return as fast as possible from this method, because its called directly from the ADC interrupt service routine
            // and avoid calling other functions which depend on interrupts
            .... your actions on bus voltage return ....
            e.g.
            bcu.begin(...);
        }

        int MySubBusVoltageCallback::convertADmV(int valueAD)
        {
            // provide a function or polynomial 2th degree (or better) to convert AD values to millivoltages of the bus voltage
            return valueAD;
        }

        int MySubBusVoltageCallback::convertmVAD(int valuemV)
        {
            // provide a function or polynomial 2th degree (or better) to convert millivoltages of the bus voltage to AD values
            return valuemV;
        }

        MySubBusVoltageCallback callback; // instance of MySubBusVoltageCallback a sub-class of class BusVoltageCallback to receive notifications

 *
 *
 *      start bus voltage monitoring in your app_main.cpp, e.g.:
 *
 *
        void setup()
        {
            .... your code ....
            if (busVoltageMonitor.setup(VBUS_AD_PIN, VBUS_AD_CHANNEL, VBUS_ADC_SAMPLE_FREQ,
                                VBUS_THRESHOLD_FAILED, VBUS_THRESHOLD_RETURN,
                                VBUS_VOLTAGE_FAILTIME_MS, VBUS_VOLTAGE_RETURNTIME_MS,
                                &timer32_0, 0, &callback)
            // setup & enable bus voltage monitoring
            if (busVoltageMonitor.setup(PIN_VBUS,   // IO-Pin used for bus voltage monitoring
                                        AD7,        // AD-Channel correspondending to the IO-Pin
                                        10000,      // AD-conversion frequency in Hz
                                        20000,      // Threshold for bus failing in milli-voltage
                                        21000,      // Threshold for bus returning in milli-voltage
                                        20,         // time (milliseconds) bus voltage (milli-voltage) must stay below, to trigger bus failing notification
                                        1500,       // time (milliseconds) bus voltage (milli-voltage) must stay above, to trigger bus returning notification
                                        &timer32_0, // timer used for AD-conversion. Can be timer16_0 or timer32_0 from sblib/timer.h, timer can't be used otherwise!!
                                        0,          // timer match channel used for AD-conversion. Can be 0 or 1
                                        &callback)) // instance of a sub-class of class BusVoltageCallback
            {
                busVoltageMonitor.enable();
                while (busVoltageMonitor.busFailed())
                {
                  delay(1);
                }
            }
            .... your code ....
        }
 */

#ifndef BUS_VOLTAGE_H_
#define BUS_VOLTAGE_H_

#include <sblib/timer.h>
#include <sblib/ioports.h>
#include <sblib/io_pin_names.h>
#include <sblib/timer.h>

/*
 * implement a subclass of BusVoltageCallback in your application
 */
class BusVoltageCallback
{
public:
    virtual void BusVoltageFail() = 0;   // return as fast as possible from this method, because its called directly from the ADC interrupt service routine
                                         // and avoid calling other functions which depend on interrupts
    virtual void BusVoltageReturn() = 0; // return as fast as possible from this method, because its called directly from the ADC interrupt service routine
                                         // and avoid calling other functions which depend on interrupts
    virtual int convertADmV(int valueAD) = 0; // converts AD values to millivoltage
    virtual int convertmVAD(int valuemV) = 0; // converts millivoltage to AD values
};

class BusVoltage
{
public:
    enum State : unsigned int {FAILED = 0x00, OK = 0x01, FALLING = 0x10, RISING = 0x11, UNKNOWN = 0xFF};
    BusVoltage();
    /**
     * setup Bus Voltage Monitoring.
     *
     * @param ADPin IO-Pin used for bus voltage monitoring.
     * @param ADChannel AD-Channel correspondending to the IO-Pin
     * @param ADSampleFrequency AD-conversion frequency in Hz
     * @param thresholdVoltageFailed Threshold for bus failing in milli-voltage
     * @param thresholdVoltageReturn Threshold for bus returning in milli-voltage
     * @param busVoltageFailTimeMs time (milliseconds) bus voltage (milli-voltage) must stay below, to trigger bus failing notification
     * @param busVoltageReturnTimeMs time (milliseconds) bus voltage (milli-voltage) must stay above, to trigger bus returning notification
     * @param *adTimer timer used for AD-conversion. Can be timer16_0 or timer32_0 from sblib/timer.h
     * @param timerMatchChannel timer match channel used for AD-conversion. Can be 0 or 1
     * @param callback instance of a sub-class of class BusVoltageCallback
     *
     * @return non-zero if successful.
     */
    unsigned int setup(unsigned int ADPin, unsigned int ADChannel, unsigned int ADSampleFrequency,
                       unsigned int thresholdVoltageFailed, unsigned int thresholdVoltageReturn,
                       unsigned int busVoltageFailTimeMs, unsigned int busVoltageReturnTimeMs,
                       Timer *adTimer, unsigned int timerMatchChannel,
                       BusVoltageCallback *callback);
    void enable();
    void disable();
    bool busFailed();
    int valueBusVoltagemV(); // returns measured bus voltage in mV (-1 if measurement is invalid)
    int valueBusVoltageAD(); // returns measured bus voltage in AD (-1 if measurement is invalid)
    void adcInterruptHandler(); // This method is called by the ADC interrupt handler. Consider it to be a private method and do not call it.

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
    BusVoltageCallback *_callback;

    Timer *_adTimer; // a timer is needed to periodically start the ADC (16 or 32 bit).
    unsigned int _timerMatchChannel;

    // careful, variables can be changed any time by the Isr ADC_IRQHandler
    volatile int isrbusVoltagethresholdFailed;      // AD-value threshold below which a bus failure should be reported
    volatile int isrbusVoltagethresholdReturn;      // AD-value threshold above which a bus return should be reported
    volatile int isrmeanbusVoltage;                 // mean bus voltage as AD-value
    volatile unsigned int isrADSampleCount;
    volatile unsigned int isrFailedFirstSysTick;
    volatile unsigned int isrReturnedFirstSysTick;
    volatile BusVoltage::State isrBusVoltageState;
};

#define ADC_INTERRUPT_HANDLER(handler, busVoltageMonitorObj) \
    extern "C" void handler() { busVoltageMonitor.adcInterruptHandler(); }

extern BusVoltage busVoltageMonitor; // declared in bus_voltage.cpp, use only this instance for access

#endif /* BUS_VOLTAGE_H_ */
