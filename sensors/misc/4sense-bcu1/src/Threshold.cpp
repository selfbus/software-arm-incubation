//
// Created by Mario Theodoridis on 19.05.20.
//

#include "common.h"

#define Action_above 0b1000u
#define Value_above  0b0100u
#define Action_below 0b0010u
#define Value_below  0b0001u
#define Value_off  0b0000u

// a = above, b = below, v = value
//  avbv
//  3210
const char* Threshold::actionString[] = {
        "deactivated", // DEACTIVATED (0) 0000
        nullptr,
        "below -> 0", // FALLS_BELOW_0 (2) 0010
        "below -> 1", // FALLS_BELOW_1 (3) 0011
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "above -> 0",               // EXCEED_0 (8) 1000
        nullptr,
        "below -> 0 / above -> 0", // FALLS_BELOW_0_EXCEED_0 (10) 1010
        "below -> 1 / above -> 0", // FALLS_BELOW_1_EXCEED_0 (11) 1011
        "above -> 1",              // EXCEED_1 (12)               1100
        nullptr,
        "below -> 0 / above -> 1", // FALLS_BELOW_0_EXCEED_1 (14) 1110
        "below -> 1 / above -> 1", // FALLS_BELOW_1_EXCEED_1 (15) 1111
};

void
Threshold::init(int idx, int sensNum, GRENZWERT_ZUORDNUNG triggerType,
        uint triggerValue, uint triggerAction, COM comObject) {
    sensorNum = sensNum;
    thresholdNum = idx + 1;
    type = triggerType;
    action = (uint8_t)triggerAction;
    trigger = triggerValue * .1f;
    comObj = comObject;
    fixRamLoc(comObject);
    currentValue = Value_off;
    LOG("#%d threshold:%d for %s at %d action is %s",
        sensorNum, thresholdNum,
        type == GRENZWERT_ZUORDNUNG_HUMIDITY? "humidity" : "temperature",
        pretty(trigger), actionString[action]);
}

void Threshold::test(float tempValue, float humValue, bool periodic) {
    if (action == GRENZWERT_REAKTION_DEACTIVATED) return;
    float value = type == GRENZWERT_ZUORDNUNG_HUMIDITY? humValue : tempValue;
    if (value > trigger && action & Action_above) {
        byte v = action & Value_above? 1 : 0;
        if (currentValue != Value_above || periodic) {
            // only send changes unless we updating periodially
            LOG("#%d value %d is above threshold %d, so sending %d",
                sensorNum, pretty(value), thresholdNum, v);
            bcu.comObjects->objectWrite(comObj, &v);
        }
        currentValue = Value_above;
        return;
    }
    if (value < trigger && action & Action_below) {
        byte v = action & Value_below? 1 : 0;
        if (currentValue != Value_below || periodic) {
            // only send changes unless we updating periodially
            LOG("#%d value %d is below threshold %d, so sending %d",
                sensorNum, pretty(value), thresholdNum, v);
            bcu.comObjects->objectWrite(comObj, &v);
        }
        currentValue = Value_below;
        return;
    }
    currentValue = Value_off;
}
