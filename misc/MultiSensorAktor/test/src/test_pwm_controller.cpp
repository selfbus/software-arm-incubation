/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include <cstring>

#include <sblib/digital_pin.h>
#include <sblib/ioports.h>

#include <ARMPinItem.h>
#include <PWMPin.h>
#include <PwmController.h>

#include <catch.hpp>

namespace
{

using TimerId = PwmController::TimerId;
constexpr uint32_t P = PwmController::periodTicks;
constexpr uint32_t noEdge = 0xFFFFFFFF;

// The 16 IOs of the io-16x-fused-fm (hil/io/README.md of the workspace)
constexpr uint32_t moduleIos[16] = {PIO3_4, PIO0_3, PIO2_5, PIO0_7, PIO3_5, PIO2_1, PIO0_6, PIO2_9,
                                    PIO0_8, PIO2_10, PIO1_10, PIO0_9, PIO1_4, PIO0_2, PIO2_7, PIO2_8};

/** Emulated registers back to zero, as after a reset. */
void resetRegisters()
{
    for (LPC_TMR_TypeDef* t : {LPC_TMR16B0, LPC_TMR16B1, LPC_TMR32B0, LPC_TMR32B1})
    {
        std::memset(t, 0, sizeof(*t));
    }
    for (LPC_GPIO_TypeDef* g : {LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3})
    {
        std::memset(g, 0, sizeof(*g));
    }
    std::memset(LPC_IOCON, 0, sizeof(*LPC_IOCON));
    std::memset(NVIC, 0, sizeof(*NVIC));
    LPC_SYSCON->SYSAHBCLKCTRL = 0;
}

uint32_t mask(const uint32_t pin)
{
    return digitalPinToBitMask(pin);
}

/** What the interrupt finds: the flag of @p match set, the counter at @p tc. */
void interrupt(PwmController& pwm, const int match, const uint32_t tc)
{
    LPC_TMR32B1->IR = 1 << match;
    LPC_TMR32B1->TC = tc;
    pwm.softwareTimerInterrupt();
}

void startPeriod(PwmController& pwm)
{
    interrupt(pwm, 0, P - 1);
}

} // namespace

TEST_CASE("PWM hardware channels of the pins", "[PwmController]")
{
    struct Expected
    {
        uint32_t pin;
        TimerId timer;
        uint8_t match;
    };
    // Match outputs of the LPC1115 on CT16B0, CT32B0, CT32B1 except the period registers (CT16B0 MR3, CT32B0 MR1,
    // CT32B1 MR0) and the software edge (CT32B1 MR1)
    for (const auto& e : {Expected {PIO0_8, TimerId::CT16B0, 0}, Expected {PIO3_0, TimerId::CT16B0, 0},
                          Expected {PIO0_9, TimerId::CT16B0, 1}, Expected {PIO3_1, TimerId::CT16B0, 1},
                          Expected {PIO3_2, TimerId::CT16B0, 2}, Expected {PIO1_6, TimerId::CT32B0, 0},
                          Expected {PIO2_5, TimerId::CT32B0, 0}, Expected {PIO0_1, TimerId::CT32B0, 2},
                          Expected {PIO2_7, TimerId::CT32B0, 2}, Expected {PIO0_11, TimerId::CT32B0, 3},
                          Expected {PIO2_8, TimerId::CT32B0, 3}, Expected {PIO1_4, TimerId::CT32B1, 3}})
    {
        const auto h = PwmController::hardwareChannel(e.pin);
        CAPTURE(e.pin);
        REQUIRE(h.timer == e.timer);
        REQUIRE(h.match == e.match);
    }
    // CT16B1 (bus: PIO1_9, PIO1_10, PIO2_4), SWD (PIO0_10, PIO1_3), period/edge registers, pins without a match
    for (const uint32_t pin : {PIO1_9, PIO1_10, PIO2_4, PIO0_10, PIO1_3, PIO1_7, PIO2_6, PIO1_1, PIO1_2, PIO0_3,
                               PIO2_1, PIO3_4, PIO3_5, PIO2_9, PIO2_10, PIO0_2})
    {
        CAPTURE(pin);
        REQUIRE(PwmController::hardwareChannel(pin).timer == TimerId::none);
    }
}

