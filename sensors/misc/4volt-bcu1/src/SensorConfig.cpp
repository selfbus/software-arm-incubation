//
// Created by Mario Theodoridis on 17.05.20.
//
#include <sblib/analog_pin.h>
#include "common.h"

// Enable the Parasite power (check ds18x20 Datasheet )
#define PARASITE_POWER  false

// where the sensors are hanging .... "on"
//                  IO11     IO12     IO13    IO14
int sensorPins[] = {PIO1_10, PIO0_11, PIO1_0, PIO1_2};
int adChannels[] = {AD6,     AD0,     AD1,    AD3};
// IO2  PIO2_2
// IO3  PIO0_9
// IO4  PIO2_11
// IO5  PIO1_1  AD2
// IO6  PIO3_0
// IO7  PIO3_1
// IO8  PIO3_2
// IO10 PIO0_8
// IO11 PIO1_10 AD6
// IO12 PIO0_11 AD0
// IO13 PIO1_0  AD1
// IO14 PIO1_2  AD3
// IO15 PIO2_3
// IO16 PIO1_5
// IO17 PIO0_5
// IO18 PIO0_4
//
// 1V 323
// 2v 646
// 3v 968-970
// a usable alternative over the stupid 130ms
#define TIME_BASE_MS 125u

// specifies what schedule is set to when deltas or thresholds are set.
#define DEFAULT_SAMPLE_PERIOD 1000

// easy to read on the bus
#define INVALID_DATA (-999)

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
bool SensorConfig::init(int sensorIdx, uint sensorType, int dpt, int8_t mult, COM voltCo) {
    sensorNum = sensorIdx + 1;
    if (sensorType != SENSOR_TYP_TEMPERATURE_SENSOR) {
        LOG("Ignoring Sensor %d because it is not set to temperature type but %d",
            sensorNum, sensorType);
        return false;
    }
    LOG("Initializing Sensor %d", sensorNum);
    type = sensorType;
    port = adChannels[sensorIdx];
    pinMode(sensorPins[sensorIdx], INPUT_ANALOG);

    res = dpt;
    if (dpt == SENDETYP_MW_DPT5_EIS6_NON_STANDARD_COMPLIANT) {
        LOG("#%d using non-standard DPT5 temp resolution", sensorNum);
    }
    multiplier = mult;
    // a good invalid start value ;)
    voltage = lastTrig = INVALID_DATA;
    LOG("#%d using a voltage multiplier for a %dV target", sensorNum, mult/2);
    com = voltCo;
    fixRamLoc(voltCo);
    return true;
}

void SensorConfig::setSendPeriod(int sendValAtStart, uint startFactor,
        int sendTempThen, uint tempTimeFactor, uint tempTimeBase) {

    int ttb = TIME_BASE_MS << tempTimeBase;
    uint now = millis();
    if (sendTempThen == JA_NEIN_YES) {
        sendFreq = ttb * tempTimeFactor;
        LOG("#%d will send voltage updates every %dms", sensorNum, sendFreq);
        // schedule the first send
        sendTime = now - (now % sendFreq) + sendFreq;
    }

    // do this one second so sendXXXTime overrides the period one
    if(sendValAtStart == INIT_MW_SENT_WITH_DELAY) {
        uint32_t sendStart = ttb * startFactor;
        LOG("#%d will send voltage %dms after start", sensorNum, sendStart);
        sendTime = now - (now % sendStart) + sendStart;
    }
}

void SensorConfig::setDiffTrigger(bool checkTemp, uint tempTrigger) {

    if (checkTemp == JA_NEIN_YES) {
        trig = tempTrigger * .1;
        schedule = DEFAULT_SAMPLE_PERIOD;
        LOG("#%d will send if voltage varies by more than %dmV", sensorNum, tempTrigger);
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

void SensorConfig::setThreshold(uint num, uint triggerType,
        uint triggerValue, uint triggerAction, COM comObject) {
    schedule = DEFAULT_SAMPLE_PERIOD;
    th[num].init(num, sensorNum, (GRENZWERT_ZUORDNUNG)triggerType,
            triggerValue, triggerAction, comObject);
}

// Runtime functions
void SensorConfig::readValues() {
    if (!isActive()) return;
    uint32_t now = millis();

    if (sampleTime > now) {
        return;
    }
    sampleTime = now + 200;

#ifdef LOGGING
    int oldAdc = adc;
#endif
    adc = analogValidRead(port);
    if (adc == -1 || adc == 1023) {
        LOG("#%d ignoring invalid adc reading %d", sensorNum, adc);
        return;
    }
    pool.getVariance(adc);
    // Our reference voltage is measured at 900.
    // Multiplier indicates target voltage * 2
    voltage = (float)((double)pool.avg * multiplier * 0.5 / 900 * 1000);
#ifdef LOGGING
    int diff = abs(oldAdc - adc);
    if (diff > 2) {
        LOG("#%d read volt: %dmV avg: %d diff: %d", sensorNum, pretty(voltage), pool.avg, diff);
    }
#endif
    // test for variations
    if (trig != 0) {
        if (lastTrig == INVALID_DATA) {
            lastTrig = voltage;
        } else {
            float thisDiff = abs(voltage - lastTrig);
            if (thisDiff > trig) {
                LOG("#%d volt: %dmV is more than %dmV from last volt: %dmV",
                    sensorNum, pretty(voltage),
                    pretty(trig), pretty(lastTrig));
                lastTrig = voltage;
                // send it on the bus
                sendVoltage();
            }
        }
    }
    if (voltage == INVALID_DATA) return;
    if (thTime && thTime < (now+1000)) {
        // sending this in a second oughta do, too.
        return;
    }
    // test for changes
    for (auto &t : th) {
        t.test(voltage, false);
    }
}

void SensorConfig::doPeriodics() {
    if (!isActive()) return;
    if (voltage == INVALID_DATA) return;
    uint32_t now = millis();
    // check periodics
    if (sendTime && sendTime < now) {
        // send it on the bus
        LOG("#%d sending volt: %dmV", sensorNum, pretty(voltage));
        sendVoltage();
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
            t.test(voltage, true);
        }
        if (thFreq) {
            thTime = now - (now % thFreq) + thFreq;
        } else {
            thTime = 0;
        }
    }
}

void SensorConfig::sendVoltage() const {
    if (res == SENDETYP_MW_DPT9_EIS5_DEFAULT) {
        bcu.comObjects->objectWriteFloat(com, voltage * 100);
    } else {
        bcu.comObjects->objectWrite(com, (uint8_t)voltage);
    }
}

