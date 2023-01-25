//
// Created by Mario Theodoridis on 17.05.20.
//

#include <sblib/io_pin_names.h>
#include "common.h"

// Enable the Parasite power (check ds18x20 Datasheet )
#define PARASITE_POWER  false

// where the sensors are hanging .... "on" a TS_ARM
#ifdef TS_ARM
//                      IO16    IO14    IO12     IO10
    int sensorPins[] = {PIO1_5, PIO1_2, PIO0_11, PIO0_8};
#else //default 4TE-Controller
    //                  PIO2_2   PIO0_7   PIO2_10  PIO2_9
    int sensorPins[] = {PIN_IO1, PIN_IO2, PIN_IO3, PIN_IO4};
#endif

// a usable alternative over the stupid 130ms
#define TIME_BASE_MS 125u

// specifies what schedule is set to when deltas or thresholds are set.
#define DEFAULT_SAMPLE_PERIOD 1000

#define INIT_DELAY 2200 //!< default conversion start delay after a initialization

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

bool SensorConfig::isInitialized() const
{
    return (initialized);
}

void SensorConfig::setInitialized(bool newState)
{
    LOG("#%d initialized set to %d", sensorNum, newState);
    initialized = newState;
}

// Initializers
bool SensorConfig::init(int sensorIdx, SENSOR_TYP sensorType, SENSOR_FEUCHTIGKEIT senserDHTType,
        int tempDpt, int8_t tempOffset, COM tempCo,
        int humDpt, int8_t humOffset, COM humCo) {
    sensorNum = sensorIdx + 1;
    switch (sensorType)
    {
    case SENSOR_TYP_TEMPERATURE_SENSOR: // DS18..
        type = SN_DS18XX;
        isDht = false;
        ds.DS18x20Init(sensorPins[sensorIdx], PARASITE_POWER);
        LOG("#%d ETS configured as DS18xx Sensor", sensorNum);
        break;
    case SENSOR_TYP_TEMPERATURE_HUMIDITY_SENSOR: // DHT..
        isDht = true;
        eType dhtType;
        switch (senserDHTType)
        {
            case SENSOR_FEUCHTIGKEIT_DTH11:
                dhtType = DHT11;
                type = SN_DHT11;
                LOG("#%d ETS configured as DHT11 Sensor", sensorNum);
                break;
            default:
                type = SN_DHT22;
                dhtType = DHT22;
                LOG("#%d ETS configured as DHT22 Sensor", sensorNum);
        }
        dht.DHTInit(sensorPins[sensorIdx], dhtType);
        break;
    default:
        type = SN_NO_SENSOR;
        LOG("#%d ETS not configured", sensorNum);
        return (false);
    }

    // Scan the 1-Wire bus for DS18x devices
    bool success = false;
    uint8_t searchResult;
    for(int8_t tries = 1; tries <= 3; tries++) {
        switch (type) {
            case SN_DS18XX :
                searchResult = ds.Search();
                if (searchResult == 0) { // success
                    isDht = false;
                    leadTime = DS18x20::leadTime;
                    success = true;
                }
                break;
            case SN_DHT11 :
            case SN_DHT22 :
                searchResult = dht.readData(true);
                if (searchResult != 0) { // success
                    isDht = true;
                    leadTime = DHT::leadTime;
                    success = true;
                }
                break;
            default :
                return (false);
        }
        if (success) {
            break;
        }
        LOG("#%d Try %d no devices found. Ec: %d", sensorNum, tries, success);
        delay(50); // maybe bus settling?
    }

    if (!success)
    {
        return (false);
    }

    setInitialized(true);
    converting = false;
    // convTime = millis() + INIT_DELAY;
    // readTime = millis() + leadTime;

    switch (type)
    {
    case SN_DHT11 :
        if (success) {
            LOG("#%d has a DHT11 Sensor", sensorNum);
        }
        break;
    case SN_DHT22 :
        if (success) {
            LOG("#%d has a DHT22 Sensor", sensorNum);
        }
        break;
    default:
        if (sensorType == SENSOR_TYP_TEMPERATURE_HUMIDITY_SENSOR) {
            LOG("#%d cannot do humidity with a DS18 sensor", sensorNum);
            setInitialized(false);
            return (false);
        }
        if (!ds.m_foundDevices) {
            setInitialized(false);
            return (false);
        }
        for (int i = 0; i < ds.m_foundDevices; i++) {
            LOG("#%d found a %s device", sensorNum, ds.TypeStr(i));
        }
    }

    tRes = tempDpt;
    if (tempDpt == SENDETYP_MW_DPT5_EIS6_NON_STANDARD_COMPLIANT) {
        LOG("#%d using non-standard DPT5 temp resolution", sensorNum);
    }
    tOffset = tempOffset * .1f;
    // a good invalid start value ;)
    temperature = lastTempTrig = INVALID_DATA;
    LOG("#%d using a temperature offset of %dd°", sensorNum, tempOffset);
    tCom = tempCo;
    fixRamLoc(tempCo);

    if (sensorType == SENSOR_TYP_TEMPERATURE_HUMIDITY_SENSOR) {
        doesHumidity = true;
        hRes = humDpt;
        if (humDpt == SENDETYP_MW_DPT5_EIS6_NON_STANDARD_COMPLIANT) {
            LOG("#%d using non-standard DPT5 humidity resolution", sensorNum);
        }
        hOffset = humOffset * .1f;
        LOG("#%d using a humidity offset of %dpm", sensorNum, humOffset);
        humidity = INVALID_DATA;
        hCom = humCo;
        fixRamLoc(humCo);
    }
    return (true);
}

