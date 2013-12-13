#include "Basic.h"
#include "Automated.h"

extern  CU_SuiteInfo EEP_Suites[];

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();
   if (CUE_SUCCESS != CU_register_nsuites(1, EEP_Suites))
   {
       CU_cleanup_registry();
       return CU_get_error();
   }

#if 1
   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
#else
   CU_set_output_filename("sblib");
   CU_list_tests_to_file();
   CU_set_output_filename("sblib-results");
   CU_automated_run_tests();
#endif
   CU_cleanup_registry();
   return CU_get_error();
}

