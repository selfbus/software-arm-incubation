
#include "common.h"

APP_VERSION("SB4sense", "2", "10")

void loop() {
    for (auto &sc : configs) {
        if (!sc.isInitialized()) {
            if (millis() > nextInit) {
                initSensors(false);
            }
        }
        else {
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