void SensorConfig::setSendPeriod(int sendValAtStart, uint startFactor,
        int sendTempThen, uint tempTimeFactor, uint tempTimeBase,
        int sendHumThen, uint humTimeFactor, uint humTimeBase) {

    int ttb = TIME_BASE_MS << tempTimeBase;
    int htb = TIME_BASE_MS << humTimeBase;
    uint now = millis();
    if (sendTempThen == JA_NEIN_YES) {
        sendTempFreq = ttb * tempTimeFactor;
        LOG("#%d will send temperature updates every %dms", sensorNum, sendTempFreq);
        // schedule the first send
        sendTempTime = now - (now % sendTempFreq) + sendTempFreq;
    }
    if (sendHumThen == JA_NEIN_YES) {
        sendHumFreq = htb * humTimeFactor;
        LOG("#%d will send humidity updates every %dms", sensorNum, sendHumFreq);
        // schedule the first send
        sendHumTime = now - (now % sendHumFreq) + sendHumFreq;
    }

    // do this one second so sendXXXTime overrides the period one
    if(sendValAtStart == INIT_MW_SENT_WITH_DELAY) {
        uint32_t sendStart = ttb * startFactor;
        LOG("#%d will send temperature %dms after start", sensorNum, sendStart);
        sendTempTime = now - (now % sendStart) + sendStart;
        if (sendHumThen == JA_NEIN_YES) {
            sendStart = htb * startFactor;
            LOG("#%d will send humidity %dms after start", sensorNum, sendStart);
            sendHumTime = now - (now % sendStart) + sendStart;
        }
    }
}

