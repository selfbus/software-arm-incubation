/*
 * dimming.cpp
 *
 *  Created on: 21.02.2020
 *      Author: X
 */

#include <dimming.h>
#include <com_objs.h>
#include "config.h"
#include "DimmingCurves.h"
#include <sblib/eibMASK0701.h>


void dimming::init(int ch)
{
	channel = ch;
}

void dimming::start(int destination, int speed)
{
	if (!blocked1 && !blocked2)
	{
		dimStartTime = millis();
		dimSpeed = speed*1000; 				// speed in s, dimSpeed in ms
		dimStartValue = actualDimValue;
		dimDestinationValue = MAXOUTPUTVALUE * destination / 255.0f;  //destination 0-255, dimDestinationValue 0-MAXOUTPUTVALUE
		finished = false;
		isDimming = true;
		isOn = true;
		if (destination > 0)
		{
			lastDimValue = destination;
		}
		if (dimSpeed == 0)
		{  //Wert sofort einstellen
			actualDimValue = dimDestinationValue;
			this->stop();
			setOutput(channel, actualDimValue);
		}
	}
}

void dimming::stop()
{
	finished = true;
	isDimming = false;
	if (actualDimValue > MAXOUTPUTVALUE)
	{		//Ausgangsgröße begrenzen
		actualDimValue = MAXOUTPUTVALUE;
	}
	else if (actualDimValue <= 0)
	{					//Ausgangsgröße begrenzen
		actualDimValue = 0;
		isOn = false;
		isOnOneCycle = true;
		isOffOneCycle = true;
	}
}

void dimming::checkperiodic()
{
	if (!isDimming)
	{
	    return;
	}

	int tRest;
	uint32_t dimTimeElapsed = millis() - dimStartTime;
	bool isDimmingUp = dimStartValue <= dimDestinationValue;
    if (isDimmingUp)
    {
        tRest = dimSpeed * (MAXOUTPUTVALUE - dimStartValue) / MAXOUTPUTVALUE;
        if (tRest == 0) ///\todo implement a better catch devision by zero
        {
            actualDimValue = dimDestinationValue;
        }
        else
        {
            actualDimValue = dimStartValue + (dimTimeElapsed * (MAXOUTPUTVALUE - dimStartValue) / tRest);
        }

        if (actualDimValue >= dimDestinationValue)
        {
            //Zielhelligkeit erreicht
            actualDimValue = dimDestinationValue;
            this->stop();
        }
    }
    else
    {
        tRest = dimSpeed * dimStartValue / MAXOUTPUTVALUE;
        if (tRest == 0) ///\todo implement a better catch devision by zero
        {
            actualDimValue = dimDestinationValue;
        }
        else
        {
            actualDimValue = dimStartValue - (dimTimeElapsed * dimStartValue / tRest);
        }
        // TODO minimale Helligkeit bearbeiten
        if (actualDimValue <= dimDestinationValue)
        {
            //Zielhelligkeit erreicht
            actualDimValue = dimDestinationValue;
            this->stop();
        }
    }
    setOutput(channel, actualDimValue);
}

int dimming::getactualdimvalue()
{
	return actualDimValue * 255.0f / MAXOUTPUTVALUE;		// Rückgabe im KNX-Format 0-255
}

int dimming::getlastdimvalue()
{
	return lastDimValue;
}

bool dimming::getswitchstatus()
{
	return isOn;
}

bool dimming::getIsDimming()
{
	return isDimming;
}

bool dimming::changedtoon()
{
	if (isOn && isOnOneCycle)
	{
		isOnOneCycle = false;
		return true;
	}
	return false;
}

bool dimming::changedtooff()
{
	if (!isOn && isOffOneCycle)
	{
		isOffOneCycle = false;
		return true;
	}
	return false;
}
