/*
 *  jalo.cpp - Test the 8in jalo functions
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
// >>> TC:input_as_jalo_lsl
// Date: 2014-07-22 10:22:53.142551

/* Code for test case input_as_jalo_lsl */
static void input_as_jalo_lsl_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-22 10:22:53.142551
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (jal short 1         ) @ 0x194
    //    2 ( 1/0/31) <->  1 (jal short 2         ) @ 0x196
    //    3 ( 1/0/32) <->  2 (jal short 3         ) @ 0x198
    //    4 ( 1/0/33) <->  3 (jal short 4         ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (jal short 5         ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (jal short 6         ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (jal long  1         ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (jal long  2         ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (jal long  3         ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (jal long  4         ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (jal long  5         ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (jal long  6         ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (jal lock  1         ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (jal lock  2         ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (jal lock  3         ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (jal lock  4         ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (jal lock  5         ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (jal lock  6         ) @ 0x1BE
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
    //   ( 0) jal short 1          <6D, 4F, 00> @ 0x14B (1.008:UpDown)
    //   ( 1) jal short 2          <6E, 4F, 00> @ 0x14E (1.008:UpDown)
    //   ( 2) jal short 3          <6F, 4F, 00> @ 0x151 (1.008:UpDown)
    //   ( 3) jal short 4          <70, 4F, 00> @ 0x154 (1.008:UpDown)
    //   ( 4) jal short 5          <71, 4F, 00> @ 0x157 (1.008:UpDown)
    //   ( 5) jal short 6          <72, 4F, 00> @ 0x15A (1.008:UpDown)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) jal long  1          <75, 4F, 00> @ 0x163 (1.007:Step)
    //   ( 9) jal long  2          <76, 4F, 00> @ 0x166 (1.007:Step)
    //   (10) jal long  3          <77, 4F, 00> @ 0x169 (1.007:Step)
    //   (11) jal long  4          <78, 4F, 00> @ 0x16C (1.007:Step)
    //   (12) jal long  5          <79, 4F, 00> @ 0x16F (1.007:Step)
    //   (13) jal long  6          <7A, 4F, 00> @ 0x172 (1.007:Step)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) jal lock  1          <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) jal lock  2          <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) jal lock  3          <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) jal lock  4          <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) jal lock  5          <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) jal lock  6          <82, 17, 00> @ 0x18A (1.001:Switch)
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
    userEeprom[0x1CE] = 0x33;
    userEeprom[0x1CF] = 0x33;
    userEeprom[0x1D0] = 0x33;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0xA1;
    userEeprom[0x1D6] = 0x04;
    userEeprom[0x1D7] = 0x05;
    userEeprom[0x1D8] = 0x10;
    userEeprom[0x1D9] = 0x5E;
    userEeprom[0x1DA] = 0x08;
    userEeprom[0x1DB] = 0x0A;
    userEeprom[0x1DC] = 0x20;
    userEeprom[0x1DD] = 0x35;
    userEeprom[0x1DE] = 0x04;
    userEeprom[0x1DF] = 0x04;
    userEeprom[0x1E0] = 0x30;
    userEeprom[0x1E1] = 0x09;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x04;
    userEeprom[0x1E4] = 0x18;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x04;
    userEeprom[0x1E8] = 0x28;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x0B;
    userEeprom[0x1EC] = 0x38;
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
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_jalo_lsl[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}

