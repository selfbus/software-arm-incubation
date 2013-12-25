/**
 * test of the selfbus protocol implementation
 */

#include "CUnit/Basic.h"

#include "sb_eeprom.h"
#include "sb_iap.h"
#include "iap_emu.h"
#include "sb_bus.h"
#include "sb_proto.h"
#include "sb_const.h"
#include "sb_memory.h"

#include "sb_proto_test.h"

static void run_test(Test_Case * tc)
{
    int        tn = 1;
    char msg[1025];
    Telegram * t  = tc->telegram;

    sbUserRam->status = tc->status;
    if (tc->setup) tc->setup();

    while (END != t->type)
    {
        if (TEL_RX == t->type)
        {
            int s;
            memcpy(sbRecvTelegram, t->bytes, t->length);
            sbRecvTelegramLen = t->length;
            sb_process_tel();
            s = sb_send_ring_count();
            snprintf ( msg
                     , 1024
                     , "%s: Number of response telegrams for %d incorrect: e=%d, s=%d"
                     , tc->name, tn, t->response_count, s
                     );
            CU_assertImplementation(s == t->response_count, __LINE__, msg, __FILE__, "", CU_FALSE);
        }
        else
        {
            int i;
            int c = 0;
            char received [23*3 + 1] = {0};
            char expected [23*3 + 1] = {0};
            sb_send_next_tel();
            snprintf ( msg
                     , 1024
                     , "%s: Number of bytes in send telegram %d mismatch e=%d, s=%d"
                     , tc->name, tn, t->length + 2, sb_tel_length(sbSendTelegram)
                     );
            CU_assertImplementation(sb_tel_length(sbSendTelegram) == (t->length + 1), __LINE__, msg, __FILE__, "", CU_FALSE);
            snprintf (msg, 1024, "%s: Send telegram %d mismatch at byte(s)", tc->name, tn);
            for (i = 0; i < t->length; i++)
            {
                snprintf (received, 23*3, "%s %02x", received, sbSendTelegram[i]);
                snprintf (expected, 23*3, "%s %02x", expected, t->bytes[i]);
                if (t->bytes[i] != sbSendTelegram[i])
                {
                    c++;
                    snprintf (msg, 1024, "%s %d, ", msg, i);
                }
            }
            if (c)
            {
                msg [strlen (msg) - 1] = '\n';
                snprintf(msg, 1024, "%s expected: %s\n received: %s", msg, expected, received);
                CU_assertImplementation(CU_FALSE, __LINE__, msg, __FILE__, "", CU_FALSE);
            }
        }
        if (t->check) t->check();
        t++;
        tn++;
        sbState = SB_IDLE;
    }
}

extern Test_Case physical_address_programming;
void test_physical_address_programming (void)
{
    run_test(& physical_address_programming);
}

extern Test_Case parameter_programming;
void test_parameter_programming (void)
{
    run_test(& parameter_programming);
}

extern Test_Case application_programming;
void test_application_programming (void)
{
    run_test(& application_programming);
}

extern Test_Case group_address_programming;
void test_group_address_programming (void)
{
    run_test(& group_address_programming);
}

extern Test_Case device_info;
void test_device_info (void)
{
    run_test(& device_info);
}

CU_TestInfo sbp_tests[] = {
/*
  { "Program physical address",     test_physical_address_programming},
  { "Parameter programming",        test_parameter_programming},
  { "Application programming",      test_application_programming},
  { "Group address programming",    test_group_address_programming},
*/
  { "Device info gathering",        test_device_info},
  CU_TEST_INFO_NULL,
};


CU_SuiteInfo SBP_Suites[] =
  { { "Protocol Tests"  // name
    , NULL              // suite setup function
    , NULL              // suite cleanup function
    , sbp_tests         // test cases
    }
  , CU_SUITE_INFO_NULL
  };

