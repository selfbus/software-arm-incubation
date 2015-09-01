/*
 *  threshold.h - The class defining the threshold behavior
 *
 *  Copyright (c) 2015 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#ifndef THRESHOLD_H_
#define THRESHOLD_H_

#include <sblib/timeout.h>

enum
{
    //!< The value threshold was above the upper limit for the configured time
    IN_UPPER_LIMIT          = 0x01
    //!< The value threshold was below the lower limit for the configured time
,   IN_LOWER_LIMIT          = 0x02
    //!< The value is above the upper limit and the monitor time is running
,   LOWER_LIMIT_TIME_ACTIVE = 0x04
    //!< The value is below the lower limit and the monitor time is running
,   UPPER_LIMIT_TIME_ACTIVE = 0x08
    //!< Either upper or the lower limit was just engaged. This is only active
    //!< for one cycle
,   LIMIT_JUST_ENGAGED      = 0x80
};

class Threshold
{
public:
    Threshold();

    /**
     * Handle the periodic checks for the threshold object
     *
     * @param value - the current value used to make the threshold checks.
     */
    void periodic(unsigned int value);

    /**
     * Checks if the upper limit was reached in this cycle.
     *
     * @return True if the upper limit was reached in this cycle.
     */
    bool upperLimitJustExceeded();

    /**
     * Checks if the lower limit was reached in this cycle.
     *
     * @return True if the lower slimit was reached in this cycle.
     */
    bool lowerLimitJustUndershot();

    // the following "internal" variables are public to allow easy configuration from
    // the outside.
                  //!< the com object number used to send the threshold state
             int  objNumber;
                  //!< if != -1, this object will be sent without the monitoring delay
             int  immediateObjNumber;
                  //!< the lower limit
    unsigned int  lowerLimit;
                  //!< the upper limit
    unsigned int  upperLimit;
                  //!< The time (in ms) the value has to be below the lower limit to trigger the "below threshold" action
    unsigned int  lowerLimitTime;
                  //!< The time (in ms) the value has to be above the upper limit to trigger the "above threshold" action
    unsigned int  upperLimitTime;
                  //!< a com object number which can be used to change the lower limit from outside
             int  changeLowerLimit;
                  //!< a com object number which can be used to change the upper limit from outside
             int  changeUpperLimit;
                  //!< action for the "below threshold"
    unsigned char sendLimitExceeded;
                  //!< action for the "above threshold"
    unsigned char sendLowerDeviation;
                  //!< time in (ms) for the cyclic repetition of the threshold state (0 means no cyclic repetition)
    unsigned int  cycleTime;
                  //!< com object number of the blocking object (-1 means the threshold cannot be blocked)
    int           blockObjNo;


private:
    unsigned int state;
    Timeout cycleTimeout;
    Timeout limitTimeout;
};

inline bool Threshold::upperLimitJustExceeded()
{
    return (state & (IN_UPPER_LIMIT | LIMIT_JUST_ENGAGED))
        == (IN_UPPER_LIMIT | LIMIT_JUST_ENGAGED);
}
inline bool Threshold::lowerLimitJustUndershot()
{
    return (state & (IN_LOWER_LIMIT | LIMIT_JUST_ENGAGED))
        == (IN_LOWER_LIMIT | LIMIT_JUST_ENGAGED);
}

#endif /* THRESHOLD_H_ */
