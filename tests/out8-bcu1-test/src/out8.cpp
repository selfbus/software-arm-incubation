/*
 *  out8.cpp - Basic tests for the simple case
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "app_out8.h"
#include "catch.hpp"
#include "sblib/timer.h"

typedef struct
{
    unsigned int outputs;
    unsigned int pwm_on;
} Out8TestState;

static Out8TestState _refState;
static Out8TestState _stepState;

static void _gatherState (Out8TestState * state, Out8TestState * refState)
{
    int i;
    state->outputs = 0;
    state->pwm_on  = timer16_0.match(PWM0) != 0;
    for (i=0; i < 8; i++)
    {
        state->outputs |= (digitalRead(outputPins[i]) << i);
    }
    if (refState)
    {
        REQUIRE(state->outputs == refState->outputs);
        REQUIRE(state->pwm_on  == refState->pwm_on);
    }
}

static void _loop(Out8TestState * refState)
{
    bcu.loop();
    loop();
}

static void _output12Set(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs = 0x03;
}

static void _enablePWM(Out8TestState * refState)
{
    _loop(refState);
    refState->pwm_on = 0x01;
}

static void _output2Clear(Out8TestState * refState)
{
    _loop(refState);
    refState->outputs = 0x01;
}

static Telegram testCaseTelegrams[] =
{ {TIMER_TICK,      1,  0, (StepFunction *)_loop             , {}}  // 1
  // check after the init of the application that the feedback object will be sent
, {TEL_TX,          8,  0, NULL                              , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x80}} // 2
, {TIMER_TICK,      1,  0, (StepFunction *)_loop             , {}} // 3
  // only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER, 0,  0, NULL                              , {}} // 4

// receive a "ON" telegram for output 1 and output 2
, {TEL_RX         , 8,  0, (StepFunction *) NULL             , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}} // 5
// process the received telegram inside the app
// since there is a switch from OFF to ON, the PWM timer should be started
, {TIMER_TICK,      1,  0, (StepFunction *)_output12Set      , {}} // 6
// another app loop is required for the the feedback telegram creation
, {TIMER_TICK,      1,  0, (StepFunction *)_loop             , {}} // 7
// as a reaction to the change a feedback telegram should be sent
, {TEL_TX,          8,  0, NULL                              , {0xBC, 0x11, 0x01, 0x08, 0x14, 0xE1, 0x00, 0x81}} // 8
// only for one feedback channel a group address is store -> only one telegram should be sent
, {CHECK_TX_BUFFER, 0,  0, NULL                              , {}} // 9
// check that the PWM for the outputs is disabled after the PWM timeout has expired
, {TIMER_TICK,     10,  0, (StepFunction *)_enablePWM        , {}} // 10

// receive the OFF telegram for channel 2
, {TEL_RX         , 8,  0, (StepFunction *) NULL             , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
// process the received telegram inside the app
, {TIMER_TICK,      1,  0, (StepFunction *)_output2Clear     , {}}
, {END}
};

static void tc_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-02-08 13:08:45.953555
    // Assoc Table (0x15D):
    //    1 ( 1/0/20) <-> 12 (feedback 1          ) @ 0x15E
    //    2 ( 1/0/30) <->  0 (output 1            ) @ 0x160
    //    2 ( 1/0/30) <->  1 (output 2            ) @ 0x162
    //    3 ( 1/0/31) <->  1 (output 2            ) @ 0x164
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/20 @ 0x119
    //   ( 2)  1/0/30 @ 0x11B
    //   ( 3)  1/0/31 @ 0x11D
    // Com Object table (0x11F):
    //   ( 0) output 1             <6B, 57, 00> @ 0x121
    //   ( 1) output 2             <6C, 57, 00> @ 0x124
    //   ( 2) output 3             <6D, 57, 00> @ 0x127
    //   ( 3) output 4             <6E, 57, 00> @ 0x12A
    //   ( 4) output 5             <6F, 57, 00> @ 0x12D
    //   ( 5) output 6             <70, 57, 00> @ 0x130
    //   ( 6) output 7             <71, 57, 00> @ 0x133
    //   ( 7) output 8             <72, 57, 00> @ 0x136
    //   ( 8) special 1            <73, 5F, 00> @ 0x139
    //   ( 9) special 2            <74, 5F, 00> @ 0x13C
    //   (10) special 3            <75, 5F, 00> @ 0x13F
    //   (11) special 4            <76, 5F, 00> @ 0x142
    //   (12) feedback 1           <77, 4F, 00> @ 0x145
    //   (13) feedback 2           <78, 4F, 00> @ 0x148
    //   (14) feedback 3           <79, 4F, 00> @ 0x14B
    //   (15) feedback 4           <7A, 4F, 00> @ 0x14E
    //   (16) feedback 5           <7B, 4F, 00> @ 0x151
    //   (17) feedback 6           <7C, 4F, 00> @ 0x154
    //   (18) feedback 7           <7D, 4F, 00> @ 0x157
    //   (19) feedback 8           <7E, 4F, 00> @ 0x15A
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x00;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x00;
    userEeprom[0x106] = 0x00;
    userEeprom[0x107] = 0x00;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x5D;
    userEeprom[0x112] = 0x1F;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x03;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x14;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x1E;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x1F;
    userEeprom[0x11F] = 0x14;
    userEeprom[0x120] = 0x61;
    userEeprom[0x121] = 0x6B;
    userEeprom[0x122] = 0x57;
    userEeprom[0x123] = 0x00;
    userEeprom[0x124] = 0x6C;
    userEeprom[0x125] = 0x57;
    userEeprom[0x126] = 0x00;
    userEeprom[0x127] = 0x6D;
    userEeprom[0x128] = 0x57;
    userEeprom[0x129] = 0x00;
    userEeprom[0x12A] = 0x6E;
    userEeprom[0x12B] = 0x57;
    userEeprom[0x12C] = 0x00;
    userEeprom[0x12D] = 0x6F;
    userEeprom[0x12E] = 0x57;
    userEeprom[0x12F] = 0x00;
    userEeprom[0x130] = 0x70;
    userEeprom[0x131] = 0x57;
    userEeprom[0x132] = 0x00;
    userEeprom[0x133] = 0x71;
    userEeprom[0x134] = 0x57;
    userEeprom[0x135] = 0x00;
    userEeprom[0x136] = 0x72;
    userEeprom[0x137] = 0x57;
    userEeprom[0x138] = 0x00;
    userEeprom[0x139] = 0x73;
    userEeprom[0x13A] = 0x5F;
    userEeprom[0x13B] = 0x00;
    userEeprom[0x13C] = 0x74;
    userEeprom[0x13D] = 0x5F;
    userEeprom[0x13E] = 0x00;
    userEeprom[0x13F] = 0x75;
    userEeprom[0x140] = 0x5F;
    userEeprom[0x141] = 0x00;
    userEeprom[0x142] = 0x76;
    userEeprom[0x143] = 0x5F;
    userEeprom[0x144] = 0x00;
    userEeprom[0x145] = 0x77;
    userEeprom[0x146] = 0x4F;
    userEeprom[0x147] = 0x00;
    userEeprom[0x148] = 0x78;
    userEeprom[0x149] = 0x4F;
    userEeprom[0x14A] = 0x00;
    userEeprom[0x14B] = 0x79;
    userEeprom[0x14C] = 0x4F;
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x7A;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x7B;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x7C;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x7D;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
    userEeprom[0x15A] = 0x7E;
    userEeprom[0x15B] = 0x4F;
    userEeprom[0x15C] = 0x00;
    userEeprom[0x15D] = 0x04;
    userEeprom[0x15E] = 0x01;
    userEeprom[0x15F] = 0x0C;
    userEeprom[0x160] = 0x02;
    userEeprom[0x161] = 0x00;
    userEeprom[0x162] = 0x02;
    userEeprom[0x163] = 0x01;
    userEeprom[0x164] = 0x03;
    userEeprom[0x165] = 0x01;
    userEeprom[0x166] = 0x00;
    userEeprom[0x167] = 0x00;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x00;
    userEeprom[0x16A] = 0x00;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x00;
    userEeprom[0x16D] = 0x00;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x00;
    userEeprom[0x170] = 0x00;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x00;
    userEeprom[0x173] = 0x00;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x00;
    userEeprom[0x176] = 0x00;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x00;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x00;
    userEeprom[0x17C] = 0x00;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x00;
    userEeprom[0x17F] = 0x00;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x00;
    userEeprom[0x182] = 0x00;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x00;
    userEeprom[0x185] = 0x00;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x00;
    userEeprom[0x188] = 0x00;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x00;
    userEeprom[0x18B] = 0x00;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x00;
    userEeprom[0x18E] = 0x00;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x00;
    userEeprom[0x191] = 0x00;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x00;
    userEeprom[0x194] = 0x00;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x00;
    userEeprom[0x197] = 0x00;
    userEeprom[0x198] = 0x00;
    userEeprom[0x199] = 0x00;
    userEeprom[0x19A] = 0x00;
    userEeprom[0x19B] = 0x00;
    userEeprom[0x19C] = 0x00;
    userEeprom[0x19D] = 0x00;
    userEeprom[0x19E] = 0x00;
    userEeprom[0x19F] = 0x00;
    userEeprom[0x1A0] = 0x00;
    userEeprom[0x1A1] = 0x00;
    userEeprom[0x1A2] = 0x00;
    userEeprom[0x1A3] = 0x00;
    userEeprom[0x1A4] = 0x00;
    userEeprom[0x1A5] = 0x00;
    userEeprom[0x1A6] = 0x00;
    userEeprom[0x1A7] = 0x00;
    userEeprom[0x1A8] = 0x00;
    userEeprom[0x1A9] = 0x00;
    userEeprom[0x1AA] = 0x00;
    userEeprom[0x1AB] = 0x00;
    userEeprom[0x1AC] = 0x00;
    userEeprom[0x1AD] = 0x00;
    userEeprom[0x1AE] = 0x00;
    userEeprom[0x1AF] = 0x00;
    userEeprom[0x1B0] = 0x00;
    userEeprom[0x1B1] = 0x00;
    userEeprom[0x1B2] = 0x00;
    userEeprom[0x1B3] = 0x00;
    userEeprom[0x1B4] = 0x00;
    userEeprom[0x1B5] = 0x00;
    userEeprom[0x1B6] = 0x00;
    userEeprom[0x1B7] = 0x00;
    userEeprom[0x1B8] = 0x00;
    userEeprom[0x1B9] = 0x00;
    userEeprom[0x1BA] = 0x00;
    userEeprom[0x1BB] = 0x00;
    userEeprom[0x1BC] = 0x00;
    userEeprom[0x1BD] = 0x00;
    userEeprom[0x1BE] = 0x00;
    userEeprom[0x1BF] = 0x00;
    userEeprom[0x1C0] = 0x00;
    userEeprom[0x1C1] = 0x00;
    userEeprom[0x1C2] = 0x00;
    userEeprom[0x1C3] = 0x00;
    userEeprom[0x1C4] = 0x00;
    userEeprom[0x1C5] = 0x00;
    userEeprom[0x1C6] = 0x00;
    userEeprom[0x1C7] = 0x00;
    userEeprom[0x1C8] = 0x00;
    userEeprom[0x1C9] = 0x00;
    userEeprom[0x1CA] = 0x00;
    userEeprom[0x1CB] = 0x00;
    userEeprom[0x1CC] = 0x00;
    userEeprom[0x1CD] = 0x00;
    userEeprom[0x1CE] = 0x00;
    userEeprom[0x1CF] = 0x00;
    userEeprom[0x1D0] = 0x00;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x00;
    userEeprom[0x1D3] = 0x00;
    userEeprom[0x1D4] = 0x00;
    userEeprom[0x1D5] = 0x00;
    userEeprom[0x1D6] = 0x00;
    userEeprom[0x1D7] = 0x00;
    userEeprom[0x1D8] = 0x00;
    userEeprom[0x1D9] = 0x00;
    userEeprom[0x1DA] = 0x00;
    userEeprom[0x1DB] = 0x00;
    userEeprom[0x1DC] = 0x00;
    userEeprom[0x1DD] = 0x00;
    userEeprom[0x1DE] = 0x00;
    userEeprom[0x1DF] = 0x00;
    userEeprom[0x1E0] = 0x00;
    userEeprom[0x1E1] = 0x00;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x00;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x00;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x00;
    userEeprom[0x1F9] = 0x00;
    userEeprom[0x1FA] = 0x00;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x00;
    userEeprom[0x1FE] = 0x00;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static void tc_setup(void)
{
    initApplication();
}

static Test_Case testCase =
{
  "OUT8 - Test 1"
, 0x0004, 0x2060, 0x01
, tc_eepromSetup
, tc_setup
, (StateFunction *)  _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, testCaseTelegrams
};

TEST_CASE("Simple out8 testing","[APP][OUT8]")
{
    executeTest(& testCase);
}



