/*
 *  lock.cpp - Test cases for the locking special function
 *
 *  Copyright (c) 2014 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "protocol.h"
#include "out8.h"
#include "app_out8.h"
#include "catch.hpp"
#include "sblib/timer.h"
// >>> TC:out8_lock
// Date: 2015-01-13 18:33:13.399728

/* Code for test case out8_lock */
static void out8_lock_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2015-01-13 18:33:13.399748
    // Assoc Table (0x177):
    //    1 (  1/0/1) <->  0 (output 1            ) @ 0x178
    //    2 (  1/0/2) <->  1 (output 2            ) @ 0x17A
    //    3 (  1/0/3) <->  2 (output 3            ) @ 0x17C
    //    4 (  1/0/4) <->  3 (output 4            ) @ 0x17E
    //    5 (  1/0/5) <->  4 (output 5            ) @ 0x180
    //    6 (  1/0/6) <->  5 (output 6            ) @ 0x182
    //    7 (  1/0/7) <->  6 (output 7            ) @ 0x184
    //    8 (  1/0/8) <->  7 (output 8            ) @ 0x186
    //    9 ( 1/0/10) <->  8 (special 1           ) @ 0x188
    //   10 ( 1/0/11) <->  9 (special 2           ) @ 0x18A
    //   11 ( 1/0/12) <-> 10 (special 3           ) @ 0x18C
    //   12 ( 1/0/13) <-> 11 (special 4           ) @ 0x18E
    //   13 ( 1/0/21) <-> 12 (feedback 1          ) @ 0x190
    //   14 ( 1/0/22) <-> 13 (feedback 2          ) @ 0x192
    //   15 ( 1/0/23) <-> 14 (feedback 3          ) @ 0x194
    //   16 ( 1/0/24) <-> 15 (feedback 4          ) @ 0x196
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)   1/0/1 @ 0x119
    //   ( 2)   1/0/2 @ 0x11B
    //   ( 3)   1/0/3 @ 0x11D
    //   ( 4)   1/0/4 @ 0x11F
    //   ( 5)   1/0/5 @ 0x121
    //   ( 6)   1/0/6 @ 0x123
    //   ( 7)   1/0/7 @ 0x125
    //   ( 8)   1/0/8 @ 0x127
    //   ( 9)  1/0/10 @ 0x129
    //   (10)  1/0/11 @ 0x12B
    //   (11)  1/0/12 @ 0x12D
    //   (12)  1/0/13 @ 0x12F
    //   (13)  1/0/21 @ 0x131
    //   (14)  1/0/22 @ 0x133
    //   (15)  1/0/23 @ 0x135
    //   (16)  1/0/24 @ 0x137
    // Com Object table (0x139):
    //   ( 0) output 1             <6B, 1F, 07> @ 0x13B (1:<undefined>)
    //   ( 1) output 2             <6C, 1F, 07> @ 0x13E (1:<undefined>)
    //   ( 2) output 3             <6D, 1F, 07> @ 0x141 (1:<undefined>)
    //   ( 3) output 4             <6E, 1F, 07> @ 0x144 (1:<undefined>)
    //   ( 4) output 5             <6F, 1F, 07> @ 0x147 (1:<undefined>)
    //   ( 5) output 6             <70, 1F, 07> @ 0x14A (1:<undefined>)
    //   ( 6) output 7             <71, 1F, 07> @ 0x14D (1:<undefined>)
    //   ( 7) output 8             <72, 1F, 07> @ 0x150 (1:<undefined>)
    //   ( 8) special 1            <73, 1F, 07> @ 0x153 (1:<undefined>)
    //   ( 9) special 2            <74, 1F, 07> @ 0x156 (1:<undefined>)
    //   (10) special 3            <75, 1F, 07> @ 0x159 (1:<undefined>)
    //   (11) special 4            <76, 1F, 07> @ 0x15C (1:<undefined>)
    //   (12) feedback 1           <77, 57, 07> @ 0x15F (1:<undefined>)
    //   (13) feedback 2           <78, 57, 07> @ 0x162 (1:<undefined>)
    //   (14) feedback 3           <79, 57, 07> @ 0x165 (1:<undefined>)
    //   (15) feedback 4           <7A, 57, 07> @ 0x168 (1:<undefined>)
    //   (16) feedback 5           <7B, 57, 07> @ 0x16B (1:<undefined>)
    //   (17) feedback 6           <7C, 57, 07> @ 0x16E (1:<undefined>)
    //   (18) feedback 7           <7D, 57, 07> @ 0x171 (1:<undefined>)
    //   (19) feedback 8           <7E, 57, 07> @ 0x174 (1:<undefined>)
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x77;
    userEeprom[0x112] = 0x39;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x10;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x01;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x02;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x03;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x04;
    userEeprom[0x121] = 0x08;
    userEeprom[0x122] = 0x05;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x06;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x07;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x08;
    userEeprom[0x129] = 0x08;
    userEeprom[0x12A] = 0x0A;
    userEeprom[0x12B] = 0x08;
    userEeprom[0x12C] = 0x0B;
    userEeprom[0x12D] = 0x08;
    userEeprom[0x12E] = 0x0C;
    userEeprom[0x12F] = 0x08;
    userEeprom[0x130] = 0x0D;
    userEeprom[0x131] = 0x08;
    userEeprom[0x132] = 0x15;
    userEeprom[0x133] = 0x08;
    userEeprom[0x134] = 0x16;
    userEeprom[0x135] = 0x08;
    userEeprom[0x136] = 0x17;
    userEeprom[0x137] = 0x08;
    userEeprom[0x138] = 0x18;
    userEeprom[0x139] = 0x14;
    userEeprom[0x13A] = 0x61;
    userEeprom[0x13B] = 0x6B;
    userEeprom[0x13C] = 0x1F;
    userEeprom[0x13D] = 0x07;
    userEeprom[0x13E] = 0x6C;
    userEeprom[0x13F] = 0x1F;
    userEeprom[0x140] = 0x07;
    userEeprom[0x141] = 0x6D;
    userEeprom[0x142] = 0x1F;
    userEeprom[0x143] = 0x07;
    userEeprom[0x144] = 0x6E;
    userEeprom[0x145] = 0x1F;
    userEeprom[0x146] = 0x07;
    userEeprom[0x147] = 0x6F;
    userEeprom[0x148] = 0x1F;
    userEeprom[0x149] = 0x07;
    userEeprom[0x14A] = 0x70;
    userEeprom[0x14B] = 0x1F;
    userEeprom[0x14C] = 0x07;
    userEeprom[0x14D] = 0x71;
    userEeprom[0x14E] = 0x1F;
    userEeprom[0x14F] = 0x07;
    userEeprom[0x150] = 0x72;
    userEeprom[0x151] = 0x1F;
    userEeprom[0x152] = 0x07;
    userEeprom[0x153] = 0x73;
    userEeprom[0x154] = 0x1F;
    userEeprom[0x155] = 0x07;
    userEeprom[0x156] = 0x74;
    userEeprom[0x157] = 0x1F;
    userEeprom[0x158] = 0x07;
    userEeprom[0x159] = 0x75;
    userEeprom[0x15A] = 0x1F;
    userEeprom[0x15B] = 0x07;
    userEeprom[0x15C] = 0x76;
    userEeprom[0x15D] = 0x1F;
    userEeprom[0x15E] = 0x07;
    userEeprom[0x15F] = 0x77;
    userEeprom[0x160] = 0x57;
    userEeprom[0x161] = 0x07;
    userEeprom[0x162] = 0x78;
    userEeprom[0x163] = 0x57;
    userEeprom[0x164] = 0x07;
    userEeprom[0x165] = 0x79;
    userEeprom[0x166] = 0x57;
    userEeprom[0x167] = 0x07;
    userEeprom[0x168] = 0x7A;
    userEeprom[0x169] = 0x57;
    userEeprom[0x16A] = 0x07;
    userEeprom[0x16B] = 0x7B;
    userEeprom[0x16C] = 0x57;
    userEeprom[0x16D] = 0x07;
    userEeprom[0x16E] = 0x7C;
    userEeprom[0x16F] = 0x57;
    userEeprom[0x170] = 0x07;
    userEeprom[0x171] = 0x7D;
    userEeprom[0x172] = 0x57;
    userEeprom[0x173] = 0x07;
    userEeprom[0x174] = 0x7E;
    userEeprom[0x175] = 0x57;
    userEeprom[0x176] = 0x07;
    userEeprom[0x177] = 0x10;
    userEeprom[0x178] = 0x01;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x02;
    userEeprom[0x17B] = 0x01;
    userEeprom[0x17C] = 0x03;
    userEeprom[0x17D] = 0x02;
    userEeprom[0x17E] = 0x04;
    userEeprom[0x17F] = 0x03;
    userEeprom[0x180] = 0x05;
    userEeprom[0x181] = 0x04;
    userEeprom[0x182] = 0x06;
    userEeprom[0x183] = 0x05;
    userEeprom[0x184] = 0x07;
    userEeprom[0x185] = 0x06;
    userEeprom[0x186] = 0x08;
    userEeprom[0x187] = 0x07;
    userEeprom[0x188] = 0x09;
    userEeprom[0x189] = 0x08;
    userEeprom[0x18A] = 0x0A;
    userEeprom[0x18B] = 0x09;
    userEeprom[0x18C] = 0x0B;
    userEeprom[0x18D] = 0x0A;
    userEeprom[0x18E] = 0x0C;
    userEeprom[0x18F] = 0x0B;
    userEeprom[0x190] = 0x0D;
    userEeprom[0x191] = 0x0C;
    userEeprom[0x192] = 0x0E;
    userEeprom[0x193] = 0x0D;
    userEeprom[0x194] = 0x0F;
    userEeprom[0x195] = 0x0E;
    userEeprom[0x196] = 0x10;
    userEeprom[0x197] = 0x0F;
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
    userEeprom[0x1D8] = 0x21;
    userEeprom[0x1D9] = 0x43;
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
    userEeprom[0x1ED] = 0x55;
    userEeprom[0x1EE] = 0x00;
    userEeprom[0x1EF] = 0x90;
    userEeprom[0x1F0] = 0x66;
    userEeprom[0x1F1] = 0x0C;
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

