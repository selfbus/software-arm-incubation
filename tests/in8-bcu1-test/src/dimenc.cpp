/*
 *  dimenc.cpp - Test the 8in dim encoder
 *
 *  Copyright (c) 2014 Martin Glück <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "protocol.h"
#include "in8.h"
#include "app_in8.h"
#include "catch.hpp"
#include "sblib/timer.h"
// >>> TC:input_as_dim_encoder_locking
// Date: 2014-07-24 08:26:40.717702

/* Code for test case input_as_dim_encoder_locking */
static void input_as_dim_encoder_locking_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-24 08:26:40.717702
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dimen        1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dimen        2      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dimen        3      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dimen        4      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dimen        5      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (switch state 6.1    ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (switch state 7.1    ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (switch state 8.1    ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (switch state 1.2    ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (switch state 2.2    ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (switch state 3.2    ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (switch state 4.2    ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (switch state 5.2    ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (switch state 6.2    ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (switch state 7.2    ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (switch state 8.2    ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dimenc lock  1      ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dimenc lock  2      ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dimenc lock  3      ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dimenc lock  4      ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dimenc lock  5      ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (switch lock 6       ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (switch lock 7       ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (switch lock 8       ) @ 0x1C2
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/30 @ 0x119
    //   ( 2)  1/0/31 @ 0x11B
    //   ( 3)  1/0/32 @ 0x11D
    //   ( 4)  1/0/33 @ 0x11F
    //   ( 5)  1/0/34 @ 0x121
    //   ( 6)  1/0/35 @ 0x123
    //   ( 7)  1/0/36 @ 0x125
    //   ( 8)  1/0/37 @ 0x127
    //   ( 9)  1/0/38 @ 0x129
    //   (10)  1/0/39 @ 0x12B
    //   (11)  1/0/40 @ 0x12D
    //   (12)  1/0/41 @ 0x12F
    //   (13)  1/0/42 @ 0x131
    //   (14)  1/0/43 @ 0x133
    //   (15)  1/0/44 @ 0x135
    //   (16)  1/0/45 @ 0x137
    //   (17)  1/0/50 @ 0x139
    //   (18)  1/0/51 @ 0x13B
    //   (19)  1/0/52 @ 0x13D
    //   (20)  1/0/53 @ 0x13F
    //   (21)  1/0/54 @ 0x141
    //   (22)  1/0/55 @ 0x143
    //   (23)  1/0/56 @ 0x145
    //   (24)  1/0/57 @ 0x147
    // Com Object table (0x149):
    //   ( 0) dimen        1       <6D, 4F, 07> @ 0x14B (5.001:Scaling)
    //   ( 1) dimen        2       <6E, 4F, 07> @ 0x14E (5.001:Scaling)
    //   ( 2) dimen        3       <6F, 4F, 07> @ 0x151 (5.001:Scaling)
    //   ( 3) dimen        4       <70, 4F, 07> @ 0x154 (5.001:Scaling)
    //   ( 4) dimen        5       <71, 4F, 07> @ 0x157 (5.001:Scaling)
    //   ( 5) switch state 6.1     <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) switch state 7.1     <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) switch state 8.1     <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) switch state 1.2     <75, 4F, 00> @ 0x163 (1.001:Switch)
    //   ( 9) switch state 2.2     <76, 4F, 00> @ 0x166 (1.001:Switch)
    //   (10) switch state 3.2     <77, 4F, 00> @ 0x169 (1.001:Switch)
    //   (11) switch state 4.2     <78, 4F, 00> @ 0x16C (1.001:Switch)
    //   (12) switch state 5.2     <79, 4F, 00> @ 0x16F (1.001:Switch)
    //   (13) switch state 6.2     <7A, 4F, 00> @ 0x172 (1.001:Switch)
    //   (14) switch state 7.2     <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) switch state 8.2     <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dimenc lock  1       <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dimenc lock  2       <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dimenc lock  3       <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dimenc lock  4       <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dimenc lock  5       <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) switch lock 6        <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) switch lock 7        <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) switch lock 8        <84, 17, 00> @ 0x190 (1.001:Switch)
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x54;
    userEeprom[0x106] = 0x70;
    userEeprom[0x107] = 0x02;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x93;
    userEeprom[0x112] = 0x49;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x18;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x1E;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x1F;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x20;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x21;
    userEeprom[0x121] = 0x08;
    userEeprom[0x122] = 0x22;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x23;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x24;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x25;
    userEeprom[0x129] = 0x08;
    userEeprom[0x12A] = 0x26;
    userEeprom[0x12B] = 0x08;
    userEeprom[0x12C] = 0x27;
    userEeprom[0x12D] = 0x08;
    userEeprom[0x12E] = 0x28;
    userEeprom[0x12F] = 0x08;
    userEeprom[0x130] = 0x29;
    userEeprom[0x131] = 0x08;
    userEeprom[0x132] = 0x2A;
    userEeprom[0x133] = 0x08;
    userEeprom[0x134] = 0x2B;
    userEeprom[0x135] = 0x08;
    userEeprom[0x136] = 0x2C;
    userEeprom[0x137] = 0x08;
    userEeprom[0x138] = 0x2D;
    userEeprom[0x139] = 0x08;
    userEeprom[0x13A] = 0x32;
    userEeprom[0x13B] = 0x08;
    userEeprom[0x13C] = 0x33;
    userEeprom[0x13D] = 0x08;
    userEeprom[0x13E] = 0x34;
    userEeprom[0x13F] = 0x08;
    userEeprom[0x140] = 0x35;
    userEeprom[0x141] = 0x08;
    userEeprom[0x142] = 0x36;
    userEeprom[0x143] = 0x08;
    userEeprom[0x144] = 0x37;
    userEeprom[0x145] = 0x08;
    userEeprom[0x146] = 0x38;
    userEeprom[0x147] = 0x08;
    userEeprom[0x148] = 0x39;
    userEeprom[0x149] = 0x18;
    userEeprom[0x14A] = 0x61;
    userEeprom[0x14B] = 0x6D;
    userEeprom[0x14C] = 0x4F;
    userEeprom[0x14D] = 0x07;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x07;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x07;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x07;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x07;
    userEeprom[0x15A] = 0x72;
    userEeprom[0x15B] = 0x4F;
    userEeprom[0x15C] = 0x00;
    userEeprom[0x15D] = 0x73;
    userEeprom[0x15E] = 0x4F;
    userEeprom[0x15F] = 0x00;
    userEeprom[0x160] = 0x74;
    userEeprom[0x161] = 0x4F;
    userEeprom[0x162] = 0x00;
    userEeprom[0x163] = 0x75;
    userEeprom[0x164] = 0x4F;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x7B;
    userEeprom[0x176] = 0x4F;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x7C;
    userEeprom[0x179] = 0x4F;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x7D;
    userEeprom[0x17C] = 0x17;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x7E;
    userEeprom[0x17F] = 0x17;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x7F;
    userEeprom[0x182] = 0x17;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x80;
    userEeprom[0x185] = 0x17;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x81;
    userEeprom[0x188] = 0x17;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x82;
    userEeprom[0x18B] = 0x17;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x83;
    userEeprom[0x18E] = 0x17;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x84;
    userEeprom[0x191] = 0x17;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x18;
    userEeprom[0x194] = 0x01;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x02;
    userEeprom[0x197] = 0x01;
    userEeprom[0x198] = 0x03;
    userEeprom[0x199] = 0x02;
    userEeprom[0x19A] = 0x04;
    userEeprom[0x19B] = 0x03;
    userEeprom[0x19C] = 0x05;
    userEeprom[0x19D] = 0x04;
    userEeprom[0x19E] = 0x06;
    userEeprom[0x19F] = 0x05;
    userEeprom[0x1A0] = 0x07;
    userEeprom[0x1A1] = 0x06;
    userEeprom[0x1A2] = 0x08;
    userEeprom[0x1A3] = 0x07;
    userEeprom[0x1A4] = 0x09;
    userEeprom[0x1A5] = 0x08;
    userEeprom[0x1A6] = 0x0A;
    userEeprom[0x1A7] = 0x09;
    userEeprom[0x1A8] = 0x0B;
    userEeprom[0x1A9] = 0x0A;
    userEeprom[0x1AA] = 0x0C;
    userEeprom[0x1AB] = 0x0B;
    userEeprom[0x1AC] = 0x0D;
    userEeprom[0x1AD] = 0x0C;
    userEeprom[0x1AE] = 0x0E;
    userEeprom[0x1AF] = 0x0D;
    userEeprom[0x1B0] = 0x0F;
    userEeprom[0x1B1] = 0x0E;
    userEeprom[0x1B2] = 0x10;
    userEeprom[0x1B3] = 0x0F;
    userEeprom[0x1B4] = 0x11;
    userEeprom[0x1B5] = 0x10;
    userEeprom[0x1B6] = 0x12;
    userEeprom[0x1B7] = 0x11;
    userEeprom[0x1B8] = 0x13;
    userEeprom[0x1B9] = 0x12;
    userEeprom[0x1BA] = 0x14;
    userEeprom[0x1BB] = 0x13;
    userEeprom[0x1BC] = 0x15;
    userEeprom[0x1BD] = 0x14;
    userEeprom[0x1BE] = 0x16;
    userEeprom[0x1BF] = 0x15;
    userEeprom[0x1C0] = 0x17;
    userEeprom[0x1C1] = 0x16;
    userEeprom[0x1C2] = 0x18;
    userEeprom[0x1C3] = 0x17;
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
    userEeprom[0x1CE] = 0x44;
    userEeprom[0x1CF] = 0x44;
    userEeprom[0x1D0] = 0x14;
    userEeprom[0x1D1] = 0x11;
    userEeprom[0x1D2] = 0x0A;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0xA9;
    userEeprom[0x1D6] = 0x05;
    userEeprom[0x1D7] = 0x8E;
    userEeprom[0x1D8] = 0x03;
    userEeprom[0x1D9] = 0xA9;
    userEeprom[0x1DA] = 0x68;
    userEeprom[0x1DB] = 0x8D;
    userEeprom[0x1DC] = 0x03;
    userEeprom[0x1DD] = 0x02;
    userEeprom[0x1DE] = 0xFD;
    userEeprom[0x1DF] = 0x90;
    userEeprom[0x1E0] = 0x8C;
    userEeprom[0x1E1] = 0xF0;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x21;
    userEeprom[0x1E4] = 0x14;
    userEeprom[0x1E5] = 0xE0;
    userEeprom[0x1E6] = 0x40;
    userEeprom[0x1E7] = 0x0C;
    userEeprom[0x1E8] = 0x15;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x11;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x11;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x11;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x22;
    userEeprom[0x1F9] = 0x33;
    userEeprom[0x1FA] = 0x03;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x33;
    userEeprom[0x1FE] = 0x33;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dim_encoder_locking[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE2, 0x00, 0x80, 0x8E}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE2, 0x00, 0x80, 0x8D}}
          // Bus return action for channel 3
/*   4 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x8C}}

// Test locking of channel 1
          // Activate the lock
/*   5 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x81}}
/*   6 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send rising value
/*   7 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE2, 0x00, 0x80, 0x8E}}
/*   8 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Deactivate the lock
/*  10 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x80}}
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Test locking of channel 2
          // Activate the lock
/*  14 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*  15 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Deactivate the lock
/*  18 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send falling value
/*  20 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE2, 0x00, 0x80, 0x8D}}
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Test locking of channel 3
          // Activate the lock
/*  23 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send falling value
/*  25 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x90}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Deactivate the lock
/*  28 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send falling value
/*  30 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x8C}}
/*  31 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Activate the lock
/*  33 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  34 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send falling value
/*  35 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x90}}
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Rising edge on channel 3
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  41 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Deactivate the lock
/*  42 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  43 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send falling value
/*  44 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x90}}
/*  45 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  46 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_dim_encoder_locking_tc =
{
  "IN8 - Dim Encoder Locking"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dim_encoder_locking_eepromSetup
, _resetInputs
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dim_encoder_locking
};

#if 1
TEST_CASE("IN8 - Dim Encoder Locking","[APP][IN8][DIMENC][DIMENC_LOCK]")
{
  executeTest(& input_as_dim_encoder_locking_tc);
}
#endif
// <<< TC:input_as_dim_encoder_locking
// >>> TC:input_as_dim_encoder
// Date: 2014-07-24 08:27:22.940117

/* Code for test case input_as_dim_encoder */
static void input_as_dim_encoder_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-24 08:27:22.940117
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dimen        1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dimen        2      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dimen        3      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dimen        4      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dimen        5      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (switch state 6.1    ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (switch state 7.1    ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (switch state 8.1    ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (switch state 1.2    ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (switch state 2.2    ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (switch state 3.2    ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (switch state 4.2    ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (switch state 5.2    ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (switch state 6.2    ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (switch state 7.2    ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (switch state 8.2    ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dimenc lock  1      ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dimenc lock  2      ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dimenc lock  3      ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dimenc lock  4      ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dimenc lock  5      ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (switch lock 6       ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (switch lock 7       ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (switch lock 8       ) @ 0x1C2
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/30 @ 0x119
    //   ( 2)  1/0/31 @ 0x11B
    //   ( 3)  1/0/32 @ 0x11D
    //   ( 4)  1/0/33 @ 0x11F
    //   ( 5)  1/0/34 @ 0x121
    //   ( 6)  1/0/35 @ 0x123
    //   ( 7)  1/0/36 @ 0x125
    //   ( 8)  1/0/37 @ 0x127
    //   ( 9)  1/0/38 @ 0x129
    //   (10)  1/0/39 @ 0x12B
    //   (11)  1/0/40 @ 0x12D
    //   (12)  1/0/41 @ 0x12F
    //   (13)  1/0/42 @ 0x131
    //   (14)  1/0/43 @ 0x133
    //   (15)  1/0/44 @ 0x135
    //   (16)  1/0/45 @ 0x137
    //   (17)  1/0/50 @ 0x139
    //   (18)  1/0/51 @ 0x13B
    //   (19)  1/0/52 @ 0x13D
    //   (20)  1/0/53 @ 0x13F
    //   (21)  1/0/54 @ 0x141
    //   (22)  1/0/55 @ 0x143
    //   (23)  1/0/56 @ 0x145
    //   (24)  1/0/57 @ 0x147
    // Com Object table (0x149):
    //   ( 0) dimen        1       <6D, 4F, 07> @ 0x14B (5.001:Scaling)
    //   ( 1) dimen        2       <6E, 4F, 07> @ 0x14E (5.001:Scaling)
    //   ( 2) dimen        3       <6F, 4F, 07> @ 0x151 (5.001:Scaling)
    //   ( 3) dimen        4       <70, 4F, 07> @ 0x154 (5.001:Scaling)
    //   ( 4) dimen        5       <71, 4F, 07> @ 0x157 (5.001:Scaling)
    //   ( 5) switch state 6.1     <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) switch state 7.1     <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) switch state 8.1     <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) switch state 1.2     <75, 4F, 00> @ 0x163 (1.001:Switch)
    //   ( 9) switch state 2.2     <76, 4F, 00> @ 0x166 (1.001:Switch)
    //   (10) switch state 3.2     <77, 4F, 00> @ 0x169 (1.001:Switch)
    //   (11) switch state 4.2     <78, 4F, 00> @ 0x16C (1.001:Switch)
    //   (12) switch state 5.2     <79, 4F, 00> @ 0x16F (1.001:Switch)
    //   (13) switch state 6.2     <7A, 4F, 00> @ 0x172 (1.001:Switch)
    //   (14) switch state 7.2     <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) switch state 8.2     <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dimenc lock  1       <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dimenc lock  2       <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dimenc lock  3       <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dimenc lock  4       <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dimenc lock  5       <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) switch lock 6        <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) switch lock 7        <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) switch lock 8        <84, 17, 00> @ 0x190 (1.001:Switch)
    userEeprom[0x100] = 0x00;
    userEeprom[0x101] = 0x00;
    userEeprom[0x102] = 0x00;
    userEeprom[0x103] = 0x04;
    userEeprom[0x104] = 0x00;
    userEeprom[0x105] = 0x54;
    userEeprom[0x106] = 0x70;
    userEeprom[0x107] = 0x02;
    userEeprom[0x108] = 0x00;
    userEeprom[0x109] = 0x00;
    userEeprom[0x10A] = 0x00;
    userEeprom[0x10B] = 0x00;
    userEeprom[0x10C] = 0x00;
    userEeprom[0x10D] = 0x00;
    userEeprom[0x10E] = 0x00;
    userEeprom[0x10F] = 0x00;
    userEeprom[0x110] = 0x00;
    userEeprom[0x111] = 0x93;
    userEeprom[0x112] = 0x49;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x18;
    userEeprom[0x117] = 0x11;
    userEeprom[0x118] = 0x01;
    userEeprom[0x119] = 0x08;
    userEeprom[0x11A] = 0x1E;
    userEeprom[0x11B] = 0x08;
    userEeprom[0x11C] = 0x1F;
    userEeprom[0x11D] = 0x08;
    userEeprom[0x11E] = 0x20;
    userEeprom[0x11F] = 0x08;
    userEeprom[0x120] = 0x21;
    userEeprom[0x121] = 0x08;
    userEeprom[0x122] = 0x22;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x23;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x24;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x25;
    userEeprom[0x129] = 0x08;
    userEeprom[0x12A] = 0x26;
    userEeprom[0x12B] = 0x08;
    userEeprom[0x12C] = 0x27;
    userEeprom[0x12D] = 0x08;
    userEeprom[0x12E] = 0x28;
    userEeprom[0x12F] = 0x08;
    userEeprom[0x130] = 0x29;
    userEeprom[0x131] = 0x08;
    userEeprom[0x132] = 0x2A;
    userEeprom[0x133] = 0x08;
    userEeprom[0x134] = 0x2B;
    userEeprom[0x135] = 0x08;
    userEeprom[0x136] = 0x2C;
    userEeprom[0x137] = 0x08;
    userEeprom[0x138] = 0x2D;
    userEeprom[0x139] = 0x08;
    userEeprom[0x13A] = 0x32;
    userEeprom[0x13B] = 0x08;
    userEeprom[0x13C] = 0x33;
    userEeprom[0x13D] = 0x08;
    userEeprom[0x13E] = 0x34;
    userEeprom[0x13F] = 0x08;
    userEeprom[0x140] = 0x35;
    userEeprom[0x141] = 0x08;
    userEeprom[0x142] = 0x36;
    userEeprom[0x143] = 0x08;
    userEeprom[0x144] = 0x37;
    userEeprom[0x145] = 0x08;
    userEeprom[0x146] = 0x38;
    userEeprom[0x147] = 0x08;
    userEeprom[0x148] = 0x39;
    userEeprom[0x149] = 0x18;
    userEeprom[0x14A] = 0x61;
    userEeprom[0x14B] = 0x6D;
    userEeprom[0x14C] = 0x4F;
    userEeprom[0x14D] = 0x07;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x07;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x07;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x07;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x07;
    userEeprom[0x15A] = 0x72;
    userEeprom[0x15B] = 0x4F;
    userEeprom[0x15C] = 0x00;
    userEeprom[0x15D] = 0x73;
    userEeprom[0x15E] = 0x4F;
    userEeprom[0x15F] = 0x00;
    userEeprom[0x160] = 0x74;
    userEeprom[0x161] = 0x4F;
    userEeprom[0x162] = 0x00;
    userEeprom[0x163] = 0x75;
    userEeprom[0x164] = 0x4F;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x7B;
    userEeprom[0x176] = 0x4F;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x7C;
    userEeprom[0x179] = 0x4F;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x7D;
    userEeprom[0x17C] = 0x17;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x7E;
    userEeprom[0x17F] = 0x17;
    userEeprom[0x180] = 0x00;
    userEeprom[0x181] = 0x7F;
    userEeprom[0x182] = 0x17;
    userEeprom[0x183] = 0x00;
    userEeprom[0x184] = 0x80;
    userEeprom[0x185] = 0x17;
    userEeprom[0x186] = 0x00;
    userEeprom[0x187] = 0x81;
    userEeprom[0x188] = 0x17;
    userEeprom[0x189] = 0x00;
    userEeprom[0x18A] = 0x82;
    userEeprom[0x18B] = 0x17;
    userEeprom[0x18C] = 0x00;
    userEeprom[0x18D] = 0x83;
    userEeprom[0x18E] = 0x17;
    userEeprom[0x18F] = 0x00;
    userEeprom[0x190] = 0x84;
    userEeprom[0x191] = 0x17;
    userEeprom[0x192] = 0x00;
    userEeprom[0x193] = 0x18;
    userEeprom[0x194] = 0x01;
    userEeprom[0x195] = 0x00;
    userEeprom[0x196] = 0x02;
    userEeprom[0x197] = 0x01;
    userEeprom[0x198] = 0x03;
    userEeprom[0x199] = 0x02;
    userEeprom[0x19A] = 0x04;
    userEeprom[0x19B] = 0x03;
    userEeprom[0x19C] = 0x05;
    userEeprom[0x19D] = 0x04;
    userEeprom[0x19E] = 0x06;
    userEeprom[0x19F] = 0x05;
    userEeprom[0x1A0] = 0x07;
    userEeprom[0x1A1] = 0x06;
    userEeprom[0x1A2] = 0x08;
    userEeprom[0x1A3] = 0x07;
    userEeprom[0x1A4] = 0x09;
    userEeprom[0x1A5] = 0x08;
    userEeprom[0x1A6] = 0x0A;
    userEeprom[0x1A7] = 0x09;
    userEeprom[0x1A8] = 0x0B;
    userEeprom[0x1A9] = 0x0A;
    userEeprom[0x1AA] = 0x0C;
    userEeprom[0x1AB] = 0x0B;
    userEeprom[0x1AC] = 0x0D;
    userEeprom[0x1AD] = 0x0C;
    userEeprom[0x1AE] = 0x0E;
    userEeprom[0x1AF] = 0x0D;
    userEeprom[0x1B0] = 0x0F;
    userEeprom[0x1B1] = 0x0E;
    userEeprom[0x1B2] = 0x10;
    userEeprom[0x1B3] = 0x0F;
    userEeprom[0x1B4] = 0x11;
    userEeprom[0x1B5] = 0x10;
    userEeprom[0x1B6] = 0x12;
    userEeprom[0x1B7] = 0x11;
    userEeprom[0x1B8] = 0x13;
    userEeprom[0x1B9] = 0x12;
    userEeprom[0x1BA] = 0x14;
    userEeprom[0x1BB] = 0x13;
    userEeprom[0x1BC] = 0x15;
    userEeprom[0x1BD] = 0x14;
    userEeprom[0x1BE] = 0x16;
    userEeprom[0x1BF] = 0x15;
    userEeprom[0x1C0] = 0x17;
    userEeprom[0x1C1] = 0x16;
    userEeprom[0x1C2] = 0x18;
    userEeprom[0x1C3] = 0x17;
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
    userEeprom[0x1CE] = 0x44;
    userEeprom[0x1CF] = 0x44;
    userEeprom[0x1D0] = 0x14;
    userEeprom[0x1D1] = 0x11;
    userEeprom[0x1D2] = 0x0A;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0xA9;
    userEeprom[0x1D6] = 0x05;
    userEeprom[0x1D7] = 0x8E;
    userEeprom[0x1D8] = 0x03;
    userEeprom[0x1D9] = 0xA9;
    userEeprom[0x1DA] = 0x68;
    userEeprom[0x1DB] = 0x8D;
    userEeprom[0x1DC] = 0x03;
    userEeprom[0x1DD] = 0x02;
    userEeprom[0x1DE] = 0xFD;
    userEeprom[0x1DF] = 0x90;
    userEeprom[0x1E0] = 0x8C;
    userEeprom[0x1E1] = 0xF0;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x21;
    userEeprom[0x1E4] = 0x14;
    userEeprom[0x1E5] = 0xE0;
    userEeprom[0x1E6] = 0x40;
    userEeprom[0x1E7] = 0x0C;
    userEeprom[0x1E8] = 0x15;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x11;
    userEeprom[0x1EB] = 0x00;
    userEeprom[0x1EC] = 0x00;
    userEeprom[0x1ED] = 0x00;
    userEeprom[0x1EE] = 0x11;
    userEeprom[0x1EF] = 0x00;
    userEeprom[0x1F0] = 0x00;
    userEeprom[0x1F1] = 0x00;
    userEeprom[0x1F2] = 0x11;
    userEeprom[0x1F3] = 0x00;
    userEeprom[0x1F4] = 0x00;
    userEeprom[0x1F5] = 0x00;
    userEeprom[0x1F6] = 0x00;
    userEeprom[0x1F7] = 0x00;
    userEeprom[0x1F8] = 0x22;
    userEeprom[0x1F9] = 0x33;
    userEeprom[0x1FA] = 0x03;
    userEeprom[0x1FB] = 0x00;
    userEeprom[0x1FC] = 0x00;
    userEeprom[0x1FD] = 0x33;
    userEeprom[0x1FE] = 0x33;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dim_encoder[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE2, 0x00, 0x80, 0x8E}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE2, 0x00, 0x80, 0x8D}}
          // Bus return action for channel 3
/*   4 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x8C}}

// Channel 1 sends at rising edge
          // Rising edge on channel 1
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*   8 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE2, 0x00, 0x80, 0x8E}}
/*   9 */ , {TIMER_TICK     ,  100,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Long time press -> should not change the value
/*  12 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 1
/*  15 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
/*  16 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  18 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Rising edge on channel 1
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
/*  20 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  22 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE2, 0x00, 0x80, 0x8E}}
/*  23 */ , {TIMER_TICK     ,  100,  0, (StepFunction *) _loop               , {}}
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  25 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Long time press -> should not change the value
/*  26 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 1
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
/*  30 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 2 sends at falling edge
          // Rising edge on channel 2
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
/*  34 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  39 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 2
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
/*  41 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  42 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  43 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE2, 0x00, 0x80, 0x8D}}
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  45 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Rising edge on channel 2
/*  46 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
/*  47 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  48 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  49 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  50 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  51 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  52 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 2
/*  53 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
/*  54 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  55 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  56 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE2, 0x00, 0x80, 0x8D}}
/*  57 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  58 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 3 sends at both edges
          // Rising edge on channel 3
/*  59 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  60 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  61 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  62 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x90}}
/*  63 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  64 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  65 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/*  66 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/*  67 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  68 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  69 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x8C}}
/*  70 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  71 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Rising edge on channel 3
/*  72 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  73 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  74 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  75 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x90}}
/*  76 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
/*  77 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  78 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/*  79 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/*  80 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  81 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  82 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE2, 0x00, 0x80, 0x8C}}
/*  83 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  84 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 4 sends at rising edge
          // Rising edge on channel 4
/*  85 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
/*  86 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  87 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  88 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x21}}
          // Long time press -> should not change the value
/*  89 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  90 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  91 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x12}}
          // Long time press -> should change the value
/*  92 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  93 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  94 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x03}}
          // Long time press -> should change the value
/*  95 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  96 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/*  97 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x00}}
          // Long time press -> should not change the value
/*  98 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
/*  99 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // We are already ouside the limits 0-255, therefore no further telegram
/* 100 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 4
/* 101 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
/* 102 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 103 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 104 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 4 sends at rising edge
          // Rising edge on channel 4
/* 105 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
/* 106 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 107 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 108 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x00}}
          // Long time press -> should not change the value
/* 109 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 110 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 111 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x03}}
          // Long time press -> should change the value
/* 112 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 113 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 114 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x12}}
          // Long time press -> should change the value
/* 115 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 116 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 117 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x21}}
          // Long time press -> should change the value
/* 118 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 119 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 120 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x30}}
          // Long time press -> should change the value
/* 121 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 122 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 123 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x3F}}
          // Long time press -> should change the value
/* 124 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 125 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 126 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x4E}}
          // Long time press -> should change the value
/* 127 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 128 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 129 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x5D}}
          // Long time press -> should change the value
/* 130 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 131 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 132 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x6C}}
          // Long time press -> should change the value
/* 133 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 134 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 135 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x7B}}
          // Long time press -> should change the value
/* 136 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 137 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 138 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x8A}}
          // Long time press -> should change the value
/* 139 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 140 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 141 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0x99}}
          // Long time press -> should change the value
/* 142 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 143 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 144 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xA8}}
          // Long time press -> should change the value
/* 145 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 146 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 147 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xB7}}
          // Long time press -> should change the value
/* 148 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 149 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 150 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xC6}}
          // Long time press -> should change the value
/* 151 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 152 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 153 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xD5}}
          // Long time press -> should change the value
/* 154 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 155 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 156 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xE4}}
          // Long time press -> should change the value
/* 157 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 158 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 159 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xF3}}
          // Long time press -> should change the value
/* 160 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 161 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 162 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xFF}}
          // Long time press -> should not change the value
/* 163 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
/* 164 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // We are already ouside the limits 0-255, therefore no further telegram
/* 165 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 4
/* 166 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
/* 167 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 168 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 169 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 4 sends at rising edge
          // Rising edge on channel 4
/* 170 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
/* 171 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 172 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 173 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xFF}}
          // Long time press -> should not change the value
/* 174 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 175 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 176 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xF3}}
          // Long time press -> should change the value
/* 177 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 178 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 179 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xE4}}
          // Long time press -> should change the value
/* 180 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 181 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 182 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xD5}}
          // Long time press -> should change the value
/* 183 */ , {TIMER_TICK     , 10560,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 184 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 185 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE2, 0x00, 0x80, 0xC6}}
          // Falling edge on channel 4
/* 186 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
/* 187 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 188 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 189 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 5 sends at falling edge
          // Rising edge on channel 5
/* 190 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
/* 191 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 192 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 193 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 194 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Long time press -> should not change the value
/* 195 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 196 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 197 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x00}}
          // Long time press -> should not change the value
/* 198 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
/* 199 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // We are already ouside the limits 0-255, therefore no further telegram
/* 200 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 5
/* 201 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
/* 202 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 203 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 204 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x00}}
/* 205 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 206 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 5 sends at falling edge
          // Rising edge on channel 5
/* 207 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
/* 208 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 209 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 210 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 211 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Long time press -> should not change the value
/* 212 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 213 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 214 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x0C}}
          // Long time press -> should change the value
/* 215 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 216 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 217 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x1A}}
          // Long time press -> should change the value
/* 218 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 219 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 220 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x28}}
          // Long time press -> should change the value
/* 221 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 222 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 223 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x36}}
          // Long time press -> should change the value
/* 224 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 225 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 226 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x44}}
          // Long time press -> should change the value
/* 227 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 228 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 229 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x52}}
          // Long time press -> should change the value
/* 230 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 231 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 232 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x60}}
          // Long time press -> should change the value
/* 233 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 234 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 235 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x6E}}
          // Long time press -> should change the value
/* 236 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 237 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 238 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x7C}}
          // Long time press -> should change the value
/* 239 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 240 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 241 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x8A}}
          // Long time press -> should change the value
/* 242 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 243 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 244 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0x98}}
          // Long time press -> should change the value
/* 245 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 246 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 247 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xA6}}
          // Long time press -> should change the value
/* 248 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 249 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 250 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xB4}}
          // Long time press -> should change the value
/* 251 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 252 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 253 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xC2}}
          // Long time press -> should change the value
/* 254 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 255 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 256 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xD0}}
          // Long time press -> should change the value
/* 257 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 258 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 259 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xDE}}
          // Long time press -> should change the value
/* 260 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 261 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 262 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xEC}}
          // Long time press -> should change the value
/* 263 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 264 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 265 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xFA}}
          // Long time press -> should change the value
/* 266 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 267 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 268 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xFF}}
          // Long time press -> should not change the value
/* 269 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
/* 270 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // We are already ouside the limits 0-255, therefore no further telegram
/* 271 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 5
/* 272 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
/* 273 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 274 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 275 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xFF}}
/* 276 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 277 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Channel 5 sends at falling edge
          // Rising edge on channel 5
/* 278 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
/* 279 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 280 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 281 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 282 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Long time press -> should not change the value
/* 283 */ , {TIMER_TICK     , 5000,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 284 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 285 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xFA}}
          // Long time press -> should change the value
/* 286 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 287 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 288 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xEC}}
          // Long time press -> should change the value
/* 289 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 290 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 291 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xDE}}
          // Long time press -> should change the value
/* 292 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 293 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 294 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xD0}}
          // Long time press -> should change the value
/* 295 */ , {TIMER_TICK     , 11088,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 296 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 297 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xC2}}
          // Falling edge on channel 5
/* 298 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
/* 299 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 300 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set value
/* 301 */ , {TEL_TX         ,    9,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE2, 0x00, 0x80, 0xC2}}
/* 302 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 303 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_dim_encoder_tc =
{
  "IN8 - Dim Encoder"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dim_encoder_eepromSetup
, _resetInputs
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dim_encoder
};

TEST_CASE("IN8 - Dim Encoder","[APP][IN8][DIMENC]")
{
  executeTest(& input_as_dim_encoder_tc);
}
// <<< TC:input_as_dim_encoder
