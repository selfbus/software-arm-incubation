
#include "common.h"

APP_VERSION("SBlight ", "1", "10") // Don't forget to also change the build-variable sw_version

void loop() {
    sc->readValues();
    sc->doPeriodics();
    // Sleep up to 1 millisecond if there is nothing to do
    waitForInterrupt();
}

/**
 * The processing loop while no KNX-application is loaded
 */
void loop_noapp()
{

}
