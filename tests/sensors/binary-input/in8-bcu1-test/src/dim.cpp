/*
 *  dim.cpp - Test the 8in dimmer functions
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
// >>> TC:input_as_dimmer_locking
// Date: 2014-07-17 13:08:23.914454

/* Code for test case input_as_dimmer_locking */
static void input_as_dimmer_locking_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:23.914454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_locking[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Test channel 2
/*   4 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send ON
/*   6 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}
/*   7 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*   8 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock OFF -> send OFF
/*   9 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  10 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send ON
/*  12 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}
/*  13 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock OFF -> send OFF
/*  15 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}

// Test channel 3
          // Inverted -> 0 lock on, 1 -> lock off
/*  16 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send OFF
/*  18 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  19 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  20 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock off -> no reaction
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Inverted -> 0 lock on, 1 -> lock off
/*  22 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send OFF
/*  24 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  25 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock off -> no reaction
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Test channel 4
/*  28 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x81}}
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> toggle value
/*  30 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}}
/*  31 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x80}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock off -> no reaction
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x81}}
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> toggle value
/*  36 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x80}}
/*  37 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x80}}
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock off -> no reaction
/*  39 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_dimmer_locking_tc = 
{
  "IN8 - Dimmer locking"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_locking_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_locking
};

TEST_CASE("IN8 - Dimmer locking","[APP][IN8][DIM][LOCKING]")
{
  executeTest(& input_as_dimmer_locking_tc);
}
// <<< TC:input_as_dimmer_locking
// >>> TC:input_as_dimmer
// Date: 2014-07-17 13:08:23.952454

/* Code for test case input_as_dimmer */
static void input_as_dimmer_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:23.952454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 1
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set ON for OBJ 0
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}

// Clear input 1
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}

// Test Dimmer brighter
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
          // No direct action, only after debounce is complete
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set OFF for OBJ 0
/*  14 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  15 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
/*  17 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  18 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x8A}}
/*  20 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // After the repitition time has exipred, no telegram should be sent
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
          // No action after debounce because dimming has not been started
/*  24 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  25 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Stop telegram should be sent
/*  26 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x88}}

// The next toggle should turn on the output and dimm it darker
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
          // No direct action, only after debounce is complete
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set OFF for OBJ 0
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  32 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x83}}
/*  35 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // After the repitition time has exipred, no telegram should be sent
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
          // No action after debounce because dimming has not been started
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Stop telegram should be sent
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x80}}
/*  42 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  43 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_dimmer_tc = 
{
  "IN8 - Dimmer"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer
};

TEST_CASE("IN8 - Dimmer","[APP][IN8][DIM][ONE]")
{
  executeTest(& input_as_dimmer_tc);
}
// <<< TC:input_as_dimmer
// >>> TC:input_as_dimmer_two_on_brighter
// Date: 2014-07-17 13:08:23.988454

/* Code for test case input_as_dimmer_two_on_brighter */
static void input_as_dimmer_two_on_brighter_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:23.988454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_two_on_brighter[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 2
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set ON for OBJ 1
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Clear input 2
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// The next switch should send an ON telegram again
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
          // After debounce set ON for OBJ 1
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}
/*  18 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x8B}}
/*  21 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 2
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
          // Generate the stop telegramm
/*  25 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x88}}

// The next switch should send an ON telegram again
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
          // After debounce set ON for OBJ 1
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}
/*  32 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x8B}}
/*  35 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 2
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
          // Generate the stop telegramm
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x88}}
, {END}
};
static Test_Case input_as_dimmer_two_on_brighter_tc = 
{
  "IN8 - Dimmer Two Hand On Brighter"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_two_on_brighter_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_two_on_brighter
};

TEST_CASE("IN8 - Dimmer Two Hand On Brighter","[APP][IN8][DIM][TWO_ON_B]")
{
  executeTest(& input_as_dimmer_two_on_brighter_tc);
}
// <<< TC:input_as_dimmer_two_on_brighter
// >>> TC:input_as_dimmer_two_on_brighter_rep
// Date: 2014-07-17 13:08:24.012454

/* Code for test case input_as_dimmer_two_on_brighter_rep */
static void input_as_dimmer_two_on_brighter_rep_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:24.012454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_two_on_brighter_rep[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 6
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set ON for OBJ 5
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x23, 0xE1, 0x00, 0x81}}

// Clear input 2
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// The next switch should send an ON telegram again
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
          // After debounce set ON for OBJ 5
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x23, 0xE1, 0x00, 0x81}}
/*  18 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x8B}}
/*  21 */ , {TIMER_TICK     , 2640,  0, (StepFunction *) _loop               , {}}
          // The dim object should be repeated
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x8B}}
/*  24 */ , {TIMER_TICK     , 2640,  0, (StepFunction *) _loop               , {}}
          // The dim object should be repeated
