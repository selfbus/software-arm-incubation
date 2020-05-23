
#include "common.h"
#include <sblib/eib/sblib_default_objects.h>


void loop() {
    for (auto &sc : configs) {
        sc.sampleValues();
        sc.readValues();
        sc.doPeriodics();
    }
//    if (millis() > 6000) NVIC_SystemReset();
    // Sleep until the next interrupt happens
    __WFI();
}
