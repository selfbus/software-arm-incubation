//
// Created by mario on 17.05.20.
//
#include "common.h"

// Enable the Parasite power (check ds18x20 Datasheet )
#define PARASITE_POWER  false

// where the sensors are hanging .... "on"
int sensorPins[] = {PIO2_2, PIO2_3, PIO2_4, PIO2_5};

// a usable alternative over the stupid 130ms
#define TIME_BASE_MS 125u

// specifies what schedule is set to when deltas or thresholds are set.
#define DEFAULT_SAMPLE_PERIOD 1000

// easy to read on the bus
#define INVALID_TEMPERATURE (int16_t)0xdead

// Gaussian filter for temp readings
uint Filter::getVariance(int num) {
    // all good we don't know
    uint var = 0;
    int last = 0, pos = cnt % POOL_SIZE;
    if (cnt) {
        var = abs(avg - num);
        last = pool[pos];
    }
    pool[pos] = num;
    sum += num;
    if (cnt < POOL_SIZE) {
        avg = sum / (cnt+1);
    } else {
        sum -= last;
        avg = sum / POOL_SIZE;
    }
    //LOG("num: %d avg: %d cnt: %d pos: %d sum: %d", num, avg, cnt, pos, sum);
    cnt++;
    return var;
}

// Initializers
bool SensorConfig::init(int sensorIdx, uint sensorType, int dpt,
                        int8_t tempOffset, COM comObject) {
    sensorNum = sensorIdx + 1;
    LOG("Initializing Sensor %d", sensorNum);
    ds.DS18x20Init(sensorPins[sensorIdx], PARASITE_POWER);
    // Scan the 1-Wire bus for DS18x devices
    for(int8_t tries = 1; tries <= 3; tries++) {
        uint8_t s = ds.Search();
        if (s) {
            LOG("#%d Try %d no devices found. Ec: %d",
                    sensorNum, tries, s);
            delay(50); // maybe bus settling?
            continue;
        }
        break;
    }
    if (!ds.m_foundDevices) return false;

    for (int i = 0; i < ds.m_foundDevices; i++) {
        LOG("#%d found a %s device", sensorNum, ds.TypeStr(i));
    }
    type = sensorType;
    resolution = dpt;
    if (dpt == SENDETYP_MW_DPT5_EIS6_NON_STANDARD_COMPLIANT) {
        LOG("#%d using non-standard DPT5 resolution", sensorNum);
    }
    offset = (int16_t)tempOffset * 1.6;
    // a good invalid start value ;)
    temperature = lastTrigger = INVALID_TEMPERATURE;
    LOG("#%d using a temperature offset of %dd°", sensorNum, tempOffset);
    comObj = comObject;
    return true;
}

void SensorConfig::setDiffTrigger(uint trigger) {
    diffTrigger = trigger * 1.6;
    schedule = DEFAULT_SAMPLE_PERIOD;
    LOG("#%d will send if temperature varies by more than %dd°", sensorNum, trigger);
}

void SensorConfig::setSendPeriod(int sendAtStart, int sendThen, uint startFactor,
        uint timeFactor, uint timeBase) {

    int tb = TIME_BASE_MS << timeBase;
    uint now = millis();
    if (sendThen == JA_NEIN_YES) {
        sendFreq = tb * timeFactor;
        LOG("#%d will send temperature updates every %dms", sensorNum, sendFreq);
        // schedule the first send
        sendTime = now - (now % sendFreq) + sendFreq;
    }
    // do this one second so sendTime overrides the period one
    if(sendAtStart == INIT_MW_SENT_WITH_DELAY) {
        sendStart = tb * startFactor;
        LOG("#%d will send temperature %dms after start", sensorNum, sendStart);
        sendTime = now - (now % sendStart) + sendStart;
    }
}

void SensorConfig::setThresholdPeriod(int sendAtStart, int sendThen,
        uint startFactor, uint timeFactor, uint timeBase) {

    int tb = TIME_BASE_MS << timeBase;
    if (sendAtStart == INIT_GW_SEND_DELAYED) {
        thStart = tb * startFactor;
    } else if(sendAtStart == INIT_GW_SENT_IMMEDIATELY) {
        thStart = 1;
    }
    if (thStart) {
        thTime = thStart;
        LOG("#%d thresholds will send updates %dms after start", sensorNum, thStart);
    }
    if (sendThen == JA_NEIN_YES) {
        thFreq = tb * timeFactor;
        LOG("#%d thresholds will send updates every %dms", sensorNum, thFreq);
    }
}

