/*
 *  timeout_test.cpp - Tests for the timeout class
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "catch.hpp"
#include "protocol.h"

#include "sblib/Timeout.h"

Timeout to;

static void _check_expired_no(void * refState)
{
	REQUIRE(to.expired() == false);
	REQUIRE(to.started() == true);
	REQUIRE(to.stopped() == false);
}
static void _start_to_20(void * refState)
{
	to.start(20);
	_check_expired_no(refState);
}

static void _check_expired_yes(void * refState)
{
	REQUIRE(to.expired() == true);
	REQUIRE(to.started() == false);
	REQUIRE(to.stopped() == true);
}
static void _check_time_wrap(void * refState)
{
	_check_expired_no(refState);
	REQUIRE(systemTime < 10);
}

static Telegram testCaseTelegrams[] =
{ {TIMER_TICK,  6,  0, _start_to_20,       {}}
, {TIMER_TICK,  19, 0, _check_expired_no,  {}}
, {TIMER_TICK,   1, 0, _check_expired_no,  {}}
, {TIMER_TICK,   1, 0, _check_expired_yes, {}}
// force a wrap-around of the systemTime
, {TIMER_TICK, (unsigned int) -38, 0, _start_to_20,       {}}
, {TIMER_TICK,  19, 0, _check_expired_no,  {}}
, {TIMER_TICK,   1, 0, _check_time_wrap,   {}}
, {TIMER_TICK,   1, 0, _check_expired_yes, {}}
, {END}
};

static void tc_setup(void)
{
    REQUIRE(to.started() == false);
}

static Test_Case testCase =
{
  "Timeout Test"
, 0x0004, 0x2060, 0x01
, NULL
, tc_setup
, NULL
, NULL
, NULL
, testCaseTelegrams
};

TEST_CASE("Test of the timeout primitives","[TIMEOUT][SBLIB]")
{
    executeTest(& testCase);
}
