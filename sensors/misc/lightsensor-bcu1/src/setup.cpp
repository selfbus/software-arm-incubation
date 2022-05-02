//
// Created by Mario Theodoridis on 22.05.21.
//
#include "common.h"

BCU1 bcu = BCU1();

SensorConfig config;
SensorConf sc = &config;

/**
 * Works around the fact that the ram location for objects isn't set in the
 * com table, so they all overwrite each other into the same RM location.
 * @param comNo each com object gets 2 bytes
 */
void fixRamLoc(COM comNo) {
    // uint8_t* l = (uint8_t*)&objectConfig(comNo)->dataPtr;
    ComObjectsBCU1* comObjs = (ComObjectsBCU1*)bcu.comObjects;
    uint8_t* l = (uint8_t*)&(comObjs->objectConfigBCU1(comNo)->dataPtr); ///\todo this looks stupid and i think it is.
    *l = comNo * 2;
}

void initSensor() {
    LOG("\n* * * Init Sensors * * *");
    // spiffy those puppies
    //memset(&config[0], 0, sizeof(SensorConfig) * NUM_SENSORS);
    // so if inits run clean we're fine
    if (EE_TYPE_SENSOR_1 == SENSOR_TYP_NO_SENSOR) {
        LOG("Sensor 1 is not used for knx");
        return;
    }

    if (!sc->init(0, EE_TYPE_SENSOR_1,
                  EE_SEND_TEMPERATURE_AS_SENSOR_1,
                  (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_1,
                  COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_1)) {
        return;
    }

    sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_1,
                      EE_TIME_FACTOR_MEAS_VALUE_SENSOR_1,
                      EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                      EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                      EE_TIMEBASE_TEMPERATURE_SENSOR_1);

    sc->setDiffTrigger(EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_1,
                       EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_1);

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

/**
 * Initialize the application.
 */
BcuBase* setup() {
#if LOGGING
    initLogger();
#endif
    bcu.begin(0x4c, 0x438, 0x6); // 4Sense
    LOG("diagnosis on %x", EE_DIAGNOSIS);
    initSensor();
    return (&bcu);
}
