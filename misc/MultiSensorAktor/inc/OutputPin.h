/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef OUTPUTPIN_H_
#define OUTPUTPIN_H_

#include <sblib/eib/bcu_base.h>

#include <GenericPin.h>
#include <OutputPinConfig.h>

class OutputPin : public GenericPin
{
public:
	OutputPin(uint8_t firstComObj, OutputPinConfig *config, uint16_t& objRamPointer);
	~OutputPin() = default;

	uint8_t GetState(uint32_t now, uint8_t updatedObjectNo);

	/**
	 * Level of the pin for the switching state, after "Ausgang invertieren".
	 *
	 * @param config configuration of the output (Invert)
	 * @param state  switching state (object value, blink phase)
	 * @return the level to output
	 */
	[[nodiscard]] static bool outputLevel(const OutputPinConfig& config, bool state);

	/**
	 * Whether the output is inverted. ETS writes "Ausgang invertieren" as 0x80, so the byte is read, not the bool.
	 */
	[[nodiscard]] static bool inverted(const OutputPinConfig& config);

	int ConfigLength() { return sizeof(OutputPinConfig); }
	int ComObjCount() { return 4; }

protected:
	OutputPinConfig* config;
	bool sw = false;
	bool stateDuringLock = false;
	bool blinkObjState = false;
	bool blinkState = false;
	bool blinkStateDuringLock = false;
	bool blink = false;
	bool lastState = false;
	bool locked = false;
	uint32_t blinkActionTime = 0;

	PortOutLockAction lockAction;
	PortOutLockAction lockBlinkAction;
	bool lockInvert;
	bool lockResume;

	void setType(uint32_t now);
};

#endif /* OUTPUTPIN_H_ */
