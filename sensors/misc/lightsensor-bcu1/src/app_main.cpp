
#include "common.h"

APP_VERSION("SBlight ", "1", "10")

void loop() {
    sc->readValues();
    sc->doPeriodics();
    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle()) waitForInterrupt();
}
