
#include "common.h"

APP_VERSION("SB4sense", "1", "10")

void loop() {
    if (needSensorInit) {
        if (millis() > nextInit) initSensors();
            }
    if (!needSensorInit) {
        for (auto &sc : configs) {
            sc.sampleValues();
            sc.readValues();
            sc.doPeriodics();
        }
    }
    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle()) waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
