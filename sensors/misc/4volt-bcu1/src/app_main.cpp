
#include "common.h"

void loop() {
    for (auto &sc : configs) {
        sc.readValues();
        sc.doPeriodics();
    }
    // Sleep up to 1 millisecond if there is nothing to do
    if (bcu.bus->idle()) waitForInterrupt();
}
