/**
 * test of the EEPROM emulation
 */

#include "CUnit/Basic.h"

#include "sb_eep_emu.h"
#include "sb_iap.h"
#include "iap_emu.h"

static const unsigned char pattern[] = {0xCA, 0xFF, 0xEE, 0xAF, 0xFE, 0xDE, 0xAD};

void test_sb_eep_init_0 (void)
{
    int iap_save [5] ;

    memcpy (iap_save, iap_calls, sizeof (iap_calls));
    CU_ASSERT (SB_EEP_NO_VALID_PAGE_FOUND == sb_eep_init (1));
    CU_ASSERT (eep_r.state == 255);
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 1));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 1));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 0));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 0));

    memcpy (iap_save, iap_calls, sizeof (iap_calls));
    CU_ASSERT (SB_EEP_NO_VALID_PAGE_FOUND == sb_eep_init (0));
    CU_ASSERT (eep_r.state == 255);
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 0));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 0));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 0));
}

void test_sb_eep_init_1 (void)
{
    int iap_save [5] ;

    memcpy    (iap_save, iap_calls, sizeof (iap_calls));
    memset    (eep, 0, eep_r.size);
    CU_ASSERT (0 == sb_eep_init (0));
    CU_ASSERT (eep_r.state == 1);
    CU_ASSERT (0 == memcmp (eep, (void *) SB_EEP_FLASH_SECTOR_ADDRESS + 256, eep_r.size));
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 0));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 0));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 0));

}

void test_sb_eep_update_0 (void)
{
    int iap_save [5] ;

    memcpy (eep, pattern, sizeof (pattern));
    memcpy (iap_save, iap_calls, sizeof (iap_calls));
    CU_ASSERT (0 == sb_eep_update ());
    CU_ASSERT (eep_r.state == 0);
    CU_ASSERT (0 == memcmp (eep, (void *)SB_EEP_FLASH_SECTOR_ADDRESS, eep_r.size));
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 2));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 1));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 1));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));

    eep[0] ^= 0xFF;
    memcpy (eep + 1, pattern, sizeof (pattern));
    memcpy (iap_save, iap_calls, sizeof (iap_calls));
    CU_ASSERT (0 == sb_eep_update ());
    CU_ASSERT (eep_r.state == 1);
    CU_ASSERT (0 == memcmp (eep, (void *)SB_EEP_FLASH_SECTOR_ADDRESS + 256, eep_r.size));
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
}

void test_sb_eep_update_1 (void)
{
    int iap_save [5] ;
    int i;

    for (i = 2; i < eep_r.rom_pages; i++)
    {
        eep [0] = i;
        memcpy (eep + 1, pattern, sizeof (pattern));
        memcpy (iap_save, iap_calls, sizeof (iap_calls));
        CU_ASSERT (0 == sb_eep_update ());
        CU_ASSERT (eep_r.state == i);
        CU_ASSERT (0 == memcmp (eep, (void *)SB_EEP_FLASH_SECTOR_ADDRESS + i * 256, eep_r.size));
        CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 1));
        CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 0));
        CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 0));
        CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
        CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
    }
}

void test_sb_eep_update_2 (void)
{
    int iap_save [5] ;

    memcpy (eep, pattern, sizeof (pattern));
    memcpy (iap_save, iap_calls, sizeof (iap_calls));
    CU_ASSERT (0 == sb_eep_update ());
    CU_ASSERT (eep_r.state == 0);
    CU_ASSERT (0 == memcmp (eep, (void *)SB_EEP_FLASH_SECTOR_ADDRESS, eep_r.size));
    CU_ASSERT (iap_calls [I_PREPARE]     == (iap_save [I_PREPARE]     + 2));
    CU_ASSERT (iap_calls [I_ERASE]       == (iap_save [I_ERASE]       + 1));
    CU_ASSERT (iap_calls [I_BLANK_CHECK] == (iap_save [I_BLANK_CHECK] + 1));
    CU_ASSERT (iap_calls [I_RAM2FLASH]   == (iap_save [I_RAM2FLASH]   + 1));
    CU_ASSERT (iap_calls [I_COMPARE]     == (iap_save [I_COMPARE]     + 1));
}

CU_TestInfo eep_tests[] = {
  { "sb_eep_init_0",   test_sb_eep_init_0 },
  { "sb_eep_update_0", test_sb_eep_update_0 },
  { "sb_eep_init_1",   test_sb_eep_init_1 },
  { "sb_eep_update_1", test_sb_eep_update_1 },
  { "sb_eep_update_2", test_sb_eep_update_2 },
  CU_TEST_INFO_NULL,
};


CU_SuiteInfo EEP_Suites[] =
  { { "EEP-EMU" // name
    , NULL      // suite setup function
    , NULL      // suite cleanup function
    , eep_tests // test cases
    }
  , CU_SUITE_INFO_NULL
  };

