/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "CUnit/Basic.h"
#include "internal/sb_timer.h"
#include "sb_utils.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CU_ASSERT_MSG(value, msg) \
  { CU_assertImplementation((value), __LINE__, msg, __FILE__, "", CU_FALSE); }

static void timer_tick(unsigned int t)
{
    unsigned int old = LPC_TMR16B0->TC;
    LPC_TMR16B0->TC = (LPC_TMR16B0->TC + t) & 0xFFFF;
    if (LPC_TMR16B0->TC < old) LPC_TMR16B0->IR |=  (1 << 3);
    else                       LPC_TMR16B0->IR &= ~(1 << 3);
}

void test_systick_timer_single (void)
{
    SbTimer timer1;
    sb_timer_init();
    timer_tick(2);
    sb_timer_start(& timer1, 4, 0); // single shot timer for 2 ms @ 500µs
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(1 == sb_timer_check(& timer1));
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(0xFFFF - 9);
    sb_timer_start(& timer1, 4, 0); // single shot timer for 2 ms @ 500µs
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(1 == sb_timer_check(& timer1));
}

void test_systick_timer_periodic (void)
{
    SbTimer timer1;
    int i;

    sb_timer_init();
    timer_tick(2);
    sb_timer_start(& timer1, 0, 4); // timer with 2 ms period @ 500µs
    timer_tick(1);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    for (i = 0;i < 20000;i++)
    {
        timer_tick(1);
        CU_ASSERT(0 == sb_timer_check(& timer1));
        timer_tick(1);
        CU_ASSERT(0 == sb_timer_check(& timer1));
        timer_tick(1);
        CU_ASSERT(0 == sb_timer_check(& timer1));
        timer_tick(1);
        CU_ASSERT(1 == sb_timer_check(& timer1));
    }
}

void test_debounce(void)
{
    SbDebounce debounce = {0x0000};

    sb_timer_init();
    timer_tick(2);
    CU_ASSERT(0x0000 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0000 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50 + 1);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(50 + 1);
    CU_ASSERT(0x0002 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
}

CU_TestInfo syst_tests[] = {
    { "SysTick timer single",     test_systick_timer_single},
    { "SysTick timer periodic",   test_systick_timer_periodic},
    { "Debounce test",            test_debounce},
  CU_TEST_INFO_NULL,
};


CU_SuiteInfo SYST_Suites[] =
  { { "Timer Tests"     // name
    , NULL              // suite setup function
    , NULL              // suite cleanup function
    , syst_tests        // test cases
    }
  , CU_SUITE_INFO_NULL
  };