TEST_CASE("PWM match values and active time", "[PwmController]")
{
    REQUIRE(P == 10000);
    REQUIRE(PwmController::activeTicks(0) == 0);
    REQUIRE(PwmController::activeTicks(1) == 39);
    REQUIRE(PwmController::activeTicks(128) == 5020);
    REQUIRE(PwmController::activeTicks(254) == 9961);
    REQUIRE(PwmController::activeTicks(255) == P);

    // high from the match to the end of the period
    REQUIRE(PwmController::matchValue(0, false) == P); // never reached: low
    REQUIRE(PwmController::matchValue(255, false) == 0); // high
    REQUIRE(PwmController::matchValue(128, false) == P - 5020);
    REQUIRE(PwmController::matchValue(0, true) == 0);
    REQUIRE(PwmController::matchValue(255, true) == P);
    REQUIRE(PwmController::matchValue(128, true) == 5020);
}

TEST_CASE("PWM hardware channel drives the pin by the timer", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int ch = pwm.attach(PIO2_5, false); // IO3, CT32B0 MR0
    REQUIRE(pwm.channelHardware(ch).timer == TimerId::CT32B0);

    REQUIRE((LPC_SYSCON->SYSAHBCLKCTRL & (1 << 9)) != 0); // clock of CT32B0
    REQUIRE(LPC_TMR32B0->PR == 47);                      // 1 MHz
    REQUIRE(LPC_TMR32B0->MR1 == P - 1);                  // period
    REQUIRE(LPC_TMR32B0->MCR == (2 << 3));               // reset on MR1, no interrupt
    REQUIRE(LPC_TMR32B0->PWMC == 1);
    REQUIRE(LPC_TMR32B0->TCR == 1);
    REQUIRE(LPC_TMR32B0->MR0 == P);                      // value 0: low
    REQUIRE((LPC_IOCON->PIO2_5 & 7) == 1);               // function CT32B0_MAT0
    REQUIRE((NVIC->ISER[0] & (1 << TIMER_32_0_IRQn)) == 0);

    pwm.setValue(ch, 128);
    REQUIRE(LPC_TMR32B0->MR0 == P - 5020);
    pwm.setValue(ch, 255);
    REQUIRE(LPC_TMR32B0->MR0 == 0);

    const int inv = pwm.attach(PIO2_8, true); // IO16, CT32B0 MR3, inverted
    REQUIRE(LPC_TMR32B0->MR3 == 0);           // value 0 inverted: high
    REQUIRE(LPC_TMR32B0->PWMC == 0b1001);
    pwm.setValue(inv, 64);
    REQUIRE(LPC_TMR32B0->MR3 == PwmController::activeTicks(64));
    REQUIRE((LPC_TMR32B1->TCR & 1) == 0); // no software pin, CT32B1 stays off
}

TEST_CASE("PWM pins of the same match register: the second one in software", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    REQUIRE(pwm.channelHardware(pwm.attach(PIO0_8, false)).timer == TimerId::CT16B0);
    REQUIRE(pwm.channelHardware(pwm.attach(PIO3_0, false)).timer == TimerId::none);
    REQUIRE((LPC_IOCON->PIO3_0 & 7) == 0); // GPIO
    REQUIRE(LPC_TMR16B0->MR3 == P - 1);
    REQUIRE(LPC_TMR16B0->MCR == (2 << 9));
}

TEST_CASE("PWM software channel starts the scheduler", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int ch = pwm.attach(PIO0_3, false);
    REQUIRE(pwm.channelHardware(ch).timer == TimerId::none);
    REQUIRE((LPC_GPIO0->DIR & mask(PIO0_3)) != 0);
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[mask(PIO0_3)] == 0); // inactive
    REQUIRE(LPC_TMR32B1->PR == 47);
    REQUIRE(LPC_TMR32B1->MR0 == P - 1);
    REQUIRE(LPC_TMR32B1->MR1 == noEdge);
    REQUIRE(LPC_TMR32B1->MCR == (3 | (1 << 3))); // MR0 interrupt and reset, MR1 interrupt
    REQUIRE(LPC_TMR32B1->TCR == 1);
    REQUIRE((NVIC->ISER[0] & (1 << TIMER_32_1_IRQn)) != 0);

    const int inv = pwm.attach(PIO3_5, true);
    REQUIRE(LPC_GPIO3->MASKED_ACCESS[mask(PIO3_5)] == mask(PIO3_5)); // inactive inverted: high
    (void) inv;
}

