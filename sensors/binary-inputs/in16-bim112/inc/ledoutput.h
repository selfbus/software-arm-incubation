/*
 * ledoutput.h
 *
 *  Created on: 29.01.2018
 *      Author: uwe223
 */

#ifndef LEDOUTPUT_H_
#define LEDOUTPUT_H_


#include "channel.h"
#include "config.h"

/*
 *
 */
class _LedOutput_: public Channel
{
protected:
    _LedOutput_(unsigned int no, unsigned int longPress) :
            Channel(no, longPress)
    {
    }
    ;
public:
    //virtual void setLock(unsigned int value);
};

class LedOutput: public _LedOutput_
{
public:
	LedOutput(unsigned int no, unsigned int longPress, unsigned int channelConfig,
            unsigned int busReturn, unsigned int value);
    virtual void checkPeriodic(void);
    virtual void objectChanged(int value);
private:
    bool outputValue;
    bool blink;
    //bool pulse;
    bool invert;
    bool objectValueUnknown;
};


#endif /* LEDOUTPUT_H_ */