/*  25 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  26 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x8B}}
/*  27 */ , {TIMER_TICK     , 2640,  0, (StepFunction *) _loop               , {}}
          // The dim object should be repeated
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  29 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x8B}}

// Clear input 2
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
          // Generate the stop telegramm
/*  31 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x88}}
/*  34 */ , {TIMER_TICK     , 2640,  0, (StepFunction *) _loop               , {}}
          // After the stop telegram, no repition should happen
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_dimmer_two_on_brighter_rep_tc = 
{
  "IN8 - Dimmer Two Hand On Brighter with Repition"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_two_on_brighter_rep_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_two_on_brighter_rep
};

TEST_CASE("IN8 - Dimmer Two Hand On Brighter with Repition","[APP][IN8][DIM][TWO_ON_B_REP]")
{
  executeTest(& input_as_dimmer_two_on_brighter_rep_tc);
}
// <<< TC:input_as_dimmer_two_on_brighter_rep
// >>> TC:input_as_dimmer_two_toggle_brighter
// Date: 2014-07-17 13:08:24.038454

/* Code for test case input_as_dimmer_two_toggle_brighter */
static void input_as_dimmer_two_toggle_brighter_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:24.038454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_two_toggle_brighter[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 4
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set ON for OBJ 3
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}}

// Clear input 4
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// The next switch should send an ON telegram again
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
          // After debounce set OFF for OBJ 3 (Toggle)
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x80}}
/*  18 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x89}}
/*  21 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 4
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
          // Generate the stop telegramm
/*  25 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x88}}

// The next switch should send an ON telegram again
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
          // After debounce set OFF for OBJ 3 (Toggle)
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}}
/*  32 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x89}}
/*  35 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 4
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
          // Generate the stop telegramm
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x88}}
, {END}
};
static Test_Case input_as_dimmer_two_toggle_brighter_tc = 
{
  "IN8 - Dimmer Two Hand Toggle Brighter"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_two_toggle_brighter_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_two_toggle_brighter
};

TEST_CASE("IN8 - Dimmer Two Hand Toggle Brighter","[APP][IN8][DIM][TWO_TO_B]")
{
  executeTest(& input_as_dimmer_two_toggle_brighter_tc);
}
// <<< TC:input_as_dimmer_two_toggle_brighter
// >>> TC:input_as_dimmer_two_off_darker
// Date: 2014-07-17 13:08:24.063454

/* Code for test case input_as_dimmer_two_off_darker */
static void input_as_dimmer_two_off_darker_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:24.063454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_two_off_darker[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 3
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set OFF for OBJ 2
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}

