
#include <sblib/eibMASK0701.h>
#include "common.h"

const unsigned char hardwareVersion[] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x4f};

// configuration
int sensorPin = PIO3_2; // IO8
uint32_t startTime, lifeFreq, lifeTime, cycleFreq, cycleTime, rainDelay, dryDelay;
bool sendOnchange, raining;
int sensCnt; // for debouncing

MASK0701 bcu = MASK0701();

APP_VERSION("SBrain  ", "1", "00")

BcuBase* setup() {
    // reset init stuff
    startTime = 0;
    lifeFreq = 0;
    sendOnchange = false;
    cycleFreq = 0;
    rainDelay = 0;
    dryDelay = 0;
    // state
    raining = false;
    sensCnt = 0;
    cycleTime = 0;
    lifeTime = 0;

#if LOGGING
    initLogger();
#endif
    bcu.begin(0x83, hardwareVersion[5], 0x11); // MDT SCN-RS1R1.01 rain sensor
    bcu.setHardwareType(hardwareVersion, sizeof(hardwareVersion));

    if (EE_STARTUP_TIMEOUT <= 60) {
        startTime = millis() + (EE_STARTUP_TIMEOUT * 1000);
        LOG("Start operating after %d seconds", EE_STARTUP_TIMEOUT);
    }

#ifdef LOGGING
    const char *label = "Reporting presense every %s";
#endif
    switch (EE_CYCLIC_SEND_OPERATING_TELEGRAM) {
        case EN_INBETRIEB_10_MIN:
            lifeFreq = 10 * 60000;
            LOG(label, "10 minutes");
            break;
        case EN_INBETRIEB_30_MIN:
            lifeFreq = 30 * 60000;
            LOG(label, "30 minutes");
            break;
        case EN_INBETRIEB_1_H:
            lifeFreq = 60 * 60000;
            LOG(label, "hour");
            break;
        case EN_INBETRIEB_3_H:
            lifeFreq = 3 * 60 * 60000;
            LOG(label, "3 hours");
            break;
        case EN_INBETRIEB_6_H:
            lifeFreq = 6 * 60 * 60000;
            LOG(label, "6 hours");
            break;
        case EN_INBETRIEB_12_H:
            lifeFreq = 12 * 60 * 60000;
            LOG(label, "12 hours");
            break;
        case EN_INBETRIEB_24_H:
            lifeFreq = 24 * 60 * 60000;
            LOG(label, "24 hours");
            break;
    }

    if (!EE_SEND_OBJECT_RAIN) {
        LOG("This Sensor is ignored");
    } else {
        if (EE_SEND_OBJECT_RAIN & EN_SENDRS_AT_CHANGES) {
            LOG("Triggering on change");
            sendOnchange = true;
        }
        if (EE_SEND_OBJECT_RAIN & EN_SENDRS_CYCLIC) {
#ifdef LOGGING
            label = "Reporting rain every %s";
#endif
            switch (EE_TIME_FOR_CYCLIC_SENDING_SEND_OBJECT_RAIN) {
                case EN_TIMERS_10_SEC:
                    cycleFreq = 10000;
                    LOG(label, "10 seconds");
                    break;
                case EN_TIMERS_30_SEC:
                    cycleFreq = 30000;
                    LOG(label, "30 seconds");
                    break;
                case EN_TIMERS_1_MIN:
                    cycleFreq = 1 * 60000;
                    LOG(label, "minute");
                    break;
                case EN_TIMERS_5_MIN:
                    cycleFreq = 5 * 60000;
                    LOG(label, "5 minutes");
                    break;
                case EN_TIMERS_30_MIN:
                    cycleFreq = 30 * 60000;
                    LOG(label, "half hour");
                    break;
                case EN_TIMERS_60_MIN:
                    cycleFreq = 60 * 60000;
                    LOG(label, "hour");
                    break;
            }
        }
    }
/*
    switch(EE_SENSITIVITY_OF_RAIN_SENSOR) {
        case EN_SETUPRM_LOW:
        case EN_SETUPRM_HIGH:
        case EN_SETUPRM_VERY_HIGH:
            LOG("This Sensor does not implement sensitivity");
        break;
    }

    switch(EE_INFO_OBJECT_FOR_HEATING_IS_ACTIVE) {
        case EN_INFOHEIZUNG_NO_SENDING_POSSIBLE_REQUEST:
        case EN_INFOHEIZUNG_SEND_AT_CHANGES:
            LOG("This Sensor does not implement heating status");
            break;
    }
*/
    if(EE_DELAY_FOR_MESSAGE_RAIN_ON <= 60) {
        rainDelay = 1000 * EE_DELAY_FOR_MESSAGE_RAIN_ON;
        LOG("Delaying the rain call by %d seconds",
                EE_DELAY_FOR_MESSAGE_RAIN_ON);
    }

#ifdef LOGGING
    label = "Delaying the rain stopped call by %s";
#endif
    switch(EE_DELAY_FOR_MESSAGE_RAIN_OFF) {
        case EN_OFFDELAY_10_SEC:
            dryDelay = 10000;
            LOG(label, "10 seconds");
            break;
        case EN_OFFDELAY_30_SEC:
            dryDelay = 30000;
            LOG(label, "30 seconds");
            break;
        case EN_OFFDELAY_1_MIN:
            dryDelay = 60000;
            LOG(label, "1 minute");
            break;
        case EN_OFFDELAY_2_MIN:
            dryDelay = 2 * 60000;
            LOG(label, "2 minutes");
            break;
        case EN_OFFDELAY_3_MIN:
            dryDelay = 3 * 60000;
            LOG(label, "3 minutes");
            break;
        case EN_OFFDELAY_5_MIN:
            dryDelay = 5 * 60000;
            LOG(label, "5 minutes");
            break;
        case EN_OFFDELAY_10_MIN:
            dryDelay = 10 * 60000;
            LOG(label, "10 minutes");
            break;
        case EN_OFFDELAY_15_MIN:
            dryDelay = 15 * 60000;
            LOG(label, "15 minutes");
            break;
        case EN_OFFDELAY_20_MIN:
            dryDelay = 20 * 60000;
            LOG(label, "20 minutes");
            break;
    }

    // set initial send times
    if (cycleFreq) {
        if (startTime) {
            cycleTime = startTime;
        } else {
            cycleTime = cycleFreq;
        }
    }
    if (lifeFreq) {
        if (startTime) {
            lifeTime = startTime;
        } else {
            lifeTime = lifeFreq;
        }
    }
    // let pin float, pull up will raise
    pinMode(sensorPin, INPUT | PULL_DOWN);
    return (&bcu);
}

