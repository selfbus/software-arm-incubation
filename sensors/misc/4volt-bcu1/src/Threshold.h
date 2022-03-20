//
// Created by Mario Theodoridis on 19.05.20.
//

#ifndef INC_4SENSE_THRESHOLD_H
#define INC_4SENSE_THRESHOLD_H


class Threshold {
private:
    // string representations of the actions
    static const char* actionString[];
    int sensorNum;      // The sensor this threshold belongs to
    int thresholdNum;   // The threshold number i am
    GRENZWERT_ZUORDNUNG type; // whether i'm voltage or humididty
    float trigger;      // value at which this threshold triggers
    uint8_t action;     // which GRENZWERT_REAKTION
    // whether were currently above, below or within our threshold
    uint8_t currentValue;
    int comObj;         // which com object to send with

public:
    void init(int idx, int sensNum, GRENZWERT_ZUORDNUNG triggerType,
            uint triggerValue, uint triggerAction, COM comObject);
    void test(float tempValue, bool periodic);
};


#endif //INC_4SENSE_THRESHOLD_H