TEST_CASE("PWM software schedule: start levels and edges in time order", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int a = pwm.attach(PIO0_3, false); // port 0
    const int b = pwm.attach(PIO0_7, true);  // port 0, inverted
    const int c = pwm.attach(PIO2_1, false); // port 2
    const int d = pwm.attach(PIO3_4, true);  // port 3, inverted
    pwm.setValue(a, 64);
    pwm.setValue(b, 192);
    pwm.setValue(c, 255);
    pwm.setValue(d, 0);
    pwm.update();
    REQUIRE(pwm.schedulePending());

    startPeriod(pwm);
    REQUIRE_FALSE(pwm.schedulePending());
    const auto& s = pwm.activeSchedule();
    REQUIRE(s.startMask[0] == (mask(PIO0_3) | mask(PIO0_7)));
    REQUIRE(s.edgeCount == 2);
    const uint32_t m0 = mask(PIO0_3) | mask(PIO0_7);
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[m0] == mask(PIO0_3)); // a active high, b active low
    REQUIRE(LPC_GPIO2->MASKED_ACCESS[mask(PIO2_1)] == mask(PIO2_1)); // 255: on, no edge
    REQUIRE(LPC_GPIO3->MASKED_ACCESS[mask(PIO3_4)] == mask(PIO3_4)); // 0 inverted: high, no edge
    REQUIRE(LPC_TMR32B1->MR1 == PwmController::activeTicks(64));

    interrupt(pwm, 1, PwmController::activeTicks(64));
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[mask(PIO0_3)] == 0);
    REQUIRE(LPC_TMR32B1->MR1 == PwmController::activeTicks(192));

    interrupt(pwm, 1, PwmController::activeTicks(192) + 1);
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[mask(PIO0_7)] == mask(PIO0_7)); // inverted: inactive high
    REQUIRE(LPC_TMR32B1->MR1 == noEdge);

    // the next period starts the same way again
    LPC_GPIO0->MASKED_ACCESS[m0] = 0;
    startPeriod(pwm);
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[m0] == mask(PIO0_3));
    REQUIRE(LPC_TMR32B1->MR1 == PwmController::activeTicks(64));
}

TEST_CASE("PWM software edges of the same time and port are one write", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int a = pwm.attach(PIO0_3, false);
    const int b = pwm.attach(PIO0_6, false);
    const int c = pwm.attach(PIO2_1, false);
    const int d = pwm.attach(PIO0_7, false);
    for (const int ch : {a, b, c})
    {
        pwm.setValue(ch, 100);
    }
    pwm.setValue(d, 10);
    pwm.update();
    startPeriod(pwm);
    const auto& s = pwm.activeSchedule();
    REQUIRE(s.edgeCount == 3);
    REQUIRE(s.edges[0].time == PwmController::activeTicks(10));
    REQUIRE(s.edges[1].time == PwmController::activeTicks(100));
    REQUIRE(s.edges[2].time == PwmController::activeTicks(100));
    REQUIRE(s.edges[1].port != s.edges[2].port);
    const auto& port0 = s.edges[1].port == 0 ? s.edges[1] : s.edges[2];
    REQUIRE(port0.mask == (mask(PIO0_3) | mask(PIO0_6)));
    REQUIRE(port0.value == 0);
}

TEST_CASE("PWM software edges passed while the interrupt waited are written at once", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int a = pwm.attach(PIO0_3, false);
    const int b = pwm.attach(PIO2_1, false);
    const int c = pwm.attach(PIO3_4, false);
    pwm.setValue(a, 50);
    pwm.setValue(b, 51); // 39 ticks later
    pwm.setValue(c, 200);
    pwm.update();
    startPeriod(pwm);
    LPC_GPIO0->MASKED_ACCESS[mask(PIO0_3)] = 1;
    LPC_GPIO2->MASKED_ACCESS[mask(PIO2_1)] = 1;

    // late by more than the second edge, not yet at the third
    interrupt(pwm, 1, PwmController::activeTicks(51) + 5);
    REQUIRE(LPC_GPIO0->MASKED_ACCESS[mask(PIO0_3)] == 0);
    REQUIRE(LPC_GPIO2->MASKED_ACCESS[mask(PIO2_1)] == 0);
    REQUIRE(LPC_GPIO3->MASKED_ACCESS[mask(PIO3_4)] == mask(PIO3_4));
    REQUIRE(LPC_TMR32B1->MR1 == PwmController::activeTicks(200));

    // within the guard of 2 ticks the edge is written now, not left to a match that could come too late
    interrupt(pwm, 1, PwmController::activeTicks(200) - 2);
    REQUIRE(LPC_GPIO3->MASKED_ACCESS[mask(PIO3_4)] == 0);
    REQUIRE(LPC_TMR32B1->MR1 == noEdge);
}

