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
    Telegram * t  = tc->telegram;

    sbStatus      = tc->status;
    if (tc->setup) tc->setup();

    while (END != t->type)
    {
        if (TEL_RX == t->type)
        {
            memcpy(sbRecvTelegram, t->bytes, t->length);
            sbRecvTelegramLen = t->length;
            sb_process_tel();
        }
        else
        {
            int i;
            sb_send_next_tel();
            CU_ASSERT(sbSendTelegramLen == (t->length + 1));
            for (i = 0; i < t->length; i++)
            {
                if (t->bytes[i] != sbSendTelegram[i])
                {
                    char msg[1025];
                    snprintf ( msg
                             , 1024
                             , "Send telegram %d missmatch at byte %d e=%02X, r=%02X"
                             , tn, i, t->bytes[i], sbSendTelegram[i]
                             );
                    CU_assertImplementation(CU_FALSE, __LINE__, msg, __FILE__, "", CU_FALSE);
                }
            }
        }
        t++;
        tn++;
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

