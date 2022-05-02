//
// Created by Mario Theodoridis on 17.05.20.
//

#ifndef INC_4VOLT_SENSORCONFIG_H
#define INC_4VOLT_SENSORCONFIG_H

// variance from the average
#define MAX_TEMP_VARIANCE 5
// how many samples to average
#define POOL_SIZE 10
class Filter {
private:
    int pool[POOL_SIZE];
    int cnt = 0;
    int sum = 0;
public:
    int avg = 0;
    uint getVariance(int num);
};


class SensorConfig {
private:
    int sensorNum;      // The sensor number i am
    int port;           // The sensor pin
    uint type;           // whether this config is actually used
    int res;            // whether voltage is DPT9 float16 default or DTP5 uint8

    // voltage related
    Filter pool;        // averaging to filter out spikes
    int multiplier;       // for voltage data correction
    // current multiplier adjusted voltage sample
    int adc;
    float voltage;  // ditto
    float trig;     // at what voltage difference to send if not 0
    float lastTrig; // voltage at which last trigger was fired

    // set to a fixed sampling schedule when deltas or thresholds are set
    // or 0 in which case the sampling depends on sendTime
    uint32_t schedule;

    // for periodic voltage updates
    uint32_t sendFreq;  // sending frequency when sending voltage periodically
    uint32_t sendTime;  // when it's time to send voltage data
    uint32_t sampleTime;  // when it's time to send voltage data
    int com;            // which com object to send voltage with

    // for periodic threshold updates
    uint32_t thStart;   // sending delay after reboot
    uint32_t thFreq;    // sending frequency for thresholds
    uint32_t thTime;    // when it's time to send threshold data
    Threshold th[3];    // thresholds

public:
    // Initializers
    bool init(int sensorIdx, uint sensorType, int dpt, int8_t multiplier,
              COM voltCo);
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
    void sendVoltage() const;
};

typedef SensorConfig* SensorConf;


#endif //INC_4VOLT_SENSORCONFIG_H
