//
// Created by Mario Theodoridis on 17.05.20.
//

#ifndef INC_4SENSE_SENSORCONFIG_H
#define INC_4SENSE_SENSORCONFIG_H

// 5° variance from the average
#define MAX_TEMP_VARIANCE 50
// how many samples to average
#define POOL_SIZE 10
class Filter {
private:
    int pool[POOL_SIZE];
    int cnt = 0;
    int sum = 0;
    int avg = 0;
public:
    uint getVariance(int num);
};


class SensorConfig {
private:
    int sensorNum;      // The sensor number i am
    int type;           // whether this config is actually used
    int tRes;           // whether temperature is DPT9 float16 default or DTP5 uint8
    int hRes;           // whether humidity is DPT9 float16 default or DTP5 uint8
    bool converting;    // indicates a conversion to be sent is in progress
    DS18x20 ds;         // the DS18 sensors for this config
    DHT dht;            // rge DHT Sensor for this config
    bool isDht;         // whether it's a DHT or DS
    bool doesHumidity;  // whether this sensor tracks humidity
    uint32_t leadTime;  // time it takes for a measurement

    // temperature related
    Filter pool;        // averaging to filter out spikes
    float tOffset;      // for temperature data correction
    float hOffset;      // for humidity data correction
    // current offset adjusted temperature sample
    float temperature;  // ditto
    float humidity;     // ditto
    float tempTrig;     // at what temperature difference to send if not 0
    float lastTempTrig; // temperature at which last trigger was fired
    float humTrig;   // at what humidity difference to send if not 0
    float lastHumTrig;  // humidity at which last trigger was fired

    // set to a fixed sampling schedule when deltas or thresholds are set
    // or 0 in which case the sampling depends on sendTempTime
    uint32_t schedule;
    uint32_t convTime;  // when it's time to start a conversion
    uint32_t readTime;  // when it's time to read the data

    // for periodic temperature updates
    uint32_t sendTempFreq;  // sending frequency when sending temp periodically
    uint32_t sendTempTime;  // when it's time to send temp data
    uint32_t sendHumFreq;   // sending frequency when sending humidity periodically
    uint32_t sendHumTime;   // when it's time to send humidity data
    int tCom;               // which com object to send temperatures with
    int hCom;               // which com object to send humidity with

    // for periodic threshold updates
    uint32_t thStart;   // sending delay after reboot
    uint32_t thFreq;    // sending frequency for thresholds
    uint32_t thTime;    // when it's time to send threshold data
    Threshold th[3];    // thresholds

public:
    // Initializers
    bool init(int sensorIdx, uint sensorType,
            int tempDpt, int8_t tempOffset, COM tempCo,
            int humDpt, int8_t humOffset, COM humCo);
    void setSendPeriod(int sendValAtStart, uint startFactor,
                       int sendTempThen, uint tempTimeFactor, uint tempTimeBase,
                       int sendHumThen, uint humTimeFactor, uint humTimeBase);
    void setDiffTrigger(bool checkTemp, uint tempTrigger,
                        bool checkHum, uint humTrigger);
    void setThresholdPeriod(int sendAtStart, int sendThen, uint startFactor,
            uint timeFactor, uint timeBase);
    void setThreshold(uint num, uint triggerType,
            uint triggerValue, uint triggerAction, COM comObject);

    // Runtime functions
    bool isActive() const { return type != SENSOR_TYP_NO_SENSOR; }
    void setConvTime(uint when);
    void sampleValues();
    void readValues();
    void doPeriodics();
    void sendTemperature() const;
    void sendHumidity() const;
};

typedef SensorConfig* SensorConf;


#endif //INC_4SENSE_SENSORCONFIG_H
