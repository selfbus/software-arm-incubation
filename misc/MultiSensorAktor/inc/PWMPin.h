/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef PWMPIN_H_
#define PWMPIN_H_

#include <sblib/eib/bcu_base.h>

#include <PWMPinConfig.h>
#include <GenericPin.h>
#include <GenericItem.h>

class PWMPin : public GenericPin
{
public:
	/**
	 * @param pwmChannel channel of pwmController that outputs the value, -1 for none (pins of a PCA9555D)
	 */
	PWMPin(uint8_t firstComObj, PWMPinConfig *config, GenericItem* parent, uint16_t& objRamPtr, int pwmChannel = -1);
	~PWMPin() = default;

	uint8_t GetState(uint32_t now, uint8_t updatedObjectNo);

	/**
	 * The PWM value to output after the value or the lock object changed.
	 *
	 * @param config      configuration of the channel (FixedValueSwitch, FixedValueOnOn, FixedValueOnOff)
	 * @param objectValue value of the com object "output" (0-255)
	 * @param lock        value of the com object "lock"
	 * @param current     the value output so far, kept for an unknown FixedValueSwitch
	 * @return the new PWM value (0-255)
	 */
	[[nodiscard]] static uint8_t targetValue(const PWMPinConfig& config, uint8_t objectValue, bool lock,
	                                         uint8_t current);

	/**
	 * Whether the output is inverted. ETS writes "Ausgang invertieren" as 0x80, so the byte is read, not the bool.
	 */
	[[nodiscard]] static bool inverted(const PWMPinConfig& config);

	int ConfigLength() { return sizeof(PWMPinConfig); }
	int ComObjCount() { return 3; }

protected:
	PWMPinConfig* config;
	uint8_t value = 0;
	GenericItem* parent;
	int pwmChannel;
};

#endif
