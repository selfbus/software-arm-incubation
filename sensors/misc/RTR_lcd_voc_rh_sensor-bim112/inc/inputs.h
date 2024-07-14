#ifndef INPUTS_H
#define INPUTS_H

#include <sblib/core.h>

// quantity of switches
#define NUM_CHANNELS 2

// Debouncers for inputs
extern Debouncer inputDebouncer[NUM_CHANNELS]; // debouncing for both buttons

extern unsigned int debounceTime;

// press key this time to generate a long press command [ms]
#define LONGPRESSTIME 350

extern uint8_t inputChangedMem[NUM_CHANNELS];

extern unsigned int TasterPressTime[NUM_CHANNELS];

enum SWITCH_PRESS {
	NO_PRESS,
	SHORT_PRESS,
	LONG_PRESS
};

// Input pins
// changes have to been also in inputs.cpp!
static const int inputPins[] = { PIO2_10, PIO1_7 }; // input pins for both buttons

void init_inputs(void);

void handlePeriodicInputs(void);

#endif
