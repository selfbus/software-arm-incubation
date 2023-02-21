//
// Created by Mario Theodoridis on 22.05.21.
//
#include "common.h"

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
// IO17 PIO0_5 SDA
// IO18 PIO0_4 SCL
// where the sensors are hanging .... "on"
//                  IO11     IO12     IO13    IO14
int sensorPins[] = {PIO1_10, PIO0_11, PIO1_0, PIO1_2};

// a usable alternative over the stupid 130ms
#define TIME_BASE_MS 125u

// specifies what schedule is set to when deltas or thresholds are set.
#define DEFAULT_SAMPLE_PERIOD 1000

// easy to read on the bus
#define INVALID_DATA (-1)

float Avg::getAvg(float num) {
    sum += num;
    cnt++;
    return sum/(float)cnt;
}

void Avg::reset(){
    sum = 0;
    cnt = 0;
}

// Initializers
bool SensorConfig::init(int sensorIdx, uint sensorType, int dpt, int8_t off, COM luxCo) {
    sensorNum = sensorIdx + 1;
    if (sensorType != SENSOR_TYP_TEMPERATURE_SENSOR) {
        LOG("Ignoring Sensor %d because it is not set to temperature type but %d",
            sensorNum, sensorType);
        return false;
    }
    if (dpt != SENDETYP_MW_DPT9_EIS5_DEFAULT) {
        LOG("Ignoring Sensor %d because resolution is not DPT9/EIS5 (%d) but %d",
            sensorNum, SENDETYP_MW_DPT9_EIS5_DEFAULT, dpt);
        return false;
    }
    LOG("Initializing Sensor %d", sensorNum);
    BH1750::Mode mode = BH1750::CONTINUOUS_HIGH_RES_MODE_2;
    if (!bh.begin(mode)) {
        LOG("Failed initializing Sensor %d", sensorNum);
        return false;
    }
    type = sensorType;
    // this is currently set inside of begin so no need to initialize it here
    mTime = BH1750_DEFAULT_MTREG;
    // a good invalid start value ;)
    lux = lastTrig = INVALID_DATA;
    ag.reset();
    com = luxCo;
    fixRamLoc(luxCo);
    return true;
}

void SensorConfig::setSendPeriod(int sendValAtStart, uint startFactor,
        int sendTempThen, uint tempTimeFactor, uint tempTimeBase) {

    int ttb = TIME_BASE_MS << tempTimeBase;
    uint now = millis();
    if (sendTempThen == JA_NEIN_YES) {
        sendFreq = ttb * tempTimeFactor;
        LOG("#%d will send lux updates every %dms", sensorNum, sendFreq);
        // schedule the first send
        sendTime = now - (now % sendFreq) + sendFreq;
    }

    // do this one second so sendXXXTime overrides the period one
    if(sendValAtStart == INIT_MW_SENT_WITH_DELAY) {
        uint32_t sendStart = ttb * startFactor;
        LOG("#%d will send lux %dms after start", sensorNum, sendStart);
        sendTime = now - (now % sendStart) + sendStart;
    }
}

void SensorConfig::setDiffTrigger(bool checkTemp, uint tempTrigger) {

    if (checkTemp == JA_NEIN_YES) {
        trig = tempTrigger * .1;
        schedule = DEFAULT_SAMPLE_PERIOD;
        LOG("#%d will send if lux varies by more than %dlx", sensorNum, tempTrigger);
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

float SensorConfig::readLight() {
    float light = bh.readLightLevel();
    if (light < 0) {
        LOG("Error condition detected");
        return light;
    }

    if (light > 40000.0) {
        // reduce measurement time - needed in direct sun light
        if (mTime == 32) {
            //LOG("Returning high light value");
            return light;
        }
        if (bh.setMTreg(32)) {
            LOG("Adjusted MTReg low for high light");
            mTime = 32;
            return adjustMTimeWait;
        }
        LOG("Failed adjusting MTReg low for high light");
        return adjustMTimeFail;
    }

    if (light > 10.0) {
        // typical light environment
        if (mTime == 69) {
            //LOG("Returning normal light value");
            return light;
        }
        if (bh.setMTreg(69)) {
            LOG("Adjusted MTReg normal for normal light");
            mTime = 69;
            return adjustMTimeWait;
        }
        LOG("Failed adjusting MTReg normal for normal light");
        return adjustMTimeFail;
    }

    // very low light environment (light <= 10.0)
    if (mTime == 138) {
        //LOG("Returning low light value");
        return light;
    }
    if (bh.setMTreg(138)) {
        LOG("Adjusted MTReg high for low light");
        mTime = 138;
        return adjustMTimeWait;
    }
    LOG("Failed adjusting MTReg high for low light");
    return adjustMTimeFail;
}

// Runtime functions
void SensorConfig::readValues() {
#ifdef LOGGING
    static float oldLight = lux;
#endif
    if (!isActive()) return;
    uint32_t now = millis();

    if (sampleTime > now || !bh.measurementReady(true)) {
        return;
    }

    float light = readLight();
    if (light == adjustMTimeWait) {
        LOG("#%d switching measurement time", sensorNum);
        return;
    }
    if (light < 0) {
        LOG("#%d failed reading", sensorNum);
        return;
    }
    // only set next time if we got a good reading
    sampleTime = now + 1000;

    lux = ag.getAvg(light);
#ifdef LOGGING
    float diff = abs(oldLight - light);
    oldLight = light;
    if (diff > 1) {
        LOG("#%d read lux: %d avg: %d diff: %d", sensorNum, pretty(light), pretty(lux), pretty(diff));
    }
#endif
    // test for variations
    if (trig != 0) {
        if (lastTrig == INVALID_DATA) {
            lastTrig = lux;
        } else {
            float thisDiff = abs(lux - lastTrig);
            if (thisDiff > trig) {
                LOG("#%d lux: %dlx is more than %dlx from last lux: %dlx",
                    sensorNum, pretty(lux),
                    pretty(trig), pretty(lastTrig));
                lastTrig = lux;
                // send it on the bus
                sendReading();
            }
        }
    }
    if (lux == INVALID_DATA) return;
    if (thTime && thTime < (now+1000)) {
        // sending this in a second oughta do, too.
        return;
    }
    // test for changes
    for (auto &t : th) {
        t.test(lux, false);
    }
}

void SensorConfig::doPeriodics() {
    if (!isActive()) return;
    if (lux == INVALID_DATA) return;
    uint32_t now = millis();
    // check periodics
    if (sendTime && sendTime < now) {
        // send it on the bus
        LOG("#%d sending lux: %dlx", sensorNum, pretty(lux));
        sendReading();
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
            t.test(lux, true);
        }
        if (thFreq) {
            thTime = now - (now % thFreq) + thFreq;
        } else {
            thTime = 0;
        }
    }
}

void SensorConfig::sendReading() {
    bcu.comObjects->objectWriteFloat(com, lux * 100);
    ag.reset();
}