// Clear input 3
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// The next switch should send an OFF telegram again
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
          // After debounce set OFF for OBJ 2
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  18 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
/*  21 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 3
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
          // Generate the stop telegramm
/*  25 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}

// The next switch should send an OFF telegram again
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
          // After debounce set OFF for OBJ 2
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  32 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
/*  35 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 3
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
          // Generate the stop telegramm
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
, {END}
};
static Test_Case input_as_dimmer_two_off_darker_tc = 
{
  "IN8 - Dimmer Two Hand Off darker"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_two_off_darker_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_two_off_darker
};

TEST_CASE("IN8 - Dimmer Two Hand Off darker","[APP][IN8][DIM][TWO_OFF_D]")
{
  executeTest(& input_as_dimmer_two_off_darker_tc);
}
// <<< TC:input_as_dimmer_two_off_darker
// >>> TC:input_as_dimmer_two_to_darker
// Date: 2014-07-17 13:08:24.091454

/* Code for test case input_as_dimmer_two_to_darker */
static void input_as_dimmer_two_to_darker_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:24.091454
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (dim switch 1.1      ) @ 0x194
    //    2 ( 1/0/31) <->  1 (dim switch 2.1      ) @ 0x196
    //    3 ( 1/0/32) <->  2 (dim switch 3.1      ) @ 0x198
    //    4 ( 1/0/33) <->  3 (dim switch 4.1      ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (dim switch 5.1      ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (dim switch 6.1      ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (dim 1.2             ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (dim 2.2             ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (dim 3.2             ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (dim 4.2             ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (dim 5.2             ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (dim 6.2             ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (dim lock 1          ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (dim lock 2          ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (dim lock 3          ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (dim lock 4          ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (dim lock 5          ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (dim lock 6          ) @ 0x1BE
    //   23 ( 1/0/56) <-> 22 (generic lock 7      ) @ 0x1C0
    //   24 ( 1/0/57) <-> 23 (generic lock 8      ) @ 0x1C2
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
    //   ( 0) dim switch 1.1       <6D, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 1) dim switch 2.1       <6E, 4F, 00> @ 0x14E (1.001:Switch)
    //   ( 2) dim switch 3.1       <6F, 4F, 00> @ 0x151 (1.001:Switch)
    //   ( 3) dim switch 4.1       <70, 4F, 00> @ 0x154 (1.001:Switch)
    //   ( 4) dim switch 5.1       <71, 4F, 00> @ 0x157 (1.001:Switch)
    //   ( 5) dim switch 6.1       <72, 4F, 00> @ 0x15A (1.001:Switch)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) dim 1.2              <75, 4F, 03> @ 0x163 (3.007:Control_Dimming)
    //   ( 9) dim 2.2              <76, 4F, 03> @ 0x166 (3.007:Control_Dimming)
    //   (10) dim 3.2              <77, 4F, 03> @ 0x169 (3.007:Control_Dimming)
    //   (11) dim 4.2              <78, 4F, 03> @ 0x16C (3.007:Control_Dimming)
    //   (12) dim 5.2              <79, 4F, 03> @ 0x16F (3.007:Control_Dimming)
    //   (13) dim 6.2              <7A, 4F, 03> @ 0x172 (3.007:Control_Dimming)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) dim lock 1           <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) dim lock 2           <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) dim lock 3           <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) dim lock 4           <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) dim lock 5           <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) dim lock 6           <82, 17, 00> @ 0x18A (1.001:Switch)
    //   (22) generic lock 7       <83, 17, 00> @ 0x18D (1.001:Switch)
    //   (23) generic lock 8       <84, 17, 00> @ 0x190 (1.001:Switch)
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
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x6E;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x6F;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x70;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x71;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
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
    userEeprom[0x165] = 0x03;
    userEeprom[0x166] = 0x76;
    userEeprom[0x167] = 0x4F;
    userEeprom[0x168] = 0x03;
    userEeprom[0x169] = 0x77;
    userEeprom[0x16A] = 0x4F;
    userEeprom[0x16B] = 0x03;
    userEeprom[0x16C] = 0x78;
    userEeprom[0x16D] = 0x4F;
    userEeprom[0x16E] = 0x03;
    userEeprom[0x16F] = 0x79;
    userEeprom[0x170] = 0x4F;
    userEeprom[0x171] = 0x03;
    userEeprom[0x172] = 0x7A;
    userEeprom[0x173] = 0x4F;
    userEeprom[0x174] = 0x03;
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
    userEeprom[0x1CE] = 0x22;
    userEeprom[0x1CF] = 0x22;
    userEeprom[0x1D0] = 0x22;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x08;
    userEeprom[0x1D6] = 0x13;
    userEeprom[0x1D7] = 0x08;
    userEeprom[0x1D8] = 0x85;
    userEeprom[0x1D9] = 0x99;
    userEeprom[0x1DA] = 0x58;
    userEeprom[0x1DB] = 0x88;
    userEeprom[0x1DC] = 0x05;
    userEeprom[0x1DD] = 0x2A;
    userEeprom[0x1DE] = 0x80;
    userEeprom[0x1DF] = 0x08;
    userEeprom[0x1E0] = 0x05;
    userEeprom[0x1E1] = 0x39;
    userEeprom[0x1E2] = 0xC8;
    userEeprom[0x1E3] = 0x08;
    userEeprom[0x1E4] = 0x05;
    userEeprom[0x1E5] = 0x48;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x08;
    userEeprom[0x1E8] = 0x05;
    userEeprom[0x1E9] = 0x1C;
    userEeprom[0x1EA] = 0x18;
    userEeprom[0x1EB] = 0x08;
    userEeprom[0x1EC] = 0x0A;
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
    userEeprom[0x1FD] = 0x01;
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_dimmer_two_to_darker[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}

// Set input 5
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
          // No direct action, only after debounce is complete
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set OFF for OBJ 4
/*   6 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*   7 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   8 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE1, 0x00, 0x81}}

// Clear input 5
/*   9 */ , {TIMER_TICK     , 1055,  0, (StepFunction *) _loop               , {}}
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
          // No action after debounce because dimming has not been started
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// The next switch should send an OFF telegram again
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
          // After debounce set OFF for OBJ 4
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE1, 0x00, 0x80}}
/*  18 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}
/*  21 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 5
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
          // Generate the stop telegramm
/*  25 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  27 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}

// The next switch should send an OFF telegram again
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
          // After debounce set OFF for OBJ 4
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE1, 0x00, 0x81}}
/*  32 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  34 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}
/*  35 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // No telegram repitition
/*  36 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Clear input 5
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
          // Generate the stop telegramm
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}
, {END}
};
static Test_Case input_as_dimmer_two_to_darker_tc = 
{
  "IN8 - Dimmer Two Hand Toggle darker"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_dimmer_two_to_darker_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_dimmer_two_to_darker
};

TEST_CASE("IN8 - Dimmer Two Hand Toggle darker","[APP][IN8][DIM][TWO_TO_D]")
{
  executeTest(& input_as_dimmer_two_to_darker_tc);
}
// <<< TC:input_as_dimmer_two_to_darker