// Hit and release channel 1 within T1 (short-long time)
          // Rising edge on channel 1
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
/*   5 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*   6 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*   7 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*   8 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // No reaction
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 1
/*  10 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
/*  11 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 1 within T2 (blade changing time)
          // Rising edge on channel 1
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  18 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
/*  21 */ , {TIMER_TICK     ,  640,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 1
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
/*  23 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  25 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 1 after T2 (blade changing time)
          // Rising edge on channel 1
/*  27 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
/*  28 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  30 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  31 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/*  33 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
/*  34 */ , {TIMER_TICK     ,  660,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 1
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
/*  36 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  37 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 2 within T1 (short-long time)
          // Rising edge on channel 2
/*  39 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
/*  40 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  41 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  42 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  43 */ , {TIMER_TICK     , 1036,  0, (StepFunction *) _loop               , {}}
          // No reaction
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 2
/*  45 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
/*  46 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  47 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  48 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 2 within T2 (blade changing time)
          // Rising edge on channel 2
/*  49 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
/*  50 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  51 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  52 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  53 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  54 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/*  55 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}
/*  56 */ , {TIMER_TICK     , 1300,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 2
/*  57 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
/*  58 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  59 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  60 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  61 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 2 after T2 (blade changing time)
          // Rising edge on channel 2
/*  62 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Set          , {}}
/*  63 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  64 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  65 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  66 */ , {TIMER_TICK     , 1056,  0, (StepFunction *) _loop               , {}}
/*  67 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/*  68 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}
/*  69 */ , {TIMER_TICK     , 1320,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 2
/*  70 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input2Clear        , {}}
/*  71 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  72 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  73 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T1 (short-long time) value 1
          // Rising edge on channel 3
/*  74 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  75 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  76 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  77 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/*  78 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // No reaction
/*  79 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/*  80 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/*  81 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/*  82 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  83 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T2 (blade changing time) value 1
          // Rising edge on channel 3
/*  84 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  85 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  86 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  87 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/*  88 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/*  89 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/*  90 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
/*  91 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/*  92 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/*  93 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  94 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  95 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/*  96 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 after T2 (blade changing time) value 0
          // Rising edge on channel 3
/*  97 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/*  98 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  99 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 100 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/* 101 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/* 102 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/* 103 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
/* 104 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 105 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 106 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 107 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/* 108 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T1 (short-long time) value 1
          // Rising edge on channel 3
/* 109 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 110 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 111 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 112 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 113 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // No reaction
/* 114 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 115 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 116 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 117 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 118 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T2 (blade changing time) value 1
          // Rising edge on channel 3
/* 119 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 120 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 121 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 122 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 123 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/* 124 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/* 125 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
/* 126 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 127 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 128 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 129 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/* 130 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 131 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 after T2 (blade changing time) value 0
          // Rising edge on channel 3
/* 132 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 133 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 134 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 135 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/* 136 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/* 137 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/* 138 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
/* 139 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 140 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 141 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 142 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/* 143 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T1 (short-long time) value 1
          // Rising edge on channel 3
/* 144 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 145 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 146 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 147 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 148 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // No reaction
/* 149 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 150 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 151 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
/* 152 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/* 153 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 within T2 (blade changing time) value 1
          // Rising edge on channel 3
/* 154 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 155 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 156 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 157 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 158 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/* 159 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/* 160 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
/* 161 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 162 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 163 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 164 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/* 165 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/* 166 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 3 after T2 (blade changing time) value 0
          // Rising edge on channel 3
/* 167 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Set          , {}}
/* 168 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 169 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/* 170 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/* 171 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
/* 172 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After short long time set UP or DOWN on long obj
/* 173 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
/* 174 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 3
/* 175 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input3Clear        , {}}
/* 176 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/* 177 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/* 178 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_jalo_lsl_tc =
{
  "IN8 - Jalo ShortLongShort"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_jalo_lsl_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_jalo_lsl
};

TEST_CASE("IN8 - Jalo ShortLongShort","[JALO][SLS]")
{
  executeTest(& input_as_jalo_lsl_tc);
}
// <<< TC:input_as_jalo_lsl
// >>> TC:input_as_jalo_sl
// Date: 2014-07-22 10:22:53.105544

/* Code for test case input_as_jalo_sl */
static void input_as_jalo_sl_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-22 10:22:53.105544
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (jal short 1         ) @ 0x194
    //    2 ( 1/0/31) <->  1 (jal short 2         ) @ 0x196
    //    3 ( 1/0/32) <->  2 (jal short 3         ) @ 0x198
    //    4 ( 1/0/33) <->  3 (jal short 4         ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (jal short 5         ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (jal short 6         ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (jal long  1         ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (jal long  2         ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (jal long  3         ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (jal long  4         ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (jal long  5         ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (jal long  6         ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (jal lock  1         ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (jal lock  2         ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (jal lock  3         ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (jal lock  4         ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (jal lock  5         ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (jal lock  6         ) @ 0x1BE
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
    //   ( 0) jal short 1          <6D, 4F, 00> @ 0x14B (1.008:UpDown)
    //   ( 1) jal short 2          <6E, 4F, 00> @ 0x14E (1.008:UpDown)
    //   ( 2) jal short 3          <6F, 4F, 00> @ 0x151 (1.008:UpDown)
    //   ( 3) jal short 4          <70, 4F, 00> @ 0x154 (1.008:UpDown)
    //   ( 4) jal short 5          <71, 4F, 00> @ 0x157 (1.008:UpDown)
    //   ( 5) jal short 6          <72, 4F, 00> @ 0x15A (1.008:UpDown)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) jal long  1          <75, 4F, 00> @ 0x163 (1.007:Step)
    //   ( 9) jal long  2          <76, 4F, 00> @ 0x166 (1.007:Step)
    //   (10) jal long  3          <77, 4F, 00> @ 0x169 (1.007:Step)
    //   (11) jal long  4          <78, 4F, 00> @ 0x16C (1.007:Step)
    //   (12) jal long  5          <79, 4F, 00> @ 0x16F (1.007:Step)
    //   (13) jal long  6          <7A, 4F, 00> @ 0x172 (1.007:Step)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) jal lock  1          <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) jal lock  2          <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) jal lock  3          <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) jal lock  4          <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) jal lock  5          <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) jal lock  6          <82, 17, 00> @ 0x18A (1.001:Switch)
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
    userEeprom[0x1CE] = 0x33;
    userEeprom[0x1CF] = 0x33;
    userEeprom[0x1D0] = 0x33;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0xA1;
    userEeprom[0x1D6] = 0x04;
    userEeprom[0x1D7] = 0x05;
    userEeprom[0x1D8] = 0x10;
    userEeprom[0x1D9] = 0x5E;
    userEeprom[0x1DA] = 0x08;
    userEeprom[0x1DB] = 0x0A;
    userEeprom[0x1DC] = 0x20;
    userEeprom[0x1DD] = 0x35;
    userEeprom[0x1DE] = 0x04;
    userEeprom[0x1DF] = 0x04;
    userEeprom[0x1E0] = 0x30;
    userEeprom[0x1E1] = 0x09;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x04;
    userEeprom[0x1E4] = 0x18;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x04;
    userEeprom[0x1E8] = 0x28;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x0B;
    userEeprom[0x1EC] = 0x38;
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
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_jalo_sl[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}

// Hit and release channel 4 within T1 (blade changing time)
          // Rising edge on channel 4
/*   4 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
/*   5 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*   6 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on long obj
/*   7 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}}
/*   8 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 4
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
/*  10 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  12 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x81}}
/*  13 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 4 after T1 (blade changing time)
          // Rising edge on channel 4
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Set          , {}}
/*  15 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}}
/*  18 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 4
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input4Clear        , {}}
/*  20 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  21 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 5 within T1 (blade changing time)
          // Rising edge on channel 5
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
/*  24 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  25 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on long obj
/*  26 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}
/*  27 */ , {TIMER_TICK     ,  508,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 5
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
/*  29 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  30 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  31 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x22, 0xE1, 0x00, 0x80}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 5 after T1 (blade changing time)
          // Rising edge on channel 5
/*  33 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Set          , {}}
/*  34 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  36 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2A, 0xE1, 0x00, 0x80}}
/*  37 */ , {TIMER_TICK     ,  528,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 5
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input5Clear        , {}}
/*  39 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  40 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  41 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 within T1 (blade changing time) value 1
          // Rising edge on channel 6
/*  42 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  43 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  45 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x81}}
/*  46 */ , {TIMER_TICK     , 1432,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  47 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  48 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  49 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  50 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x23, 0xE1, 0x00, 0x81}}
/*  51 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 after T1 (blade changing time) value 0
          // Rising edge on channel 6
/*  52 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  53 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  54 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  55 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x80}}
/*  56 */ , {TIMER_TICK     , 1452,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  57 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  58 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  59 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  60 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 within T1 (blade changing time) value 1
          // Rising edge on channel 6
/*  61 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  62 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  63 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  64 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x81}}
/*  65 */ , {TIMER_TICK     , 1432,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  66 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  67 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  68 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  69 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x23, 0xE1, 0x00, 0x81}}
/*  70 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 after T1 (blade changing time) value 0
          // Rising edge on channel 6
/*  71 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  72 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  73 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  74 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x80}}
/*  75 */ , {TIMER_TICK     , 1452,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  76 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  77 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  78 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  79 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 within T1 (blade changing time) value 1
          // Rising edge on channel 6
/*  80 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  81 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  82 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  83 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x81}}
/*  84 */ , {TIMER_TICK     , 1432,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  85 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  86 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  87 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce send other UP or DOWN on short obj
/*  88 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x23, 0xE1, 0x00, 0x81}}
/*  89 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}

// Hit and release channel 6 after T1 (blade changing time) value 0
          // Rising edge on channel 6
/*  90 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Set          , {}}
/*  91 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  92 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set UP or DOWN on short obj
/*  93 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x2B, 0xE1, 0x00, 0x80}}
/*  94 */ , {TIMER_TICK     , 1452,  0, (StepFunction *) _loop               , {}}
          // Falling edge on channel 6
/*  95 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input6Clear        , {}}
/*  96 */ , {TIMER_TICK     ,   10,  0, (StepFunction *) _loop               , {}}
          // One additional loop to put the telegram into the queue
/*  97 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No telegram should be sent
/*  98 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
, {END}
};
static Test_Case input_as_jalo_sl_tc =
{
  "IN8 - Jalo LongShort"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_jalo_sl_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_jalo_sl
};

TEST_CASE("IN8 - Jalo LongShort","[JALO][LS]")
{
  executeTest(& input_as_jalo_sl_tc);
}
// <<< TC:input_as_jalo_sl
// >>> TC:input_as_jalo_locking
// Date: 2014-07-22 10:49:24.778050

/* Code for test case input_as_jalo_locking */
static void input_as_jalo_locking_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-22 10:49:24.778050
    // Assoc Table (0x193):
    //    1 ( 1/0/30) <->  0 (jal short 1         ) @ 0x194
    //    2 ( 1/0/31) <->  1 (jal short 2         ) @ 0x196
    //    3 ( 1/0/32) <->  2 (jal short 3         ) @ 0x198
    //    4 ( 1/0/33) <->  3 (jal short 4         ) @ 0x19A
    //    5 ( 1/0/34) <->  4 (jal short 5         ) @ 0x19C
    //    6 ( 1/0/35) <->  5 (jal short 6         ) @ 0x19E
    //    7 ( 1/0/36) <->  6 (generic 7.1         ) @ 0x1A0
    //    8 ( 1/0/37) <->  7 (generic 8.1         ) @ 0x1A2
    //    9 ( 1/0/38) <->  8 (jal long  1         ) @ 0x1A4
    //   10 ( 1/0/39) <->  9 (jal long  2         ) @ 0x1A6
    //   11 ( 1/0/40) <-> 10 (jal long  3         ) @ 0x1A8
    //   12 ( 1/0/41) <-> 11 (jal long  4         ) @ 0x1AA
    //   13 ( 1/0/42) <-> 12 (jal long  5         ) @ 0x1AC
    //   14 ( 1/0/43) <-> 13 (jal long  6         ) @ 0x1AE
    //   15 ( 1/0/44) <-> 14 (generic 7.2         ) @ 0x1B0
    //   16 ( 1/0/45) <-> 15 (generic 8.2         ) @ 0x1B2
    //   17 ( 1/0/50) <-> 16 (jal lock  1         ) @ 0x1B4
    //   18 ( 1/0/51) <-> 17 (jal lock  2         ) @ 0x1B6
    //   19 ( 1/0/52) <-> 18 (jal lock  3         ) @ 0x1B8
    //   20 ( 1/0/53) <-> 19 (jal lock  4         ) @ 0x1BA
    //   21 ( 1/0/54) <-> 20 (jal lock  5         ) @ 0x1BC
    //   22 ( 1/0/55) <-> 21 (jal lock  6         ) @ 0x1BE
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
    //   ( 0) jal short 1          <6D, 4F, 00> @ 0x14B (1.008:UpDown)
    //   ( 1) jal short 2          <6E, 4F, 00> @ 0x14E (1.008:UpDown)
    //   ( 2) jal short 3          <6F, 4F, 00> @ 0x151 (1.008:UpDown)
    //   ( 3) jal short 4          <70, 4F, 00> @ 0x154 (1.008:UpDown)
    //   ( 4) jal short 5          <71, 4F, 00> @ 0x157 (1.008:UpDown)
    //   ( 5) jal short 6          <72, 4F, 00> @ 0x15A (1.008:UpDown)
    //   ( 6) generic 7.1          <73, 4F, 00> @ 0x15D (1.001:Switch)
    //   ( 7) generic 8.1          <74, 4F, 00> @ 0x160 (1.001:Switch)
    //   ( 8) jal long  1          <75, 4F, 00> @ 0x163 (1.007:Step)
    //   ( 9) jal long  2          <76, 4F, 00> @ 0x166 (1.007:Step)
    //   (10) jal long  3          <77, 4F, 00> @ 0x169 (1.007:Step)
    //   (11) jal long  4          <78, 4F, 00> @ 0x16C (1.007:Step)
    //   (12) jal long  5          <79, 4F, 00> @ 0x16F (1.007:Step)
    //   (13) jal long  6          <7A, 4F, 00> @ 0x172 (1.007:Step)
    //   (14) generic 7.2          <7B, 4F, 00> @ 0x175 (1.001:Switch)
    //   (15) generic 8.2          <7C, 4F, 00> @ 0x178 (1.001:Switch)
    //   (16) jal lock  1          <7D, 17, 00> @ 0x17B (1.001:Switch)
    //   (17) jal lock  2          <7E, 17, 00> @ 0x17E (1.001:Switch)
    //   (18) jal lock  3          <7F, 17, 00> @ 0x181 (1.001:Switch)
    //   (19) jal lock  4          <80, 17, 00> @ 0x184 (1.001:Switch)
    //   (20) jal lock  5          <81, 17, 00> @ 0x187 (1.001:Switch)
    //   (21) jal lock  6          <82, 17, 00> @ 0x18A (1.001:Switch)
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
    userEeprom[0x1CE] = 0x33;
    userEeprom[0x1CF] = 0x33;
    userEeprom[0x1D0] = 0x33;
    userEeprom[0x1D1] = 0x00;
    userEeprom[0x1D2] = 0x14;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0xA1;
    userEeprom[0x1D6] = 0x04;
    userEeprom[0x1D7] = 0x05;
    userEeprom[0x1D8] = 0x10;
    userEeprom[0x1D9] = 0x5E;
    userEeprom[0x1DA] = 0x08;
    userEeprom[0x1DB] = 0x0A;
    userEeprom[0x1DC] = 0x20;
    userEeprom[0x1DD] = 0x35;
    userEeprom[0x1DE] = 0x04;
    userEeprom[0x1DF] = 0x04;
    userEeprom[0x1E0] = 0x30;
    userEeprom[0x1E1] = 0x09;
    userEeprom[0x1E2] = 0x00;
    userEeprom[0x1E3] = 0x04;
    userEeprom[0x1E4] = 0x18;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x00;
    userEeprom[0x1E7] = 0x04;
    userEeprom[0x1E8] = 0x28;
    userEeprom[0x1E9] = 0x00;
    userEeprom[0x1EA] = 0x00;
    userEeprom[0x1EB] = 0x0B;
    userEeprom[0x1EC] = 0x38;
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
    userEeprom[0x1FE] = 0x30;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_jalo_locking[] =
{
// check bus return telegram
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Bus return action for channel 1
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
          // Bus return action for channel 2
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}
          // Start locking
/*   4 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x81}}
/*   5 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send Up
/*   6 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
          // End locking
/*   7 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x80}}
/*   8 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No reaction at end of locking
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Start locking
/*  10 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x81}}
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send Up
/*  12 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}
          // End locking
/*  13 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x80}}
/*  14 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No reaction at end of locking
/*  15 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // Start locking
/*  16 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*  17 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send Down
/*  18 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}
          // End locking
/*  19 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*  20 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, toggle direction
/*  21 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x81}}
          // Start locking
/*  22 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*  23 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> send Down
/*  24 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}
          // End locking
/*  25 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*  26 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, toggle direction
/*  27 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x81}}
          // Start locking
/*  28 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  29 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> toggle Direction
/*  30 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
          // End locking
/*  31 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  32 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, send down
/*  33 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
          // Start locking
/*  34 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After lock ON -> toggle Direction
/*  36 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
          // End locking
/*  37 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  38 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, send down
/*  39 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}
          // Start locking
/*  40 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x81}}
/*  41 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No reaction at start of locking
/*  42 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End locking
/*  43 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x80}}
/*  44 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, send up
/*  45 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}}
          // Start locking
/*  46 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x81}}
/*  47 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // No reaction at start of locking
/*  48 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End locking
/*  49 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x35, 0xE1, 0x00, 0x80}}
/*  50 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // End of locking, send up
/*  51 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x81}}
, {END}
};
static Test_Case input_as_jalo_locking_tc =
{
  "IN8 - Jalo Locking"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_jalo_locking_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_jalo_locking
};

TEST_CASE("IN8 - Jalo Locking","[JALO][JLOCK]")
{
  executeTest(& input_as_jalo_locking_tc);
}
// <<< TC:input_as_jalo_locking
