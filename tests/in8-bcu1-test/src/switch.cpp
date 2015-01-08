/*
 *  switch.cpp - Test the 8in switch functions
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

// >>> TC:input_as_switch
// Date: 2014-07-17 13:08:34.512454

/* Code for test case input_as_switch */
static void input_as_switch_eepromSetup(void)
{
    // >>> EEPROM INIT
    // Date: 2014-07-17 13:08:34.512454
    // Assoc Table (0x17B):
    //    1 ( 1/0/30) <->  0 (switch state 1.1    ) @ 0x17C
    //    2 ( 1/0/31) <->  1 (switch state 2.1    ) @ 0x17E
    //    3 ( 1/0/32) <->  2 (switch state 3.1    ) @ 0x180
    //    4 ( 1/0/33) <->  3 (switch state 4.1    ) @ 0x182
    //    5 ( 1/0/38) <->  8 (switch state 1.2    ) @ 0x184
    //    6 ( 1/0/39) <->  9 (switch state 2.2    ) @ 0x186
    //    7 ( 1/0/40) <-> 10 (switch state 3.2    ) @ 0x188
    //    8 ( 1/0/41) <-> 11 (switch state 4.2    ) @ 0x18A
    //    9 ( 1/0/50) <-> 16 (switch lock 1       ) @ 0x18C
    //   10 ( 1/0/51) <-> 17 (switch lock 2       ) @ 0x18E
    //   11 ( 1/0/52) <-> 18 (switch lock 3       ) @ 0x190
    //   12 ( 1/0/53) <-> 19 (switch lock 4       ) @ 0x192
    // Address Table (0x116):
    //   ( 0)   1.1.1 @ 0x117
    //   ( 1)  1/0/30 @ 0x119
    //   ( 2)  1/0/31 @ 0x11B
    //   ( 3)  1/0/32 @ 0x11D
    //   ( 4)  1/0/33 @ 0x11F
    //   ( 5)  1/0/38 @ 0x121
    //   ( 6)  1/0/39 @ 0x123
    //   ( 7)  1/0/40 @ 0x125
    //   ( 8)  1/0/41 @ 0x127
    //   ( 9)  1/0/50 @ 0x129
    //   (10)  1/0/51 @ 0x12B
    //   (11)  1/0/52 @ 0x12D
    //   (12)  1/0/53 @ 0x12F
    // Com Object table (0x131):
    //   ( 0) switch state 1.1     <6D, 4F, 00> @ 0x133 (1.001:Switch)
    //   ( 1) switch state 2.1     <6E, 4F, 00> @ 0x136 (1.001:Switch)
    //   ( 2) switch state 3.1     <6F, 4F, 00> @ 0x139 (1.001:Switch)
    //   ( 3) switch state 4.1     <70, 4F, 00> @ 0x13C (1.001:Switch)
    //   ( 4) switch state 5.1     <71, 4F, 00> @ 0x13F (1.001:Switch)
    //   ( 5) switch state 6.1     <72, 4F, 00> @ 0x142 (1.001:Switch)
    //   ( 6) switch state 7.1     <73, 4F, 00> @ 0x145 (1.001:Switch)
    //   ( 7) switch state 8.1     <74, 4F, 00> @ 0x148 (1.001:Switch)
    //   ( 8) switch state 1.2     <75, 4F, 00> @ 0x14B (1.001:Switch)
    //   ( 9) switch state 2.2     <76, 4F, 00> @ 0x14E (1.001:Switch)
    //   (10) switch state 3.2     <77, 4F, 00> @ 0x151 (1.001:Switch)
    //   (11) switch state 4.2     <78, 4F, 00> @ 0x154 (1.001:Switch)
    //   (12) switch state 5.2     <79, 4F, 00> @ 0x157 (1.001:Switch)
    //   (13) switch state 6.2     <7A, 4F, 00> @ 0x15A (1.001:Switch)
    //   (14) switch state 7.2     <7B, 4F, 00> @ 0x15D (1.001:Switch)
    //   (15) switch state 8.2     <7C, 4F, 00> @ 0x160 (1.001:Switch)
    //   (16) switch lock 1        <7D, 17, 00> @ 0x163 (1.001:Switch)
    //   (17) switch lock 2        <7E, 17, 00> @ 0x166 (1.001:Switch)
    //   (18) switch lock 3        <7F, 17, 00> @ 0x169 (1.001:Switch)
    //   (19) switch lock 4        <80, 17, 00> @ 0x16C (1.001:Switch)
    //   (20) switch lock 5        <81, 17, 00> @ 0x16F (1.001:Switch)
    //   (21) switch lock 6        <82, 17, 00> @ 0x172 (1.001:Switch)
    //   (22) switch lock 7        <83, 17, 00> @ 0x175 (1.001:Switch)
    //   (23) switch lock 8        <84, 17, 00> @ 0x178 (1.001:Switch)
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
    userEeprom[0x111] = 0x7B;
    userEeprom[0x112] = 0x31;
    userEeprom[0x113] = 0x00;
    userEeprom[0x114] = 0x00;
    userEeprom[0x115] = 0x00;
    userEeprom[0x116] = 0x0C;
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
    userEeprom[0x122] = 0x26;
    userEeprom[0x123] = 0x08;
    userEeprom[0x124] = 0x27;
    userEeprom[0x125] = 0x08;
    userEeprom[0x126] = 0x28;
    userEeprom[0x127] = 0x08;
    userEeprom[0x128] = 0x29;
    userEeprom[0x129] = 0x08;
    userEeprom[0x12A] = 0x32;
    userEeprom[0x12B] = 0x08;
    userEeprom[0x12C] = 0x33;
    userEeprom[0x12D] = 0x08;
    userEeprom[0x12E] = 0x34;
    userEeprom[0x12F] = 0x08;
    userEeprom[0x130] = 0x35;
    userEeprom[0x131] = 0x18;
    userEeprom[0x132] = 0x61;
    userEeprom[0x133] = 0x6D;
    userEeprom[0x134] = 0x4F;
    userEeprom[0x135] = 0x00;
    userEeprom[0x136] = 0x6E;
    userEeprom[0x137] = 0x4F;
    userEeprom[0x138] = 0x00;
    userEeprom[0x139] = 0x6F;
    userEeprom[0x13A] = 0x4F;
    userEeprom[0x13B] = 0x00;
    userEeprom[0x13C] = 0x70;
    userEeprom[0x13D] = 0x4F;
    userEeprom[0x13E] = 0x00;
    userEeprom[0x13F] = 0x71;
    userEeprom[0x140] = 0x4F;
    userEeprom[0x141] = 0x00;
    userEeprom[0x142] = 0x72;
    userEeprom[0x143] = 0x4F;
    userEeprom[0x144] = 0x00;
    userEeprom[0x145] = 0x73;
    userEeprom[0x146] = 0x4F;
    userEeprom[0x147] = 0x00;
    userEeprom[0x148] = 0x74;
    userEeprom[0x149] = 0x4F;
    userEeprom[0x14A] = 0x00;
    userEeprom[0x14B] = 0x75;
    userEeprom[0x14C] = 0x4F;
    userEeprom[0x14D] = 0x00;
    userEeprom[0x14E] = 0x76;
    userEeprom[0x14F] = 0x4F;
    userEeprom[0x150] = 0x00;
    userEeprom[0x151] = 0x77;
    userEeprom[0x152] = 0x4F;
    userEeprom[0x153] = 0x00;
    userEeprom[0x154] = 0x78;
    userEeprom[0x155] = 0x4F;
    userEeprom[0x156] = 0x00;
    userEeprom[0x157] = 0x79;
    userEeprom[0x158] = 0x4F;
    userEeprom[0x159] = 0x00;
    userEeprom[0x15A] = 0x7A;
    userEeprom[0x15B] = 0x4F;
    userEeprom[0x15C] = 0x00;
    userEeprom[0x15D] = 0x7B;
    userEeprom[0x15E] = 0x4F;
    userEeprom[0x15F] = 0x00;
    userEeprom[0x160] = 0x7C;
    userEeprom[0x161] = 0x4F;
    userEeprom[0x162] = 0x00;
    userEeprom[0x163] = 0x7D;
    userEeprom[0x164] = 0x17;
    userEeprom[0x165] = 0x00;
    userEeprom[0x166] = 0x7E;
    userEeprom[0x167] = 0x17;
    userEeprom[0x168] = 0x00;
    userEeprom[0x169] = 0x7F;
    userEeprom[0x16A] = 0x17;
    userEeprom[0x16B] = 0x00;
    userEeprom[0x16C] = 0x80;
    userEeprom[0x16D] = 0x17;
    userEeprom[0x16E] = 0x00;
    userEeprom[0x16F] = 0x81;
    userEeprom[0x170] = 0x17;
    userEeprom[0x171] = 0x00;
    userEeprom[0x172] = 0x82;
    userEeprom[0x173] = 0x17;
    userEeprom[0x174] = 0x00;
    userEeprom[0x175] = 0x83;
    userEeprom[0x176] = 0x17;
    userEeprom[0x177] = 0x00;
    userEeprom[0x178] = 0x84;
    userEeprom[0x179] = 0x17;
    userEeprom[0x17A] = 0x00;
    userEeprom[0x17B] = 0x0C;
    userEeprom[0x17C] = 0x01;
    userEeprom[0x17D] = 0x00;
    userEeprom[0x17E] = 0x02;
    userEeprom[0x17F] = 0x01;
    userEeprom[0x180] = 0x03;
    userEeprom[0x181] = 0x02;
    userEeprom[0x182] = 0x04;
    userEeprom[0x183] = 0x03;
    userEeprom[0x184] = 0x05;
    userEeprom[0x185] = 0x08;
    userEeprom[0x186] = 0x06;
    userEeprom[0x187] = 0x09;
    userEeprom[0x188] = 0x07;
    userEeprom[0x189] = 0x0A;
    userEeprom[0x18A] = 0x08;
    userEeprom[0x18B] = 0x0B;
    userEeprom[0x18C] = 0x09;
    userEeprom[0x18D] = 0x10;
    userEeprom[0x18E] = 0x0A;
    userEeprom[0x18F] = 0x11;
    userEeprom[0x190] = 0x0B;
    userEeprom[0x191] = 0x12;
    userEeprom[0x192] = 0x0C;
    userEeprom[0x193] = 0x13;
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
    userEeprom[0x1CE] = 0x11;
    userEeprom[0x1CF] = 0x11;
    userEeprom[0x1D0] = 0x11;
    userEeprom[0x1D1] = 0x11;
    userEeprom[0x1D2] = 0x0A;
    userEeprom[0x1D3] = 0x1E;
    userEeprom[0x1D4] = 0x11;
    userEeprom[0x1D5] = 0x1D;
    userEeprom[0x1D6] = 0x03;
    userEeprom[0x1D7] = 0x96;
    userEeprom[0x1D8] = 0x02;
    userEeprom[0x1D9] = 0x62;
    userEeprom[0x1DA] = 0x11;
    userEeprom[0x1DB] = 0x96;
    userEeprom[0x1DC] = 0x01;
    userEeprom[0x1DD] = 0xB1;
    userEeprom[0x1DE] = 0x11;
    userEeprom[0x1DF] = 0x36;
    userEeprom[0x1E0] = 0x03;
    userEeprom[0x1E1] = 0xC0;
    userEeprom[0x1E2] = 0x11;
    userEeprom[0x1E3] = 0x96;
    userEeprom[0x1E4] = 0x00;
    userEeprom[0x1E5] = 0x00;
    userEeprom[0x1E6] = 0x11;
    userEeprom[0x1E7] = 0x00;
    userEeprom[0x1E8] = 0x00;
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
    userEeprom[0x1F6] = 0x30;
    userEeprom[0x1F7] = 0x33;
    userEeprom[0x1F8] = 0x33;
    userEeprom[0x1F9] = 0x33;
    userEeprom[0x1FA] = 0x43;
    userEeprom[0x1FB] = 0x33;
    userEeprom[0x1FC] = 0x33;
    userEeprom[0x1FD] = 0x33;
    userEeprom[0x1FE] = 0x33;
    userEeprom[0x1FF] = 0x00;
    // <<< EEPROM INIT
}