void SensorConfig::setDiffTrigger(bool checkTemp, uint tempTrigger,
                                  bool checkHum, uint humTrigger) {

    if (checkTemp == JA_NEIN_YES) {
        tempTrig = tempTrigger * .1f;
        schedule = DEFAULT_SAMPLE_PERIOD;
        LOG("#%d will send if temperature varies by more than %dd°", sensorNum, tempTrigger);
    }
    if (checkHum == JA_NEIN_YES) {
        humTrig = humTrigger * .1f;
        schedule = DEFAULT_SAMPLE_PERIOD;
        LOG("#%d will send if humidity varies by more than %dpm", sensorNum, humTrigger);
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
void SensorConfig::setConvTime(uint when) {
    if (when) {
        uint now = millis() + leadTime;
        convTime = now - (now % when) + when - leadTime-100; // more slack
    } else {
        convTime = 0;
    }
}

void SensorConfig::sampleValues() {
    if (!isActive()) return;
    uint32_t now = millis();
    if (!converting && convTime < now) {
        if (!isDht) ds.startConversion(0);
        readTime = now + leadTime;
        if (schedule) {
            setConvTime(schedule);
        } else {
            // only when sending periodically without thresholds or triggers
            setConvTime(sendTempFreq);
        }
        converting = true;
//        LOG("#%d read: %d conv: %d send: %d", sensorNum, readTime, convTime, sendTempTime);
    }
}

void SensorConfig::readValues() {
    if (!isActive()) return;
    uint32_t now = millis();
    if (!converting || (readTime >= now)) return;
    bool success = false;
    if (isDht) {
        success = dht.readData();
    } else {
        success = ds.readResult(0);
    }
    if (!success) {
        if (readTime && now > (readTime + 3000)) {
            LOG("#%d Failed to get reading at %d", sensorNum, readTime);
            setInitialized(false);
        }
        return;
    }

    // Check if the last temperature read was OK
    converting = false;

    if (!isDht && !ds.lastReadOk(0)) {
        LOG("#%d Ignoring invalid reading", sensorNum);
        setInitialized(false);
        return;
    }
#ifdef LOGGING
    float oldTemp = temperature, oldHum = humidity;
#endif
    float temp;
    if (isDht) {
        temp = dht._lastTemperature + tOffset;
        if (doesHumidity) {
            humidity = dht._lastHumidity + hOffset;
        }
    } else {
        temp = ds.temperature(0) + tOffset;
    }
    float var = pool.getVariance(temp);
    if (var < MAX_TEMP_VARIANCE) {
        temperature = temp;
    } else {
        LOG("#%d ignoring outlier temperature %dd° due to variance of %dd°",
            sensorNum, pretty(temp), pretty(var));
    }
#ifdef LOGGING
    if (oldTemp != temperature) {
        LOG("#%d read temp: %dd°", sensorNum, pretty(temperature));
    } else {
        // LOG("#%d read temp: %dd unchanged°", sensorNum, pretty(temperature));
    }

    if (doesHumidity && oldHum != humidity) {
        LOG("#%d read humidity: %dpm", sensorNum, pretty(humidity));
    } else {
        // LOG("#%d read humidity: %dpm unchanged", sensorNum, pretty(humidity));
    }
#endif
    // test for variations
    if (tempTrig != 0) {
        if (lastTempTrig == INVALID_DATA) {
            lastTempTrig = temperature;
        } else {
            float thisDiff = abs(temperature - lastTempTrig);
            if (thisDiff > tempTrig) {
                LOG("#%d temp: %dd° is more than %dd° from last temp: %dd°",
                    sensorNum, pretty(temperature),
                    pretty(tempTrig), pretty(lastTempTrig));
                lastTempTrig = temperature;
                // send it on the bus
                sendTemperature();
            }
        }
    }
    if (doesHumidity && humTrig != 0) {
        if (lastHumTrig == INVALID_DATA) {
            lastHumTrig = humidity;
        } else {
            float thisDiff = abs(humidity - lastHumTrig);
            if (thisDiff > humTrig) {
                LOG("#%d humidity: %dpm is more than %dpm from last humidity: %dpm",
                    sensorNum, pretty(humidity),
                    pretty(humTrig), pretty(lastHumTrig));
                lastHumTrig = humidity;
                // send it on the bus
                sendHumidity();
            }
        }
    }
    if (temperature == INVALID_DATA) return;
    if (doesHumidity && humidity == INVALID_DATA) return;
    if (thTime && thTime < (now+1000)) {
        // sending this in a second oughta do, too.
        return;
    }
    // test for changes
    for (auto &t : th) {
        t.test(temperature, humidity, false);
    }
}

void SensorConfig::doPeriodics() {
    if (!isActive()) return;
    if (temperature == INVALID_DATA) return;
    uint32_t now = millis();
    // check periodics
    if (sendTempTime && sendTempTime < now) {
        // send it on the bus
        LOG("#%d sending temp: %dd°", sensorNum, pretty(temperature));
        sendTemperature();
        if (sendTempFreq) {
            sendTempTime = now - (now % sendTempFreq) + sendTempFreq;
        } else {
            sendTempTime = 0;
        }
    }
    if (sendHumTime && sendHumTime < now) {
        // send it on the bus
        LOG("#%d sending humidity: %dpm", sensorNum, pretty(humidity));
        sendHumidity();
        if (sendHumFreq) {
            sendHumTime = now - (now % sendHumFreq) + sendHumFreq;
        } else {
            sendHumTime = 0;
        }
    }
    // deal with the thresholds
    if (thTime && thTime < now) {
        // send no matter what
        for (auto &t : th) {
            t.test(temperature, humidity, true);
        }
        if (thFreq) {
            thTime = now - (now % thFreq) + thFreq;
        } else {
            thTime = 0;
        }
    }
}

void SensorConfig::sendTemperature() const {
    if (tRes == SENDETYP_MW_DPT9_EIS5_DEFAULT) {
        bcu.comObjects->objectWriteFloat(tCom, temperature*100);
    } else {
        bcu.comObjects->objectWrite(tCom, (uint8_t)temperature);
    }
}

void SensorConfig::sendHumidity() const {
    if (hRes == SENDETYP_MW_DPT9_EIS5_DEFAULT) {
        bcu.comObjects->objectWriteFloat(hCom, humidity*100);
    } else {
        bcu.comObjects->objectWrite(hCom, (uint8_t)humidity);
    }
}

