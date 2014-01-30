/*
 *  Timeout.h - Simple timeout handling class
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */


#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include "sblib/internal/variables.h"

class Timeout/**
 * A simple timeout class which
 * - can be queried if the specified timeout has expired
 * - can be stopped
 * - queried if the timeout has been started
 *
 * Example:
 *
 *     Timeout t;
 *     ...
 *     t.start(10); // starts a timeout which will expire in 10ms
 *
 *     t.expired(); // returns of the timeout has already expired or not
 */
{
public:
	/** Create a new timeout object and put it into the stopped state */
	Timeout(void)
	{
		stop();
	};

	/** Set the new timeout to be in ms mille seconds
	 * @param   ms  The timeout value in mille seconds
	 */
	void start(unsigned int ms)
	{
		timeout = systemTime + ms;
		if(timeout == Timeout::STOPPED) timeout++;
	};

	/** Returns wether the timeout has expired or not
	 *
	 * @return If the timeout has exipred
	 */
	bool expired(void)
	{
		if (timeout != Timeout::STOPPED)
		{
			if(((int)(systemTime - timeout)) > 0)
			{
				stop();
				return true;
			}
		}
		return false;
	};

	/** Returns wether the timeout has been started or nor
	 *
	 * @return If the time as been started
	 */
	bool started(void)
	{
		return timeout != Timeout::STOPPED;
	}

	/** Stops the currently running timeout */
	void stop(void)
	{
		timeout = Timeout::STOPPED;
	};

protected:
	unsigned int timeout;

private:
	enum STATE { STOPPED  = 0};
};


#endif /* TIMEOUT_H_ */
