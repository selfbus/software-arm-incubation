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

// Only the values 0 and 1 of the bool are characterised here; ETS writes 0x80 ("Ausgang invertieren"), with which the
// firmware keeps the pin high (hil/tests/test_msa_io.py::test_ausgang_invertiert, xfail).
TEST_CASE("Output level with invert 1 is the inverse state", "[OutputPin]")
{
    REQUIRE(OutputPin::outputLevel(config(1), false) == true);
    REQUIRE(OutputPin::outputLevel(config(1), true) == false);
}