static Telegram tel_out8_lock[] =
{
// Collect the feedback telegrams from the powerup
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   2 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x00}}
/*   3 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x00}}
/*   4 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x00}}
/*   5 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x18, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*   6 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}

// Test locking of channel 1/lock=1, no action, no action
/*   8 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Set         , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
          // Activate the lock -> no immediate action
/*  13 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x01}}
          // Output is locked -> no reation
/*  14 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x00}}
          // Release the lock -> no immediate action
/*  15 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x00}}

// Test locking of channel 2/lock=1, clear, set
/*  16 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Set         , {0xBC, 0x11, 0x14, 0x08, 0x02, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  18 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
          // Activate the lock -> close
/*  21 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Output is locked -> no reation
/*  25 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x02, 0xE2, 0x00, 0x80, 0x01}}
          // Release the lock -> open
/*  26 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Set         , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  28 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}

// Test locking of channel 3/lock=0, set, clear
          // set the lock object to `not locked`
/*  31 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x01}}
/*  32 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Set         , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  36 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  37 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Clear       , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  39 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
// Start the locking test
          // Activate the lock -> set
/*  41 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Set         , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  42 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  43 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  45 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
          // Output is locked -> no reation
/*  46 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x01}}
          // Release the lock -> clear
/*  47 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x01}}
, {END}
};
static Test_Case out8_lock_tc = 
{
  "OUT8 - Locking"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, out8_lock_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_out8_lock
};

TEST_CASE("OUT8 - Locking","[LOCK]")
{
  executeTest(& out8_lock_tc);
}
// <<< TC:out8_lock
// >>> TC:out8_logic
// Date: 2015-01-13 18:27:05.433317

/* Code for test case out8_logic */
static void out8_logic_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2015-01-13 18:27:05.433333
    // Assoc Table (0x177):
    //    1 (  1/0/1) <->  0 (output 1            ) @ 0x178
    //    2 (  1/0/2) <->  1 (output 2            ) @ 0x17A
    //    3 (  1/0/3) <->  2 (output 3            ) @ 0x17C
    //    4 (  1/0/4) <->  3 (output 4            ) @ 0x17E
    //    5 (  1/0/5) <->  4 (output 5            ) @ 0x180
    //    6 (  1/0/6) <->  5 (output 6            ) @ 0x182
    //    7 (  1/0/7) <->  6 (output 7            ) @ 0x184
    //    8 (  1/0/8) <->  7 (output 8            ) @ 0x186
    //    9 ( 1/0/10) <->  8 (special 1           ) @ 0x188
    //   10 ( 1/0/11) <->  9 (special 2           ) @ 0x18A
    //   11 ( 1/0/12) <-> 10 (special 3           ) @ 0x18C
    //   12 ( 1/0/13) <-> 11 (special 4           ) @ 0x18E
    //   13 ( 1/0/21) <-> 12 (feedback 1          ) @ 0x190
    //   14 ( 1/0/22) <-> 13 (feedback 2          ) @ 0x192
    //   15 ( 1/0/23) <-> 14 (feedback 3          ) @ 0x194
    //   16 ( 1/0/24) <-> 15 (feedback 4          ) @ 0x196
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)   1/0/1 @ 0x119
    //   ( 2)   1/0/2 @ 0x11B
    //   ( 3)   1/0/3 @ 0x11D
    //   ( 4)   1/0/4 @ 0x11F
    //   ( 5)   1/0/5 @ 0x121
    //   ( 6)   1/0/6 @ 0x123
    //   ( 7)   1/0/7 @ 0x125
    //   ( 8)   1/0/8 @ 0x127
    //   ( 9)  1/0/10 @ 0x129
    //   (10)  1/0/11 @ 0x12B
    //   (11)  1/0/12 @ 0x12D
    //   (12)  1/0/13 @ 0x12F
    //   (13)  1/0/21 @ 0x131
    //   (14)  1/0/22 @ 0x133
    //   (15)  1/0/23 @ 0x135
    //   (16)  1/0/24 @ 0x137
    // Com Object table (0x139):
    //   ( 0) output 1             <6B, 1F, 07> @ 0x13B (1:<undefined>)
    //   ( 1) output 2             <6C, 1F, 07> @ 0x13E (1:<undefined>)
    //   ( 2) output 3             <6D, 1F, 07> @ 0x141 (1:<undefined>)
    //   ( 3) output 4             <6E, 1F, 07> @ 0x144 (1:<undefined>)
    //   ( 4) output 5             <6F, 1F, 07> @ 0x147 (1:<undefined>)
    //   ( 5) output 6             <70, 1F, 07> @ 0x14A (1:<undefined>)
    //   ( 6) output 7             <71, 1F, 07> @ 0x14D (1:<undefined>)
    //   ( 7) output 8             <72, 1F, 07> @ 0x150 (1:<undefined>)
    //   ( 8) special 1            <73, 1F, 07> @ 0x153 (1:<undefined>)
    //   ( 9) special 2            <74, 1F, 07> @ 0x156 (1:<undefined>)
    //   (10) special 3            <75, 1F, 07> @ 0x159 (1:<undefined>)
    //   (11) special 4            <76, 1F, 07> @ 0x15C (1:<undefined>)
    //   (12) feedback 1           <77, 57, 07> @ 0x15F (1:<undefined>)
    //   (13) feedback 2           <78, 57, 07> @ 0x162 (1:<undefined>)
    //   (14) feedback 3           <79, 57, 07> @ 0x165 (1:<undefined>)
    //   (15) feedback 4           <7A, 57, 07> @ 0x168 (1:<undefined>)
    //   (16) feedback 5           <7B, 57, 07> @ 0x16B (1:<undefined>)
    //   (17) feedback 6           <7C, 57, 07> @ 0x16E (1:<undefined>)
    //   (18) feedback 7           <7D, 57, 07> @ 0x171 (1:<undefined>)
    //   (19) feedback 8           <7E, 57, 07> @ 0x174 (1:<undefined>)
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x60;
    userEeprom[0x106] = 0x20;
    userEeprom[0x107] = 0x01;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x77;
    userEeprom[0x112] = 0x39;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x10;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x01;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x02;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x03;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x04;
    userEeprom[0x121] = 0x08;
    userEeprom[0x122] = 0x05;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x06;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x07;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x08;
    userEeprom[0x129] = 0x08;
    userEeprom[0x12A] = 0x0A;
    userEeprom[0x12B] = 0x08;
    userEeprom[0x12C] = 0x0B;
    userEeprom[0x12D] = 0x08;
    userEeprom[0x12E] = 0x0C;
    userEeprom[0x12F] = 0x08;
    userEeprom[0x130] = 0x0D;
    userEeprom[0x131] = 0x08;
    userEeprom[0x132] = 0x15;
    userEeprom[0x133] = 0x08;
    userEeprom[0x134] = 0x16;
    userEeprom[0x135] = 0x08;
    userEeprom[0x136] = 0x17;
    userEeprom[0x137] = 0x08;
    userEeprom[0x138] = 0x18;
    userEeprom[0x139] = 0x14;
    userEeprom[0x13A] = 0x61;
    userEeprom[0x13B] = 0x6B;
    userEeprom[0x13C] = 0x1F;
    userEeprom[0x13D] = 0x07;
    userEeprom[0x13E] = 0x6C;
    userEeprom[0x13F] = 0x1F;
    userEeprom[0x140] = 0x07;
    userEeprom[0x141] = 0x6D;
    userEeprom[0x142] = 0x1F;
    userEeprom[0x143] = 0x07;
    userEeprom[0x144] = 0x6E;
    userEeprom[0x145] = 0x1F;
    userEeprom[0x146] = 0x07;
    userEeprom[0x147] = 0x6F;
    userEeprom[0x148] = 0x1F;
    userEeprom[0x149] = 0x07;
    userEeprom[0x14A] = 0x70;
    userEeprom[0x14B] = 0x1F;
    userEeprom[0x14C] = 0x07;
    userEeprom[0x14D] = 0x71;
    userEeprom[0x14E] = 0x1F;
    userEeprom[0x14F] = 0x07;
    userEeprom[0x150] = 0x72;
    userEeprom[0x151] = 0x1F;
    userEeprom[0x152] = 0x07;
    userEeprom[0x153] = 0x73;
    userEeprom[0x154] = 0x1F;
    userEeprom[0x155] = 0x07;
    userEeprom[0x156] = 0x74;
    userEeprom[0x157] = 0x1F;
    userEeprom[0x158] = 0x07;
    userEeprom[0x159] = 0x75;
    userEeprom[0x15A] = 0x1F;
    userEeprom[0x15B] = 0x07;
    userEeprom[0x15C] = 0x76;
    userEeprom[0x15D] = 0x1F;
    userEeprom[0x15E] = 0x07;
    userEeprom[0x15F] = 0x77;
    userEeprom[0x160] = 0x57;
    userEeprom[0x161] = 0x07;
    userEeprom[0x162] = 0x78;
    userEeprom[0x163] = 0x57;
    userEeprom[0x164] = 0x07;
    userEeprom[0x165] = 0x79;
    userEeprom[0x166] = 0x57;
    userEeprom[0x167] = 0x07;
    userEeprom[0x168] = 0x7A;
    userEeprom[0x169] = 0x57;
    userEeprom[0x16A] = 0x07;
    userEeprom[0x16B] = 0x7B;
    userEeprom[0x16C] = 0x57;
    userEeprom[0x16D] = 0x07;
    userEeprom[0x16E] = 0x7C;
    userEeprom[0x16F] = 0x57;
    userEeprom[0x170] = 0x07;
    userEeprom[0x171] = 0x7D;
    userEeprom[0x172] = 0x57;
    userEeprom[0x173] = 0x07;
    userEeprom[0x174] = 0x7E;
    userEeprom[0x175] = 0x57;
    userEeprom[0x176] = 0x07;
    userEeprom[0x177] = 0x10;
    userEeprom[0x178] = 0x01;
    userEeprom[0x179] = 0x00;
    userEeprom[0x17A] = 0x02;
    userEeprom[0x17B] = 0x01;
    userEeprom[0x17C] = 0x03;
    userEeprom[0x17D] = 0x02;
    userEeprom[0x17E] = 0x04;
    userEeprom[0x17F] = 0x03;
    userEeprom[0x180] = 0x05;
    userEeprom[0x181] = 0x04;
    userEeprom[0x182] = 0x06;
    userEeprom[0x183] = 0x05;
    userEeprom[0x184] = 0x07;
    userEeprom[0x185] = 0x06;
    userEeprom[0x186] = 0x08;
    userEeprom[0x187] = 0x07;
    userEeprom[0x188] = 0x09;
    userEeprom[0x189] = 0x08;
    userEeprom[0x18A] = 0x0A;
    userEeprom[0x18B] = 0x09;
    userEeprom[0x18C] = 0x0B;
    userEeprom[0x18D] = 0x0A;
    userEeprom[0x18E] = 0x0C;
    userEeprom[0x18F] = 0x0B;
    userEeprom[0x190] = 0x0D;
    userEeprom[0x191] = 0x0C;
    userEeprom[0x192] = 0x0E;
    userEeprom[0x193] = 0x0D;
    userEeprom[0x194] = 0x0F;
    userEeprom[0x195] = 0x0E;
    userEeprom[0x196] = 0x10;
    userEeprom[0x197] = 0x0F;
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
    userEeprom[0x1D8] = 0x21;
    userEeprom[0x1D9] = 0x43;
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
    userEeprom[0x1ED] = 0x80;
    userEeprom[0x1EE] = 0x39;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x04;
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