static Telegram tel_input_as_switch[] =
{
// Bus Return Telegrams
/*   1 */   {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*   2 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*   3 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x81}}
/*   4 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x21, 0xE1, 0x00, 0x80}}
/*   5 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x81}}
/*   6 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
/*   7 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x29, 0xE1, 0x00, 0x80}}

// Set input 1
/*   8 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Set          , {}}
          // No direct action, only after debounce is complete
/*   9 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
          // After debounce set ON for OBJ 0 and OFF for OBJ 8
/*  10 */ , {TIMER_TICK     ,    5,  0, (StepFunction *) _loop               , {}}
/*  11 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  12 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  13 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x80}}

// check the cyclic sending
          // Cyclic timer not yet expired
/*  14 */ , {TIMER_TICK     , 3162,  0, (StepFunction *) _loop               , {}}
          // Cyclic timeout for primary 1 expired
/*  15 */ , {TIMER_TICK     ,    1,  0, (StepFunction *) _loop               , {}}
          // Put the cyclic telegram into the send queue
/*  16 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  17 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  18 */ , {TIMER_TICK     , 3168,  0, (StepFunction *) _loop               , {}}
          // Cyclic timeout for secondary 1 expired
/*  19 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  20 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  21 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x80}}

// Clear input 1
/*  22 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _input1Clear        , {}}
          // After debounce set OFF for OBJ 0 and ON for OBJ 8
/*  23 */ , {TIMER_TICK     ,    5,  0, (StepFunction *) _loop               , {}}
          // Need another tick for sending the obj
