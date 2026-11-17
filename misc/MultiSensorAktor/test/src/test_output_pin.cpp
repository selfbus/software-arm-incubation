/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <OutputPin.h>

#include <catch.hpp>

namespace
{

OutputPinConfig config(const uint8_t invertByte)
{
    OutputPinConfig result {};
    reinterpret_cast<uint8_t&>(result.Invert) = invertByte;
    return result;
}

} // namespace

TEST_CASE("Output level without invert follows the state", "[OutputPin]")
{
    REQUIRE(OutputPin::outputLevel(config(0), false) == false);
    REQUIRE(OutputPin::outputLevel(config(0), true) == true);
}

TEST_CASE("Output level with invert 1 is the inverse state", "[OutputPin]")
{
    REQUIRE(OutputPin::outputLevel(config(1), false) == true);
    REQUIRE(OutputPin::outputLevel(config(1), true) == false);
}

// ETS writes "Ausgang invertieren" as 0x80. Read as bool, the firmware kept the pin high (0x80 ^ state).
TEST_CASE("Output level with invert as ETS writes it", "[OutputPin]")
{
    REQUIRE(OutputPin::inverted(config(0x80)));
    REQUIRE_FALSE(OutputPin::inverted(config(0)));
    REQUIRE(OutputPin::outputLevel(config(0x80), false) == true);
    REQUIRE(OutputPin::outputLevel(config(0x80), true) == false);
}
