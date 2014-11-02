/*
 *  datapoint_types_test.cpp - Tests for the datapoint types
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "catch.hpp"
#include "sblib/eib/datapoint_types.h"

#include <limits.h>


TEST_CASE("Datapoint type conversion: dptToFloat","[SBLIB]")
{
    REQUIRE(dptToFloat(0) == 0);

    REQUIRE(dptToFloat(1) == 1);
    REQUIRE(dptToFloat(2) == 2);
    REQUIRE(dptToFloat(2047) == 0x07ff);
    REQUIRE(dptToFloat(2048) == 0x0c00);
    REQUIRE(dptToFloat(2050) == 0x0c01);
    REQUIRE(dptToFloat(4096) == 0x1400);
    REQUIRE(dptToFloat(8192) == 0x1c00);
    REQUIRE(dptToFloat(67043328) == 0x7ffe);
    REQUIRE(dptToFloat(67076095) == 0x7ffe);
    REQUIRE(dptToFloat(67076096) == 0x7fff);
    REQUIRE(dptToFloat(67076097) == 0x7fff);
    REQUIRE(dptToFloat(INT_MAX) == 0x7fff);

    REQUIRE(dptToFloat(-1) == 0x87ff);
    REQUIRE(dptToFloat(-2) == 0x87fe);
    REQUIRE(dptToFloat(-2048) == 0x8000);
    REQUIRE(dptToFloat(-4096) == 0x8800);
    REQUIRE(dptToFloat(-67108864) == 0xf800);
    REQUIRE(dptToFloat(-67108865) == 0x7fff);
    REQUIRE(dptToFloat(INT_MIN) == 0x7fff);
}

TEST_CASE("Datapoint type conversion: dptFromFloat","[SBLIB]")
{
    REQUIRE(dptFromFloat(0) == 0);

    REQUIRE(dptFromFloat(1) == 1);
    REQUIRE(dptFromFloat(2) == 2);
    REQUIRE(2047 == dptFromFloat(0x07ff));
    REQUIRE(2048 == dptFromFloat(0x0c00));
    REQUIRE(4096 == dptFromFloat(0x1400));
    REQUIRE(8192 == dptFromFloat(0x1c00));
    REQUIRE(67043328 == dptFromFloat(0x7ffe));
    REQUIRE(INVALID_DPT_FLOAT == dptFromFloat(0x7fff));

    REQUIRE(-1 == dptFromFloat(0x87ff));
    REQUIRE(-2 == dptFromFloat(0x87fe));
    REQUIRE(-2048 == dptFromFloat(0x8000));
    REQUIRE(-4096 == dptFromFloat(0x8800));
    REQUIRE(-67108864 == dptFromFloat(0xf800));
}
