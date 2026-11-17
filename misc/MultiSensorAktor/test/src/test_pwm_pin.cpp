/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <PWMPin.h>

#include <catch.hpp>

namespace
{

PWMPinConfig config(const uint8_t fixedValueSwitch, const uint8_t onOn = 77, const uint8_t onOff = 11)
{
    PWMPinConfig result {};
    result.Invert = false;
    result.FixedValueSwitch = static_cast<PortFixedPwmValue>(fixedValueSwitch);
    result.FixedValueOnOn = onOn;
    result.FixedValueOnOff = onOff;
    return result;
}

} // namespace

TEST_CASE("PWM value without a fixed value follows the object", "[PWMPin]")
{
    const auto c = config(PortFixedPwmValueNever);
    for (const uint8_t v : {0, 1, 128, 254, 255})
    {
        REQUIRE(PWMPin::targetValue(c, v, false, 42) == v);
        REQUIRE(PWMPin::targetValue(c, v, true, 42) == v);
    }
}

// ETS: "Festen PWM-Wert bei Sperre setzen" writes 0x80 (PortFixedPwmValueOnLock) and the value into FixedValueOnOn.
// Characterised as is: the object value while the lock object is 1, the fixed value while it is 0.
TEST_CASE("PWM value with a fixed value on lock (as is)", "[PWMPin]")
{
    const auto c = config(PortFixedPwmValueOnLock, 77);
    REQUIRE(PWMPin::targetValue(c, 200, true, 42) == 200);
    REQUIRE(PWMPin::targetValue(c, 200, false, 42) == 77);
    REQUIRE(PWMPin::targetValue(c, 0, false, 42) == 77);
}

// Not reachable from ETS (FixedValueOnOff has no parameter), kept as is.
TEST_CASE("PWM value with a fixed value always", "[PWMPin]")
{
    const auto c = config(PortFixedPwmValueAlways, 77, 11);
    REQUIRE(PWMPin::targetValue(c, 1, false, 42) == 11);
    REQUIRE(PWMPin::targetValue(c, 200, true, 42) == 11);
    REQUIRE(PWMPin::targetValue(c, 0, false, 42) == 77);
}

TEST_CASE("PWM value with an unknown fixed value switch stays", "[PWMPin]")
{
    for (const uint8_t s : {1, 127, 130, 255})
    {
        REQUIRE(PWMPin::targetValue(config(s), 200, false, 42) == 42);
    }
}
