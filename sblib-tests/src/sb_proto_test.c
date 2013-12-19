/**
 * test of the EEPROM emulation
 */

#include "Basic.h"
#include "sb_eep_emu.h"
#include "sb_iap.h"
#include "iap_emu.h"
#include "sb_bus.h"
#include "sb_proto.h"
#include "sb_const.h"

#include "sb_proto_test.h"

static void run_test(Test_Case * tc)
{
    int        tn = 0;
    char msg[1025];
    Telegram * t  = tc->telegram;

    sbStatus      = tc->status;
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
            sb_send_next_tel();
            snprintf ( msg
                     , 1024
                     , "%s: Number of bytes in send telegram %d mismatch e=%d, s=%d"
                     , tc->name, tn, t->length + 2, sbSendTelegramLen
                     );
            CU_assertImplementation(sbSendTelegramLen == (t->length + 1), __LINE__, msg, __FILE__, "", CU_FALSE);
            for (i = 0; i < t->length; i++)
            {
                if (t->bytes[i] != sbSendTelegram[i])
                {
                    snprintf ( msg
                             , 1024
                             , "%s: Send telegram %d missmatch at byte %d e=%02X, r=%02X"
                             , tc->name, tn, i, t->bytes[i], sbSendTelegram[i]
                             );
                    CU_assertImplementation(CU_FALSE, __LINE__, msg, __FILE__, "", CU_FALSE);
                }
            }
        }
        if (t->check) t->check();
        t++;
        tn++;
        sbState = SB_IDLE;
    }
}

extern Test_Case prog_pa;

void test_prog_pa (void)
{
    run_test(& prog_pa);
}

CU_TestInfo sbp_tests[] = {
  { "Program PA",   test_prog_pa },
  CU_TEST_INFO_NULL,
};


CU_SuiteInfo SBP_Suites[] =
  { { "EEP-EMU" // name
    , NULL      // suite setup function
    , NULL      // suite cleanup function
    , sbp_tests // test cases
    }
  , CU_SUITE_INFO_NULL
  };