/*  24 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  25 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
/*  26 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}

// check the cyclic sending
          // Cyclic timeout for primary 1 expired
/*  27 */ , {TIMER_TICK     , 3163,  0, (StepFunction *) _loop               , {}}
          // Put the cyclic telegram into the send queue
/*  28 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  29 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
          // Cyclic timeout for secondary 1 expired
/*  30 */ , {TIMER_TICK     , 3168,  0, (StepFunction *) _loop               , {}}
          // Put the cyclic telegram into the send queue
/*  31 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  32 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
/*  33 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}

// Test locking function: channel 1
// lock the channel
          // Receive a LOCK for channel 1
/*  34 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x81}}
/*  35 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  36 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x81}}
/*  37 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x81}}

// Try to change the input
// XXX still missing

// unlock the channel
          // Receive an UNLOCK for channel 1
/*  38 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x32, 0xE1, 0x00, 0x80}}
/*  39 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  40 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1E, 0xE1, 0x00, 0x80}}
/*  41 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x26, 0xE1, 0x00, 0x80}}

// Test locking function: channel 2
// lock the channel (inverted)
          // Receive a LOCK for channel 1
/*  42 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x80}}
/*  43 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  44 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x80}}
/*  45 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x80}}

// Try to change the input
// XXX still missing

// unlock the channel (inverted)
          // Receive an UNLOCK for channel 1
/*  46 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x33, 0xE1, 0x00, 0x81}}
/*  47 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  48 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x1F, 0xE1, 0x00, 0x81}}
/*  49 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x27, 0xE1, 0x00, 0x81}}

// Test locking function: channel 3
// lock the channel
          // Receive a LOCK for channel 3
/*  50 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x81}}
/*  51 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  52 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  53 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x80}}

// Try to change the input
// XXX still missing

// unlock the channel (inverted)
          // Receive an UNLOCK for channel 3
/*  54 */ , {TEL_RX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x14, 0x08, 0x34, 0xE1, 0x00, 0x80}}
/*  55 */ , {TIMER_TICK     ,    0,  0, (StepFunction *) _loop               , {}}
/*  56 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x20, 0xE1, 0x00, 0x80}}
/*  57 */ , {TEL_TX         ,    8,  0, (StepFunction *) _loop               , {0xBC, 0x11, 0x01, 0x08, 0x28, 0xE1, 0x00, 0x81}}
, {END}
};
static Test_Case input_as_switch_tc = 
{
  "IN8 - Normal Switch New"
, 0x0004, 0x7054, 02
, 17952 // power-on delay
, input_as_switch_eepromSetup
, NULL
, (StateFunction *) NULL
, (TestCaseState *) &_refState
, (TestCaseState *) &_stepState
, tel_input_as_switch
};

TEST_CASE("IN8 - Normal Switch New","[SWITCH]")
{
  executeTest(& input_as_switch_tc);
}
// <<< TC:input_as_switch
