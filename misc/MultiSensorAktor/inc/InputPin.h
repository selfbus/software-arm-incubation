/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef INPUTPIN_H_
#define INPUTPIN_H_

#include <sblib/eib/bcu_base.h>
#include <GenericPin.h>
#include <InputPinConfig.h>

class InputPin : public GenericPin
{
public:
	// forcedConfig is supplied only for the native ARM input modes. A null
	// pointer keeps the legacy 4-object/1-bit behaviour used by PCA9555D.
	InputPin(byte firstComObj, InputPinConfig *config, uint16_t& objRamPointer,
		InputPinForcedConfig *forcedConfig = nullptr);
	~InputPin() = default;

	void PutValue(uint32_t now, int val);

	int ConfigLength() { return sizeof(InputPinConfig) + (forcedConfig != nullptr ? sizeof(InputPinForcedConfig) : 0); }
	int ComObjCount() { return forcedConfig != nullptr ? 6 : 4; }

protected:
	enum ComObjectOffset : byte
	{
		ShortObject = 0,
		LongObject = 1,
		LockObject = 2,
		// Offset 3 is intentionally kept for the pre-existing output/DHT
		// status object of PortPinSimple.
		ShortStatusObject = 4,
		LongStatusObject = 5
	};

	void setObj(int idx, byte item, bool is2Bit, int statusIdx = -1);
	byte shortAction(bool high) const;
	byte longAction(bool high) const;

	InputPinConfig* config;
	InputPinForcedConfig* forcedConfig;
	Debouncer debouncer = Debouncer();
	bool longActive = false;
	bool longPending = false;
	bool shortActionIs2Bit = false;
	bool longActionIs2Bit = false;
	bool shortToggleActive = false;
	bool longToggleActive = false;
	uint32_t valueSet = 0;
	byte lastLongValue = 0;
};
#endif /* INPUTPIN_H_ */
