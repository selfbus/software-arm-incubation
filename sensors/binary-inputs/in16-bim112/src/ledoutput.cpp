/*
 * ledoutput.cpp
 *
 *  Created on: 29.01.2018
 *      Author: uwe223
 */

#include "ledoutput.h"

LedOutput::LedOutput(unsigned int no, unsigned int longPress,
        unsigned int channelConfig, unsigned int busReturn, unsigned int value) :
        _LedOutput_(no, longPress)
{
	pinMode(inputPins[number], OUTPUT);
	digitalWrite(inputPins[number], false);
	objectValueUnknown = true;
	int ledParamBase = currentVersion->logicBaseAddress + 57 + currentVersion->noOfChannels * 4;
	invert = (bcu.userEeprom->getUInt8(ledParamBase) >> number) & 1;  //switch inverted/not inverted
	//pulse  = (bcu.userEeprom->getUInt8(ledParamBase + 1) >> number) & 1;  //pulse by unknown object
	if ((bcu.userEeprom->getUInt8(ledParamBase + 1) >> number) & 1)		//pulse by unknown object
	{
		timeout.start(DELAYPULSEON);
		digitalWrite(inputPins[number], 1);
	}
	blink = (bcu.userEeprom->getUInt8(ledParamBase + 2) >> number) & 1;  //blink function

}

void LedOutput::objectChanged(int value)
{
	objectValueUnknown = false;
	outputValue = value ^ invert;
	digitalWrite(inputPins[number], outputValue);
	if (!outputValue)
		timeout.stop();
	if (outputValue && blink)
		timeout.start(DELAYBLINK);
}


void LedOutput::checkPeriodic(void)
{
    if (timeout.started() && timeout.expired())
    {
    	bool value = !digitalRead(inputPins[number]);
    	if (objectValueUnknown)
    	{
    		digitalWrite(inputPins[number], value);
    		if (value)
    			timeout.start(DELAYPULSEON);
    		else
    			timeout.start(DELAYPULSEOFF);
    	}
    	else if (blink)
    	{
    		digitalWrite(inputPins[number], value);
    		timeout.start(DELAYBLINK);
    	}
    }
}
