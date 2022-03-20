//
// Created by Mario Theodoridis on 22.05.21.
//

#ifndef INC_4SENSE_SENSORCONFIG_H
#define INC_4SENSE_SENSORCONFIG_H

// variance from the average
#define MAX_TEMP_VARIANCE 5
// how many samples to average
#define POOL_SIZE 10
class Avg {
private:
    uint cnt = 0;
    float sum = 0;
public:
    float getAvg(float num);
    void reset();
};


class SensorConfig {
private:
    int sensorNum;      // The sensor number i am
    int port;           // The sensor pin
    uint type;          // whether this config is actually used

    // lux related
    Avg ag;         // average aggregator
    // current factor adjusted lux sample
    BH1750 bh;      // the associated light sensor
    byte mTime;     // the current measurement time

    float lux;  // ditto
    float trig;     // at what lux difference to send if not 0
    float lastTrig; // lux at which last trigger was fired

    // set to a fixed sampling schedule when deltas or thresholds are set
    // or 0 in which case the sampling depends on sendTime
    uint32_t schedule;

    // for periodic lux updates
    uint32_t sendFreq;      // sending frequency when sending lux periodically
    uint32_t sendTime;      // when it's time to send lux data
    uint32_t sampleTime;    // when it's time to send lux data
    int com;                // which com object to send lux with

    // for periodic threshold updates
    uint32_t thStart;   // sending delay after reboot
    uint32_t thFreq;    // sending frequency for thresholds
    uint32_t thTime;    // when it's time to send threshold data
    Threshold th[3];    // thresholds
    float readLight();
    // light level changed
    const float adjustMTimeWait = -23;
    // failed setting MTreg
    const float adjustMTimeFail = -42;

public:
    // Initializers
    bool init(int sensorIdx, uint sensorType, int dpt, int8_t offset,
              COM luxCo);
    void setSendPeriod(int sendValAtStart, uint startFactor,
                       int sendTempThen, uint tempTimeFactor, uint tempTimeBase);
    void setDiffTrigger(bool checkTemp, uint tempTrigger);
    void setThresholdPeriod(int sendAtStart, int sendThen, uint startFactor,
            uint timeFactor, uint timeBase);
    void setThreshold(uint num, uint triggerType,
            uint triggerValue, uint triggerAction, COM comObject);

    // Runtime functions
    bool isActive() const { return type != SENSOR_TYP_NO_SENSOR; }
    void readValues();
    void doPeriodics();
    void sendReading();
};

typedef SensorConfig* SensorConf;


#endif //INC_4SENSE_SENSORCONFIG_H
