//
// Created by mario on 17.05.20.
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
    int resolution;     // whether DPT9 float16 default or DTP5 uint8
    bool converting;    // indicates a conversion to be sent is in progress
    DS18x20 ds;         // the sensors for this config

    // temperature related
    Filter pool;        // averaging to filter out spikes
    int16_t offset;     // for data correction (bus version)
    // current offset adjusted temperature sample
    int16_t temperature;    // bus version
    int16_t diffTrigger; // at what difference to send if not 0 (raw)
    int16_t lastTrigger;  // temperature at which last diffTrigger was fired

    // set to a fixed sampling schedule when deltas or thresholds are set
    // or 0 in which case the sampling depends on sendTime
    uint32_t schedule;
    uint32_t convTime;  // when it's time to start a conversion
    uint32_t readTime;  // when it's time to read the data

    // for periodic temperature updates
    uint32_t sendStart; // sending delay after reboot
    uint32_t sendFreq;  // sending frequency when sending periodically
    uint32_t sendTime;  // when it's time to send data
    int comObj;         // which com object to send with

    // for periodic threshold updates
    uint32_t thStart;   // sending delay after reboot
    uint32_t thFreq;    // sending frequency for thresholds
    uint32_t thTime;    // when it's time to send threshold data
    Threshold th[3];    // thresholds

public:
    // Initializers
    bool init(int sensorIdx, uint sensorType, int dpt,
              int8_t tempOffset, COM comObject);
    void setDiffTrigger(uint trigger);
    void setSendPeriod(int sendAtStart, int sendThen, uint startFactor,
            uint timeFactor, uint timeBase);
    void setThresholdPeriod(int sendAtStart, int sendThen, uint startFactor,
            uint timeFactor, uint timeBase);
    void setThreshold(uint num, uint triggerValue, uint triggerAction, COM comObject);

    // Runtime functions
    bool isActive() const { return type != SENSOR_TYP_NO_SENSOR; }
    void setConvTime(uint when);
    void sampleValues();
    void readValues();
    void doPeriodics();
    void sendTemperature() const;
};

typedef SensorConfig* SensorConf;


#endif //INC_4SENSE_SENSORCONFIG_H