TEST_CASE("PWM software values wait for the start of a period", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int a = pwm.attach(PIO0_3, false);
    pwm.update();
    startPeriod(pwm);
    REQUIRE(pwm.activeSchedule().startValue[0] == 0);

    pwm.setValue(a, 255);
    pwm.update();
    REQUIRE(pwm.schedulePending());
    REQUIRE(pwm.activeSchedule().startValue[0] == 0); // not yet

    pwm.setValue(a, 128); // the interrupt hasn't taken 255 yet: waits
    pwm.update();
    startPeriod(pwm);
    REQUIRE(pwm.activeSchedule().startValue[0] == mask(PIO0_3));
    REQUIRE(pwm.activeSchedule().edgeCount == 0);

    pwm.update(); // now 128
    startPeriod(pwm);
    REQUIRE(pwm.activeSchedule().edgeCount == 1);
    REQUIRE(pwm.activeSchedule().edges[0].time == 5020);

    pwm.update(); // nothing changed: no new schedule
    REQUIRE_FALSE(pwm.schedulePending());
}

TEST_CASE("PWM with a hardware value doesn't change the software schedule", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    const int hw = pwm.attach(PIO0_9, false);
    pwm.attach(PIO0_3, false);
    pwm.update();
    startPeriod(pwm);
    pwm.setValue(hw, 77);
    pwm.update();
    REQUIRE_FALSE(pwm.schedulePending());
    REQUIRE(LPC_TMR16B0->MR1 == PwmController::matchValue(77, false));
}

TEST_CASE("PWM on all 16 IOs of the module", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    int hardware = 0;
    for (int io = 0; io < 16; io++)
    {
        const int ch = pwm.attach(moduleIos[io], false);
        REQUIRE(ch == io);
        pwm.setValue(ch, static_cast<uint8_t>(10 + 15 * io));
        hardware += pwm.channelHardware(ch).timer != TimerId::none;
    }
    REQUIRE(hardware == 6); // IO3, IO9, IO12, IO13, IO15, IO16
    pwm.update();
    startPeriod(pwm);
    REQUIRE(pwm.activeSchedule().edgeCount == 10);
    for (int i = 1; i < 10; i++)
    {
        REQUIRE(pwm.activeSchedule().edges[i - 1].time < pwm.activeSchedule().edges[i].time);
    }
    REQUIRE(LPC_TMR32B1->MR3 == PwmController::matchValue(10 + 15 * 12, false)); // IO13 on CT32B1 next to software
    REQUIRE(LPC_TMR32B1->PWMC == (1 << 3));
}

TEST_CASE("PWM on all 36 pins of the application", "[PwmController]")
{
    resetRegisters();
    PwmController pwm;
    int hardware = 0;
    constexpr int pinCount = sizeof(ARMPinItem::PortPins) / sizeof(ARMPinItem::PortPins[0]);
    for (int i = 0; i < pinCount; i++)
    {
        const int ch = pwm.attach(ARMPinItem::PortPins[i], (i & 1) != 0);
        REQUIRE(ch == i);
        pwm.setValue(ch, static_cast<uint8_t>(1 + 7 * i));
        hardware += pwm.channelHardware(ch).timer != TimerId::none;
    }
    // PIO0_8, PIO0_9, PIO0_11, PIO2_5, PIO2_7, PIO3_2, PIO1_4; PIO2_8, PIO3_0, PIO3_1 and PIO1_6 share a register
    REQUIRE(hardware == 7);
    REQUIRE(pwm.channelCount() == pinCount);
    pwm.update();
    startPeriod(pwm);
    const auto& s = pwm.activeSchedule();
    REQUIRE(s.edgeCount == pinCount - hardware);
    int pins = 0;
    for (const uint16_t m : s.startMask)
    {
        pins += __builtin_popcount(m);
    }
    REQUIRE(pins == pinCount - hardware);

    // all edges of one period, each in its own interrupt
    for (int i = 0; i < s.edgeCount; i++)
    {
        REQUIRE(LPC_TMR32B1->MR1 == s.edges[i].time);
        interrupt(pwm, 1, s.edges[i].time);
    }
    REQUIRE(LPC_TMR32B1->MR1 == noEdge);
}

TEST_CASE("PWM inverted as ETS writes it", "[PWMPin]")
{
    PWMPinConfig c {};
    REQUIRE_FALSE(PWMPin::inverted(c));
    reinterpret_cast<uint8_t&>(c.Invert) = 0x80; // "Ausgang invertieren"
    REQUIRE(PWMPin::inverted(c));
}