void SensorConfig::setThreshold(uint num, uint triggerValue, uint triggerAction,
        COM comObject) {
    schedule = DEFAULT_SAMPLE_PERIOD;
    th[num].init(num, sensorNum, triggerValue, triggerAction, comObject);
}

// Runtime functions
void SensorConfig::setConvTime(uint when) {
    if (when) {
        uint now = millis() + DS18x20::leadTime;
        convTime = now - (now % when) + when - DS18x20::leadTime-100; // more slack
    } else {
        convTime = 0;
    }
}

void SensorConfig::sampleValues() {
    if (!isActive()) return;
    uint32_t now = millis();
    if (!converting && convTime < now) {
        ds.startConversion(0);
        readTime = convTime + DS18x20::leadTime;
        if (schedule) {
            setConvTime(schedule);
        } else {
            // only when sending periodically without thresholds or triggers
            setConvTime(sendFreq);
        }
        converting = true;
        //LOG("read: %d conv: %d send: %d", readTime, convTime, sendTime);
    }
}

void SensorConfig::readValues() {
    if (!isActive()) return;
    uint32_t now = millis();
    if (converting && readTime < now && ds.readResult(0)) {
        // Check if the last temperature read was OK
        converting = false;
        if (!ds.lastReadOk(0)) {
            LOG("#%d Ignoring invalid reading", sensorNum);
            return;
        }
#ifdef LOGGING
        int16_t oldTemp = pretty(temperature);
#endif
        int16_t temp = ds.raw(0) + offset;
        int var = pool.getVariance(temp);
        if (var < MAX_TEMP_VARIANCE) {
            temperature = temp;
        } else {
            LOG("#%d ignoring outlier temperature %dd° due to variance of %dd°",
                sensorNum, pretty(temp), pretty(var));
        }
#ifdef LOGGING
        int16_t ptemp = pretty(temperature);
        if (oldTemp != ptemp) {
            LOG("#%d read: %dd°", sensorNum, ptemp);
        }
#endif
        // test for variations
        if (diffTrigger != 0) {
            if (lastTrigger == INVALID_TEMPERATURE) {
                lastTrigger = temperature;
            } else {
                int thisDiff = abs(temperature - lastTrigger);
                if (thisDiff > diffTrigger) {
                    LOG("#%d temp: %dd° is more than %dd° from last temp: %dd°",
                        sensorNum, pretty(temperature),
                        pretty(diffTrigger), pretty(lastTrigger));
                    lastTrigger = temperature;
                    // send it on the bus
                    sendTemperature();
                }
            }
        }
        if (temperature == INVALID_TEMPERATURE) return;
        if (thTime && thTime < (now+1000)) {
            // sending this in a second oughta do too.
            return;
        }
        // test for changes
        for (auto &t : th) {
            t.test(temperature, false);
        }
    }
}

void SensorConfig::doPeriodics() {
    if (!isActive()) return;
    if (temperature == INVALID_TEMPERATURE) return;
    uint32_t now = millis();
    // check preiodics
    if (sendTime && sendTime < now) {
        // send it on the bus
        LOG("#%d sending temp: %dd°", sensorNum, pretty(temperature));
        sendTemperature();
        if (sendFreq) {
            sendTime = now - (now % sendFreq) + sendFreq;
        } else {
            sendTime = 0;
        }
    }
    // deal with the thresholds
    if (thTime && thTime < now) {
        // send no matter what
        for (auto &t : th) {
            t.test(temperature, true);
        }
        if (thFreq) {
            thTime = now - (now % thFreq) + thFreq;
        } else {
            thTime = 0;
        }
    }
}

void SensorConfig::sendTemperature() const {
    if (resolution == SENDETYP_MW_DPT9_EIS5_DEFAULT) {
        objectWrite(comObj, temperature);
    } else {
        objectWrite(comObj, (uint8_t)(temperature/16));
    }
}