static Telegram tel_out8_logic[] =
{
// Collect the feedback telegrams from the powerup
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   2 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x00}}
/*   3 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x00}}
/*   4 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x00}}
/*   5 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x18, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*   6 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}

// Test the OR logic for output 1
/*   8 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x00}}
/*   9 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Set         , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  11 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  14 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Clear       , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  15 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  18 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Set         , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  22 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  23 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  25 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Set         , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  29 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  32 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x01}}
/*  33 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x01, 0xE2, 0x00, 0x80, 0x00}}
/*  34 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output1Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0A, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  36 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x15, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Test the AND logic for output 2
/*  38 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x00}}
/*  39 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x02, 0xE2, 0x00, 0x80, 0x01}}
/*  40 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Set         , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  41 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  42 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  43 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  44 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  45 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  46 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  47 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  48 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Try setting the special com object again
/*  49 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Set         , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  50 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  51 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  52 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  53 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  54 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output2Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0B, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  55 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  56 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x16, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  57 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  58 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x02, 0xE2, 0x00, 0x80, 0x00}}
/*  59 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  60 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Test the AND with recirculation logic for output 3
/*  61 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x00}}
/*  62 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x01}}
/*  63 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Set         , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  64 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  65 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  66 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  67 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
/*  68 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  69 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  70 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  71 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Try setting the special com object again
/*  72 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x01}}
/*  73 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Set         , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x01}}
          // give lib one cycle for creating the feedback
/*  74 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  75 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x01}}
          // no further feedback telegram
/*  76 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  77 */ , {TIMER_TICK     ,   50,  0, (StepFunction *) _enablePWM          , {}}
          // Clear special 3
/*  78 */ , {TEL_RX         ,    9,  0, (StepFunction *) _output3Clear       , {0xBC, 0x11, 0x14, 0x08, 0x0C, 0xE2, 0x00, 0x80, 0x00}}
          // give lib one cycle for creating the feedback
/*  79 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  80 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x17, 0xE2, 0x00, 0x80, 0x00}}
          // no further feedback telegram
/*  81 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  82 */ , {TEL_RX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x03, 0xE2, 0x00, 0x80, 0x00}}
/*  83 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  84 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case out8_logic_tc = 
{
  "OUT8 - Logic"
, 0x0004, 0x2060, 01
, 0 // power-on delay
, out8_logic_eepromSetup
, NULL
, (StateFunction *) _gatherState
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_out8_logic
};

TEST_CASE("OUT8 - Logic","[LOGIC]")
{
  executeTest(& out8_logic_tc);
}
// <<< TC:out8_logic
