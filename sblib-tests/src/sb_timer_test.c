/*
 *  Copyright (c) 2013 Martin Glueck <martin@mangari.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#include "CUnit/Basic.h"
#include "sb_timer.h"
#include "sb_utils.h"

#ifdef __USE_CMSIS
# include "LPC11xx.h"
#endif

#define CU_ASSERT_MSG(value, msg) \
  { CU_assertImplementation((value), __LINE__, msg, __FILE__, "", CU_FALSE); }

static void timer_tick(unsigned int t)
{
    sbSysTime += t;
//    sbSysTime &= (1 << 24) - 1; // it's a 24bit timer in the ARM
}

void test_systick_timer_single (void)
{
    SbTimer timer1;
    timer_tick(2);
    sb_timer_start(& timer1, 1000 * 20, 0); // single shot timer for 2ms
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(1 == sb_timer_check(& timer1));
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(4294937292U);
    sb_timer_start(& timer1, 1000 * 20, 0); // single shot timer for 2 ms @ 500µs
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1000 * 5);
    CU_ASSERT(0 == sb_timer_check(& timer1));
    timer_tick(1);
    CU_ASSERT(1 == sb_timer_check(& timer1));
    CU_ASSERT(0 == sb_timer_check(& timer1));
}

void test_systick_timer_periodic (void)
{
    SbTimer timer1;
    int i;

    sbSysTime = 0;
    timer_tick(2);
    sb_timer_start(& timer1, 0, 1000 * 20); // timer with 2 ms period @ 500µs
    timer_tick(1000 * 5);
    CU_ASSERT_FATAL(0 == sb_timer_check(& timer1));
    for (i = 0; i < 20000; i++)
    {
        timer_tick(1000 * 5);
        CU_ASSERT_FATAL(0 == sb_timer_check(& timer1));
        timer_tick(1000 * 5);
        CU_ASSERT_FATAL(0 == sb_timer_check(& timer1));
        timer_tick(1000 * 5);
        CU_ASSERT_FATAL(0 == sb_timer_check(& timer1));
        timer_tick(1000 * 5);
        CU_ASSERT_FATAL(1 == sb_timer_check(& timer1));
    }
}

void test_debounce(void)
{
    SbDebounce debounce;
    sb_init_debounce(&debounce, 0);

    timer_tick(2);
    CU_ASSERT(0x0000 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0000 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5 + 1);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0001, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0001 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5 + 1);
    CU_ASSERT(0x0002 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
}


//
// Test debouncing with sbSysTime overflow
//
void test_debounce_sbSysTime_overflow()
{
    SbDebounce debounce;
    sb_init_debounce(&debounce, 0);

    sbSysTime = -1;
    CU_ASSERT(0x0000 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(1000 * 5);
    CU_ASSERT(0x0000 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
    timer_tick(SB_DEBOUNCE_10MS);
    CU_ASSERT(0x0002 == sb_debounce(0x0002, SB_DEBOUNCE_10MS, & debounce));
}


CU_TestInfo syst_tests[] =
{
    { "SysTick timer single",     test_systick_timer_single },
    { "SysTick timer periodic",   test_systick_timer_periodic },
    { "Debounce test",            test_debounce },
    { "Debouncing with sbSysTime overflow", test_debounce_sbSysTime_overflow },
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