void checkSensor() {
    bool rains = digitalRead(sensorPin);
    if (rains == raining) {
        sensCnt = 0;
        return; // no changes
    }
    // cnt needs to be n for debouncing
    sensCnt++;
    if (sensCnt <= 5) {
        LOG("having rain: %d take %d", rains, sensCnt);
        return;
    }
    if (sendOnchange) {
        uint32_t when = millis() + (rains? rainDelay : dryDelay);
        if (cycleTime > when) cycleTime = when;
        LOG("detected rain: %d", rains);
    }
    // update current state
    raining = rains;
}

void doPeriodics() {
    uint32_t now = millis();
    // check periodics
    if (cycleTime && cycleTime < now) {
        // send it on the bus
        LOG("Sending rain: %d", raining);
        bcu.comObjects->objectWrite(COM_RAIN, raining);
        if (cycleFreq) {
            cycleTime = now - (now % cycleFreq) + cycleFreq;
        } else {
            cycleTime = 0;
        }
    }
    if (lifeTime && lifeTime < now) {
        // send it on the bus
        LOG("Sending life signs");
        bcu.comObjects->objectWrite(COM_OPERATING, 1);
        if (lifeFreq) {
            lifeTime = now - (now % lifeFreq) + lifeFreq;
        } else {
            lifeTime = 0;
        }
    }
}

void loop() {
    if (startTime <= millis()) {
        // any changes?
        checkSensor();
        doPeriodics();
    }
    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle()) waitForInterrupt();
}
