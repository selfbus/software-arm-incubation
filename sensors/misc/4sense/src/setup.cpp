//
// Created by mario on 17.05.20.
//
#include <cstring>
#include "common.h"

SensorConfig configs[NUM_SENSORS];

void initSensors() {
    memset(&configs[0], 0, sizeof(SensorConfig) * NUM_SENSORS);
    SensorConf sc;

    while (EE_TYPE_SENSOR_1 >= SENSOR_TYP_TEMPERATURE_SENSOR) {
        sc = &configs[0];

        if (!sc->init(0, EE_TYPE_SENSOR_1,
                EE_SEND_TEMPERATURE_AS_SENSOR_1,
                (int8_t) EE_TEMPERATURE_OFFSET_IN_0_1K_SENSOR_1,
                COM_TEMPERATURE_SEND_TEMPERATURE_AS_SENSOR_1)) {
            break;
        }
        sc->setSendPeriod(EE_MEASUREMENT_VALUE_TEMP_SENSOR_1,
                EE_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                EE_TIME_FACTOR_MEAS_VALUE_SENSOR_1,
                EE_FACTOR_SEND_PERIODICALLY_TEMPERATURE_SENSOR_1,
                EE_TIMEBASE_TEMPERATURE_SENSOR_1);

        if (EE_SEND_IF_SIGNAL_CHANGES_TEMPERATURE_SENSOR_1 == JA_NEIN_YES) {
            sc->setDiffTrigger(
                EE_SEND_IF_SIGNAL_CHANGES_BY_0_1K_TEMPERATURE_SENSOR_1);
        }

        if (EE_SEND_IF_THRESHOLD_1_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
            break;
        }
        sc->setThresholdPeriod(EE_THRESHOLD_TEMPERATURE_SENSOR_1,
                EE_SEND_PERIODICALLY_THRESHOLD_1_SENSOR_1,
                EE_BUS_RECOVERY_THRESHOLD_TEMP_TIMEFACTOR,
               EE_SEND_PERIODICALLY_FACTOR_THRESHOLD_1_SENSOR_1,
                EE_SEND_PERIODICALLY_TIMEBASE_THRESHOLD_1_SENSOR_1);

        sc->setThreshold(0, EE_VALUE_IN_0_1K_THRESHOLD_1_SENSOR_1,
                EE_SEND_IF_THRESHOLD_1_SENSOR_1,
                COM_THRESHOLD_1_TEMPERATURE_THRESHOLD_1_SENSOR_1);

        if (EE_SEND_IF_THRESHOLD_2_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
            break;
        }
        sc->setThreshold(1, EE_VALUE_IN_0_1K_THRESHOLD_2_SENSOR_1,
                EE_SEND_IF_THRESHOLD_2_SENSOR_1,
                COM_THRESHOLD_2_TEMPERATURE_THRESHOLD_2_SENSOR_1);

        if (EE_SEND_IF_THRESHOLD_3_SENSOR_1 == GRENZWERT_REAKTION_DEACTIVATED) {
            break;
        }
        sc->setThreshold(2, EE_VALUE_IN_0_1K_THRESHOLD_3_SENSOR_1,
                EE_SEND_IF_THRESHOLD_3_SENSOR_1,
                COM_THRESHOLD_3_TEMPERATURE_THRESHOLD_3_SENSOR_1);
        // break out of this "if" clause
        break;
    }
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
