/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef HOERMANNSTATE_H_
#define HOERMANNSTATE_H_

class HoermannState {
public:
	bool doorOpen = false;
	bool doorClosed = false;
	bool optionRelay = false;
	bool lightRelay = false;
	bool error = false;
	bool directionDown = false;
	bool moving = false;
	bool ventingPos = false;
	bool preWarning = false;

	bool equals(HoermannState &other)
	{
		return other.doorOpen == doorOpen &&
				other.doorClosed == doorClosed &&
				other.optionRelay == optionRelay &&
				other.lightRelay == lightRelay &&
				other.error == error &&
				other.directionDown == directionDown &&
				other.moving == moving &&
				other.ventingPos == ventingPos &&
				other.preWarning == preWarning;
	}

	void copyTo(HoermannState &target)
	{
		target.doorOpen = doorOpen;
		target.doorClosed = doorClosed;
		target.optionRelay = optionRelay;
		target.lightRelay = lightRelay;
		target.error = error;
		target.directionDown = directionDown;
		target.moving = moving;
		target.ventingPos = ventingPos;
		target.preWarning = preWarning;
	}
};

#endif /* HOERMANNSTATE_H_ */
