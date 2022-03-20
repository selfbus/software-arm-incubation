
#include "common.h"
#include <sblib/eib/sblib_default_objects.h>

void loop() {
    sc->readValues();
    sc->doPeriodics();
    // Sleep up to 1 millisecond if there is nothing to do
    if (bus.idle()) waitForInterrupt();
}
