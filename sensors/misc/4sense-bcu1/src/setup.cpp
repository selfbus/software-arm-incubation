//
// Created by Mario Theodoridis on 17.05.20.
//
#include <cstring>
#include "common.h"

SensorConfig configs[NUM_SENSORS];
bool needSensorInit = false;
uint32_t nextInit = 0;

/**
 * Works around the fact that the ram location for objects isn't set in the
 * com table, so they all overwrite each other into the same RM location.
 * @param comNo each com object gets 2 bytes
 */
void fixRamLoc(COM comNo) {
    uint8_t* l = (uint8_t*)&objectConfig(comNo).dataPtr;
    *l = comNo * 2;
}

void initSensor1(SensorConf sc) {
    if (EE_TYPE_SENSOR_1 == SENSOR_TYP_NO_SENSOR) {
        LOG("Sensor 1 is not used for knx");
        return;
    }

    if (!sc->init(0, EE_TYPE_SENSOR_1,
            EE_SEND_TEMPERATURE_AS_SENSOR_1,
            (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_1,
            COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_1,
            EE_SEND_HUMIDITY_AS_SENSOR_1,
            EE_HUMIDITY_OFFSET_IN_0_1_HUMIDITY_SENSOR_1,
            COM_HUMIDITY_SEND_HUMIDITY_AS_SENSOR_1)) {
        return;
    }

    sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_1,
                      EE_TIME_FACTOR_MEAS_VALUE_SENSOR_1,
                      EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                      EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                      EE_TIMEBASE_TEMPERATURE_SENSOR_1,
                      EE_SEND_PERIODICALLY_HUMIDITY_SENSOR_1,
                      EE_FACTOR_SEND_PERIODICALLY_HUMIDITY_SENSOR_1,
                      EE_TIMEBASE_HUMIDITY_SENSOR_1);

    sc->setDiffTrigger(EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_1,
            EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_1,
            EE_SEND_IF_SIGNAL_CHANGES_HUMIDITY_SENSOR_1,
            EE_SEND_IF_SIGNAL_CHANGES_BY_0_1_RH_HUMIDITY_SENSOR_1);

    if (EE_SEND_IF_THRESHOLD_1_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThresholdPeriod(EE_THRESHOLD_TEMPERATURE_SENSOR_1,
            EE_SEND_PERIODICALLY_THRESHOLD_1_SENSOR_1,
           EE_BUS_RECOVERY_THRESHOLD_TEMP_TIMEFACTOR,
           EE_SEND_PERIODICALLY_FACTOR_THRESHOLD_1_SENSOR_1,
            EE_SEND_PERIODICALLY_TIMEBASE_THRESHOLD_1_SENSOR_1);

    sc->setThreshold(0, EE_ASSOCIATION_THRESHOLD_1_SENSOR_1,
            EE_VALUE_IN_0_1K_THRESHOLD_1_SENSOR_1,
            EE_SEND_IF_THRESHOLD_1_SENSOR_1,
            COM_THRESHOLD_1_TEMPERATURE_THRESHOLD_1_SENSOR_1);

    if (EE_SEND_IF_THRESHOLD_2_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(1, EE_ASSOCIATION_THRESHOLD_2_SENSOR_1,
            EE_VALUE_IN_0_1K_THRESHOLD_2_SENSOR_1,
            EE_SEND_IF_THRESHOLD_2_SENSOR_1,
            COM_THRESHOLD_2_TEMPERATURE_THRESHOLD_2_SENSOR_1);

    if (EE_SEND_IF_THRESHOLD_3_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(2, EE_ASSOCIATION_THRESHOLD_3_SENSOR_1,
            EE_VALUE_IN_0_1K_THRESHOLD_3_SENSOR_1,
            EE_SEND_IF_THRESHOLD_3_SENSOR_1,
            COM_THRESHOLD_3_TEMPERATURE_THRESHOLD_3_SENSOR_1);
}

void initSensor2(SensorConf sc) {
    if (EE_TYPE_SENSOR_2 == SENSOR_TYP_NO_SENSOR) {
        LOG("Sensor 2 is not used for knx");
        return;
    }

    if (!sc->init(1, EE_TYPE_SENSOR_2,
                  EE_SEND_TEMPERATURE_AS_SENSOR_2,
                  (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_2,
                  COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_2,
                  EE_SEND_HUMIDITY_AS_SENSOR_2,
                  EE_HUMIDITY_OFFSET_IN_0_1_HUMIDITY_SENSOR_2,
                  COM_HUMIDITY_SEND_HUMIDITY_AS_SENSOR_2)) {
        return;
    }

    sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_2,
                      EE_TIME_FACTOR_MEAS_VALUE_SENSOR_2,
                      EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_2,
                      EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_2,
                      EE_TIMEBASE_TEMPERATURE_SENSOR_2,
                      EE_SEND_PERIODICALLY_HUMIDITY_SENSOR_2,
                      EE_FACTOR_SEND_PERIODICALLY_HUMIDITY_SENSOR_2,
                      EE_TIMEBASE_HUMIDITY_SENSOR_2);

    sc->setDiffTrigger(EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_2,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_2,
                       EE_SEND_IF_SIGNAL_CHANGES_HUMIDITY_SENSOR_2,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1_RH_HUMIDITY_SENSOR_2);

    if (EE_SEND_IF_THRESHOLD_1_SENSOR_2 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThresholdPeriod(EE_THRESHOLD_TEMPERATURE_SENSOR_2,
                           EE_SEND_PERIODICALLY_THRESHOLD_1_SENSOR_2,
                           EE_BUS_RECOVERY_THRESHOLD_TEMP_TIMEFACTOR,
                           EE_SEND_PERIODICALLY_FACTOR_THRESHOLD_1_SENSOR_2,
                           EE_SEND_PERIODICALLY_TIMEBASE_THRESHOLD_1_SENSOR_2);

    sc->setThreshold(0, EE_ASSOCIATION_THRESHOLD_1_SENSOR_2,
                     EE_VALUE_IN_0_1K_THRESHOLD_1_SENSOR_2,
                     EE_SEND_IF_THRESHOLD_1_SENSOR_2,
                     COM_THRESHOLD_1_TEMPERATURE_THRESHOLD_1_SENSOR_2);

    if (EE_SEND_IF_THRESHOLD_2_SENSOR_2 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(1, EE_ASSOCIATION_THRESHOLD_2_SENSOR_2,
                     EE_VALUE_IN_0_1K_THRESHOLD_2_SENSOR_2,
                     EE_SEND_IF_THRESHOLD_2_SENSOR_2,
                     COM_THRESHOLD_2_TEMPERATURE_THRESHOLD_2_SENSOR_2);

    if (EE_SEND_IF_THRESHOLD_3_SENSOR_2 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(2, EE_ASSOCIATION_THRESHOLD_3_SENSOR_2,
                     EE_VALUE_IN_0_1K_THRESHOLD_3_SENSOR_2,
                     EE_SEND_IF_THRESHOLD_3_SENSOR_2,
                     COM_THRESHOLD_3_TEMPERATURE_THRESHOLD_3_SENSOR_2);
}

void initSensor3(SensorConf sc) {
    if (EE_TYPE_SENSOR_3 == SENSOR_TYP_NO_SENSOR) {
        LOG("Sensor 3 is not used for knx");
        return;
    }

    if (!sc->init(2, EE_TYPE_SENSOR_3,
                  EE_SEND_TEMPERATURE_AS_SENSOR_3,
                  (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_3,
                  COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_3,
                  EE_SEND_HUMIDITY_AS_SENSOR_3,
                  EE_HUMIDITY_OFFSET_IN_0_1_HUMIDITY_SENSOR_3,
                  COM_HUMIDITY_SEND_HUMIDITY_AS_SENSOR_3)) {
        return;
    }

    sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_3,
                      EE_TIME_FACTOR_MEAS_VALUE_SENSOR_3,
                      EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_3,
                      EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_3,
                      EE_TIMEBASE_TEMPERATURE_SENSOR_3,
                      EE_SEND_PERIODICALLY_HUMIDITY_SENSOR_3,
                      EE_FACTOR_SEND_PERIODICALLY_HUMIDITY_SENSOR_3,
                      EE_TIMEBASE_HUMIDITY_SENSOR_3);

    sc->setDiffTrigger(EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_3,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_3,
                       EE_SEND_IF_SIGNAL_CHANGES_HUMIDITY_SENSOR_3,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1_RH_HUMIDITY_SENSOR_3);

    if (EE_SEND_IF_THRESHOLD_1_SENSOR_3 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThresholdPeriod(EE_THRESHOLD_TEMPERATURE_SENSOR_3,
                           EE_SEND_PERIODICALLY_THRESHOLD_1_SENSOR_3,
                           EE_BUS_RECOVERY_THRESHOLD_TEMP_TIMEFACTOR,
                           EE_SEND_PERIODICALLY_FACTOR_THRESHOLD_1_SENSOR_3,
                           EE_SEND_PERIODICALLY_TIMEBASE_THRESHOLD_1_SENSOR_3);

    sc->setThreshold(0, EE_ASSOCIATION_THRESHOLD_1_SENSOR_3,
                     EE_VALUE_IN_0_1K_THRESHOLD_1_SENSOR_3,
                     EE_SEND_IF_THRESHOLD_1_SENSOR_3,
                     COM_THRESHOLD_1_TEMPERATURE_THRESHOLD_1_SENSOR_3);

    if (EE_SEND_IF_THRESHOLD_2_SENSOR_3 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(1, EE_ASSOCIATION_THRESHOLD_2_SENSOR_3,
                     EE_VALUE_IN_0_1K_THRESHOLD_2_SENSOR_3,
                     EE_SEND_IF_THRESHOLD_2_SENSOR_3,
                     COM_THRESHOLD_2_TEMPERATURE_THRESHOLD_2_SENSOR_3);

    if (EE_SEND_IF_THRESHOLD_3_SENSOR_3 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(2, EE_ASSOCIATION_THRESHOLD_3_SENSOR_3,
                     EE_VALUE_IN_0_1K_THRESHOLD_3_SENSOR_3,
                     EE_SEND_IF_THRESHOLD_3_SENSOR_3,
                     COM_THRESHOLD_3_TEMPERATURE_THRESHOLD_3_SENSOR_3);
}

void initSensor4(SensorConf sc) {
    if (EE_TYPE_SENSOR_4 == SENSOR_TYP_NO_SENSOR) {
        LOG("Sensor 4 is not used for knx");
        return;
    }

    if (!sc->init(3, EE_TYPE_SENSOR_4,
                  EE_SEND_TEMPERATURE_AS_SENSOR_4,
                  (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_4,
                  COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_4,
                  EE_SEND_HUMIDITY_AS_SENSOR_4,
                  EE_HUMIDITY_OFFSET_IN_0_1_HUMIDITY_SENSOR_4,
                  COM_HUMIDITY_SEND_HUMIDITY_AS_SENSOR_4)) {
        return;
    }

    sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_4,
                      EE_TIME_FACTOR_MEAS_VALUE_SENSOR_4,
                      EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_4,
                      EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_4,
                      EE_TIMEBASE_TEMPERATURE_SENSOR_4,
                      EE_SEND_PERIODICALLY_HUMIDITY_SENSOR_4,
                      EE_FACTOR_SEND_PERIODICALLY_HUMIDITY_SENSOR_4,
                      EE_TIMEBASE_HUMIDITY_SENSOR_4);

    sc->setDiffTrigger(EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_4,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_4,
                       EE_SEND_IF_SIGNAL_CHANGES_HUMIDITY_SENSOR_4,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1_RH_HUMIDITY_SENSOR_4);

    if (EE_SEND_IF_THRESHOLD_1_SENSOR_4 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThresholdPeriod(EE_THRESHOLD_TEMPERATURE_SENSOR_4,
                           EE_SEND_PERIODICALLY_THRESHOLD_1_SENSOR_4,
                           EE_BUS_RECOVERY_THRESHOLD_TEMP_TIMEFACTOR,
                           EE_SEND_PERIODICALLY_FACTOR_THRESHOLD_1_SENSOR_4,
                           EE_SEND_PERIODICALLY_TIMEBASE_THRESHOLD_1_SENSOR_4);

    sc->setThreshold(0, EE_ASSOCIATION_THRESHOLD_1_SENSOR_4,
                     EE_VALUE_IN_0_1K_THRESHOLD_1_SENSOR_4,
                     EE_SEND_IF_THRESHOLD_1_SENSOR_4,
                     COM_THRESHOLD_1_TEMPERATURE_THRESHOLD_1_SENSOR_4);

    if (EE_SEND_IF_THRESHOLD_2_SENSOR_4 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(1, EE_ASSOCIATION_THRESHOLD_2_SENSOR_4,
                     EE_VALUE_IN_0_1K_THRESHOLD_2_SENSOR_4,
                     EE_SEND_IF_THRESHOLD_2_SENSOR_4,
                     COM_THRESHOLD_2_TEMPERATURE_THRESHOLD_2_SENSOR_4);

    if (EE_SEND_IF_THRESHOLD_3_SENSOR_4 == GRENZWERT_REAKTION_DEACTIVATED) {
        return;
    }
    sc->setThreshold(2, EE_ASSOCIATION_THRESHOLD_3_SENSOR_4,
                     EE_VALUE_IN_0_1K_THRESHOLD_3_SENSOR_4,
                     EE_SEND_IF_THRESHOLD_3_SENSOR_4,
                     COM_THRESHOLD_3_TEMPERATURE_THRESHOLD_3_SENSOR_4);
}

void initSensors() {
    LOG("\n* * * Init Sensors * * *");
    // spiffy those puppies
    memset(&configs[0], 0, sizeof(SensorConfig) * NUM_SENSORS);
    // so if inits run clean we're fine
    needSensorInit = false;
    initSensor1(&configs[0]);
    initSensor2(&configs[1]);
    initSensor3(&configs[2]);
    initSensor4(&configs[3]);
    nextInit = millis() + RE_INIT_DELAY;
}

/*
 * Initialize the application.
 */
void setup() {
#if LOGGING
    initLogger();
#endif
    bcu.begin(0x4c, 0x438, 0x6); // 4Sense
    LOG("diagnosis on %x", EE_DIAGNOSIS);

    initSensors();
}
